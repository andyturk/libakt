#include <akt/json/reader.h>
#include <akt/json/visitor.h>

#include <gtest/gtest.h>
#include <cstring>
#include <cstdlib>

using namespace akt::json;

class SimpleVisitor : public Visitor {
public:
  SimpleVisitor() {
  }

  virtual void object_begin() override {
  }

  virtual void object_end() override {
  }

  virtual void array_begin() override {
  }

  virtual void array_end() override {
  }

  virtual void member_name(const char *text) override {
  }

  virtual void string(const char *text) override {
  }

  virtual void literal_true() override {
  }

  virtual void literal_false() override {
  }

  virtual void literal_null() override {
  }

  virtual void number(int32_t n) override {
  }

  virtual void number(float n) override {
  }

  virtual void error() override {
  }
};

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

  virtual void number(int32_t n) {
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

  virtual void number(float n) {
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
  SimpleVisitor visitor;
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
