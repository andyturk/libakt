#include <akt/json/reader.h>
#include <akt/json/writer.h>
#include <akt/json/visitor.h>

#include <gtest/gtest.h>
#include <cstring>
#include <cstdlib>

using namespace akt::json;

class ReplayVisitor : public Visitor {
  const char **tokens;

public:
  enum {
    INCOMPLETE,
    COMPLETE,
    ERROR
  } status;

  ReplayVisitor() :
    tokens(0),
    status(ERROR)
  {
  }

  void set_tokens(const char **tokens) {
    status = INCOMPLETE;
    this->tokens = tokens;
  }

  virtual void object_begin() override {
    const char *match = *tokens++;

    if (match == 0) {
      error();
    } else if (status == INCOMPLETE) {
      if (strcmp(match, "{")) error();
    } else {
      error();
    }
  }

  virtual void object_end() override {
    const char *match = *tokens++;

    if (match == 0) {
      error();
    } else if (status == INCOMPLETE) {
      if (strcmp(match, "}")) error();
    } else {
      error();
    }
  }

  virtual void array_begin() override {
    const char *match = *tokens++;

    if (match == 0) {
      error();
    } else if (status == INCOMPLETE) {
      if (strcmp(match, "[")) error();
    } else {
      error();
    }
  }

  virtual void array_end() override {
    const char *match = *tokens++;

    if (match == 0) {
      error();
    } else if (status == INCOMPLETE) {
      if (strcmp(match, "]")) error();
    } else {
      error();
    }
  }

  virtual void string(const char *text) {
    const char *match = *tokens++;

    if (match == 0) {
      error();
    } else if (status == INCOMPLETE) {
      if (strcmp(match, text)) error();
    } else {
      error();
    }
  }

  virtual void literal_true() override {
    string("true");
  }

  virtual void literal_false() override {
    string("false");
  }

  virtual void literal_null() override {
    string("null");
  }

  virtual void member_name(const char *text) {
    // a member name is the same as a string for our purposes
    string(text);
  }

  virtual void num_int(int32_t n) {
    const char *match = *tokens++;

    if (match == 0) {
      error();
    } else if (status == INCOMPLETE) {
      int32_t match_n = atoi(match);
      if (n != match_n) error();
    } else {
      error();
    }
  }

  virtual void num_float(float n) {
    const char *match = *tokens++;

    if (match == 0) {
      error();
    } else if (status == INCOMPLETE) {
      float match_n = (float) atof(match);
      if (n != match_n) error();
    } else {
      error();
    }
  }

  virtual void error() {
    status = ERROR;
  }

  bool succeeded() {
    return status == INCOMPLETE && tokens[0] == 0;
  }
};

class JSONTest : public ::testing::Test {
protected:
  Visitor visitor;
  Reader reader;
  char token_buffer[100];

  JSONTest() :
    Test(),
    reader(token_buffer, sizeof(token_buffer))
  {
  }

public:
  bool parse(const char *text) {
    reader.reset(&visitor);
    reader.read(text, strlen(text));

    bool done = reader.is_done();
    bool error = reader.had_error();

    return done && !error;
  }

  bool parse(const char *text, const char *check[]) {
    ReplayVisitor replay;

    replay.set_tokens(check);
    reader.reset(&replay);
    reader.read(text, strlen(text));

    bool done = reader.is_done();
    bool error = reader.had_error();

    return done && !error && replay.succeeded();
  }
};

TEST_F(JSONTest, Parse1) {
  EXPECT_FALSE(parse("{"));
  EXPECT_FALSE(parse("["));

  EXPECT_TRUE(parse("[]"));
  EXPECT_TRUE(parse("{}"));

  EXPECT_TRUE(parse("[1,2,3]"));
  EXPECT_TRUE(parse("[1,2,[],3]"));
  EXPECT_TRUE(parse("[1,2,{},3]"));
}

TEST_F(JSONTest, ParseEmptyArray) {
  static const char *replay[] = {"[", "]", 0};
  EXPECT_TRUE(parse("[]", replay));
}

TEST_F(JSONTest, ParseOneElementArray) {
  static const char *replay[] = {"[", "123", "]", 0};
  EXPECT_TRUE(parse("[123]", replay));
  EXPECT_TRUE(parse("  [  123  ]   ", replay));
}

TEST_F(JSONTest, ParseThreeElementArray) {
  static const char *replay[] = {"[", "1", "2", "3", "]", 0 };

  EXPECT_TRUE(parse("[1,2,3]", replay));
  EXPECT_TRUE(parse("[1, 2, 3 ]    ", replay));
}

TEST_F(JSONTest, ParseEmptyObject) {
  static const char *replay[] = {"{", "}", 0};
  EXPECT_TRUE(parse("{}", replay));
}

TEST_F(JSONTest, ParseOneMemberInt1) {
  static const char *replay[] = {"{", "foo", "456", "}", 0};
  EXPECT_TRUE(parse("{\"foo\": 456}", replay));
}

TEST_F(JSONTest, ParseOneMemberInt2) {
  static const char *replay[] = {"{", "foo", "456", "}", 0};
  EXPECT_TRUE(parse("  {  \"foo\":   456  }   ", replay));
}

TEST_F(JSONTest, ParseFloat1) {
  static const char *replay[] = {"[", "1.0", "3.1415", "-6", "-3.0E12", "]", 0};
  EXPECT_TRUE(parse("[1.0, 3.1415, -6, -3.0E12]", replay));
}

TEST(JSONTestCase, WriteEmptyArray) {
  StringBufWriter sbw;

  sbw.array_begin();
  sbw.array_end();

  EXPECT_TRUE("[]" == sbw.str());
}

TEST(JSONWriterTest, WriteSingleIntegerArray) {
  StringBufWriter sbw;

  sbw.array_begin();
  sbw.num_int(123);
  sbw.array_end();

  EXPECT_TRUE("[123]" == sbw.str());
}

TEST(JSONWriterTest, WriteIntegersArray) {
  StringBufWriter sbw;

  sbw.array_begin();
  sbw.num_int(123);
  sbw.num_int(456);
  sbw.array_end();

  EXPECT_TRUE("[123, 456]" == sbw.str());
}

TEST(JSONWriterTest, WriteNumbersArray) {
  StringBufWriter sbw;

  sbw.array_begin();
  sbw.num_int(-123);
  sbw.num_float((float) -456.789);
  sbw.array_end();

  EXPECT_TRUE("[-123, -456.789]" == sbw.str());
}

TEST(JSONWriterTest, WriteScientificNumbersArray) {
  StringBufWriter sbw;

  sbw.array_begin();
  sbw.num_float((float) -123E10);
  sbw.num_float((float) -456.789E-10);
  sbw.array_end();

  EXPECT_TRUE("[-1.23e+12, -4.56789e-08]" == sbw.str());
}

TEST(JSONWriterTest, WriteEmptyObject) {
  StringBufWriter sbw;

  sbw.object_begin();
  sbw.object_end();

  EXPECT_TRUE("{}" == sbw.str());
}

TEST(JSONWriterTest, WriteSingletonObject) {
  StringBufWriter sbw;

  sbw.object_begin();
  sbw.member_name("foo");
  sbw.string("bar");
  sbw.object_end();

  EXPECT_TRUE("{\"foo\" : \"bar\"}" == sbw.str());
}

TEST(JSONWriterTest, WriteObject2Members) {
  StringBufWriter sbw;

  sbw.object_begin();
  sbw.member_name("foo");
  sbw.num_int(-9999999);
  sbw.member_name("bar");
  sbw.string("zippity do dah!");
  sbw.object_end();

  EXPECT_TRUE("{\"foo\" : -9999999, \"bar\" : \"zippity do dah!\"}" == sbw.str());
}

TEST(JSONWriterTest, WriteObject2MembersWithOneArray) {
  StringBufWriter sbw;

  sbw.object_begin();
  sbw.member_name("foo");
  sbw.num_int(-9999999);
  sbw.member_name("bar");
  sbw.array_begin();
  sbw.string("zippity do dah!");
  sbw.array_end();
  sbw.object_end();

  EXPECT_TRUE("{\"foo\" : -9999999, \"bar\" : [\"zippity do dah!\"]}" == sbw.str());
}
