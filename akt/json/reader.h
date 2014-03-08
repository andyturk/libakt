// -*- Mode:C++ -*-
#pragma once

#include "akt/json/visitor.h"
#include "akt/stack.h"

namespace akt {
  namespace json {
    class Reader {
      struct {
        char *buffer;
        unsigned pos;
        unsigned max;
      } token;

      int state, state_after_value;
      bool string_is_name;
      unsigned unicode_digit_count;
      unsigned long unicode_value;
      Stack<int, 100> stack;
      void push(int s);
      void pop(int &s);

      Visitor *delegate;

      void error();
      void start_token();
      void append_token(char ch);
      void finish_token();
      void handle_keyword();
      void handle_integer();
      void handle_number();

      bool is_whitespace(char ch);

    public:
      Reader(char *token_buffer, unsigned len);

      void reset(Visitor *delegate);
      void read(const char *text, unsigned len);
      bool is_done() const;
      bool had_error() const;
    };
  }
}
