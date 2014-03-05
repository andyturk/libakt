#include "akt/json/reader.h"

using namespace akt::json;

enum state_t {
  // whitespace is significant for these states
  ERROR,
  GATHER_FIRST_WHOLE_DIGIT,
  GATHER_WHOLE_DIGITS,
  GATHER_FIRST_FRACTIONAL_DIGIT,
  GATHER_FRACTIONAL_DIGITS,
  GATHER_EXPONENT,
  GATHER_FIRST_EXPONENT_DIGIT,
  GATHER_EXPONENT_DIGITS,
  GATHER_KEYWORD,
  GATHER_STRING_ESCAPED,
  GATHER_UNICODE_DIGITS,
  GATHER_STRING,

  // whitespace is ignored in these states
  EXPECT_OBJECT_OR_ARRAY,
  EXPECT_NAME,
  EXPECT_MEMBER_SEPARATOR,
  EXPECT_NEXT_MEMBER,
  EXPECT_NEXT_ELEMENT,
  EXPECT_VALUE,
  COMPLETE,
};

Reader::Reader(char *token_buffer, unsigned len) {
  token.buffer = token_buffer;
  token.max = len;
  token.pos = 0;
}

bool Reader::is_done() const {
  return state == COMPLETE || state == ERROR;
}

bool Reader::had_error() const {
  return state == ERROR;
}

void Reader::push(int state) {
  if (!stack.full()) {
    stack.push(state);
  } else {
    error();
  }
}

void Reader::pop(int &state) {
  if (!stack.empty()) {
    stack.pop(state);
  } else {
    error();
  }
}

void Reader::reset(Visitor *visitor) {
  stack.reset();
  state = EXPECT_OBJECT_OR_ARRAY;
  delegate = visitor;
}

void Reader::error() {
  state = ERROR;
  stack.reset();
  if (delegate) delegate->error();
}

inline bool Reader::is_whitespace(char ch) {
  switch (ch) {
  case ' ' : case '\r' : case '\n' : case '\t' : return true;
  default  : return false;
  }
}

void Reader::read(const char *text, unsigned len) {
  const char *limit = text + len;

  if (delegate == 0) return;

  while (text < limit) {
    const char ch(*text++);

    if (ch == '\0') {
      if (state != COMPLETE) error();
      return;
    } else if (state > GATHER_STRING && is_whitespace(ch)) {
      continue;
    } else {
      switch (state) {
      case ERROR :
        return;

      case COMPLETE :
        // any non-whitespace character in the COMPLETE state leads to an error
        error();
        break;

      case EXPECT_OBJECT_OR_ARRAY :
        push(COMPLETE);

        if (ch == '{') {
          delegate->object_begin();
          state = EXPECT_NAME;
        } else if (ch == '[') {
          delegate->array_begin();
          state_after_value = EXPECT_NEXT_ELEMENT;
          state = EXPECT_VALUE;
        } else {
          error();
        }
        break;

      case EXPECT_NAME :
        if (ch == '"') {
          string_is_name = true;
          state_after_value = EXPECT_MEMBER_SEPARATOR;
          state = GATHER_STRING;
        } else if (ch == '}') {
          delegate->object_end();
          pop(state);
        } else {
          error();
        }
        break;

      case EXPECT_MEMBER_SEPARATOR :
        if (ch == ':') {
          state_after_value = EXPECT_NEXT_MEMBER;
          state = EXPECT_VALUE;
        } else {
          error();
        }
        break;

      case EXPECT_NEXT_MEMBER :
        if (ch == ',') {
          state = EXPECT_NAME;
        } else if (ch == '}') {
          delegate->object_end();
          pop(state);
        } else {
          error();
        }
        break;

      case EXPECT_NEXT_ELEMENT :
        if (ch == ',') {
          state_after_value = EXPECT_NEXT_ELEMENT;
          state = EXPECT_VALUE;
        } else if (ch == ']') {
          delegate->array_end();
          pop(state);
        } else {
          error();
        }
        break;

      case EXPECT_VALUE :
        switch (ch) {
        case '{' :
          delegate->object_begin();
          push(state_after_value);
          state_after_value = ERROR; // should never be used
          state = EXPECT_NAME;
          break;

        case '[' :
          delegate->array_begin();
          push(state_after_value);
          state_after_value = EXPECT_NEXT_ELEMENT;
          // state = EXPECT_VALUE;
          break;

        case '-' :
        case '+' :
          state = GATHER_FIRST_WHOLE_DIGIT;
          start_token();
          append_token(ch);
          break;

        case '0' ... '9' :
          state = GATHER_WHOLE_DIGITS;
          start_token();
          append_token(ch);
          break;

        case '"' :
          state = GATHER_STRING;
          string_is_name = false;
          start_token();
          break;

        case 'a' ... 'z' :
        case 'A' ... 'Z' :
          state = GATHER_KEYWORD;
          start_token();
          append_token(ch);
          break;

        case ']' :
          delegate->array_end();
          pop(state);
          break;
        }
        break;

      case GATHER_KEYWORD :
        switch (ch) {
        case 'a' ... 'z' :
        case 'A' ... 'Z' :
          append_token(ch);
          break;

        default :
          finish_token();
          delegate->keyword(token.buffer);
          --text; // keep this character for the next state
          state = state_after_value;
        }
        break;

      case GATHER_STRING :
        switch (ch) {
        case '\\' :
          state = GATHER_STRING_ESCAPED;
          break;

        case '"' :
          finish_token();
          if (string_is_name) {
            delegate->member_name(token.buffer);
          } else {
            delegate->string(token.buffer);
          }
          state = state_after_value;
          break;

        default :
          append_token(ch);
          break;
        }
        break;

      case GATHER_STRING_ESCAPED :
        switch (ch) {
        case '"' :
          append_token('"');
          break;

        case '\\' :
          append_token('\\');
          break;

        case '/' :
          append_token('/');
          break;

        case 'b' :
          append_token(' ');
          break;

        case 'f' :
          append_token('\f');
          break;

        case 'n' :
          append_token('\n');
          break;

        case 'r' :
          append_token('\r');
          break;

        case 't' :
          append_token('\t');
          break;

        case 'u' :
          state = GATHER_UNICODE_DIGITS;
          unicode_digit_count = 0;
          unicode_value = 0;
          break;

        default :
          error();
        }
        break;

      case GATHER_UNICODE_DIGITS :
        unicode_value *= 16;
        unicode_digit_count += 1;

        switch (ch) {
        case '0' ... '9' :
          unicode_value += (ch - '0') + 0x0;
          break;

        case 'a' ... 'f' :
          unicode_value += (ch - 'a') + 0xa;
          break;

        case 'A' ... 'F' :
          unicode_value += (ch - 'A') + 0xa;
          break;

        default :
          error();
        }

        if (unicode_digit_count == 4) {
          // PUNT: need to do something useful with the unicode value
          append_token('?');
          state = GATHER_STRING;
        }
        break;

      case GATHER_FIRST_WHOLE_DIGIT :
        switch (ch) {
        case '0' ... '9' :
          state = GATHER_WHOLE_DIGITS;
          append_token(ch);
          break;

        default :
          error();
        }
        break;


      case GATHER_WHOLE_DIGITS :
        switch (ch) {
        case '0' ... '9' :
          append_token(ch);
          break;

        case 'e' :
        case 'E' :
          state = GATHER_EXPONENT;
          append_token(ch);
          break;

        case '.' :
          state = GATHER_FIRST_FRACTIONAL_DIGIT;
          append_token(ch);
          break;

        default :
          finish_token();
          delegate->keyword(token.buffer);
          --text; // keep this character for the next state
          state = state_after_value;
        }
        break;

      case GATHER_FIRST_FRACTIONAL_DIGIT :
        switch (ch) {
        case '0' ... '9' :
          state = GATHER_FRACTIONAL_DIGITS;
          append_token(ch);
          break;

        case 'e' :
        case 'E' :
          // must have at least one fractional digit after the decimal
          error();
          break;

        default :
          finish_token();
          delegate->number(token.buffer);
          --text; // keep this character for the next state
          state = state_after_value;
        }
        break;

      case GATHER_FRACTIONAL_DIGITS :
        switch (ch) {
        case '0' ... '9' :
          append_token(ch);
          break;

        case 'e' :
        case 'E' :
          state = GATHER_EXPONENT;
          append_token(ch);
          break;

        default :
          finish_token();
          delegate->number(token.buffer);
          --text; // keep this character for the next state
          state = state_after_value;
        }
        break;

      case GATHER_EXPONENT :
        switch (ch) {
        case '-' :
        case '+' :
          state = GATHER_FIRST_EXPONENT_DIGIT;
          append_token(ch);
          break;

        case '0' ... '9' :
          state = GATHER_EXPONENT_DIGITS;
          append_token(ch);
          break;

        default :
          finish_token();
          delegate->number(token.buffer);
          --text; // keep this character for the next state
          state = state_after_value;
          break;
        }
        break;

      case GATHER_FIRST_EXPONENT_DIGIT :
        switch (ch) {
        case '0' ... '9' :
          state = GATHER_EXPONENT_DIGITS;
          append_token(ch);
          break;

        default :
          finish_token();
          delegate->number(token.buffer);
          --text; // keep this character for the next state
          state = state_after_value;
          break;
        }
        break;

      case GATHER_EXPONENT_DIGITS :
        switch (ch) {
        case '0' ... '9' :
          append_token(ch);
          break;

        default :
          finish_token();
          delegate->number(token.buffer);
          --text; // keep this character for the next state
          state = state_after_value;
          break;
        }
        break;
      }
    }
  }
}

void Reader::start_token() {
  token.pos = 0;
}

void Reader::append_token(char ch) {
  if (token.pos < sizeof(token.buffer)) {
    token.buffer[token.pos++] = ch;
  } else {
    error();
  }
}

void Reader::finish_token() {
  if (token.pos < sizeof(token.buffer)) {
    token.buffer[token.pos++] = '\0';
  } else {
    error();
  }
}
