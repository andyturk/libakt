// -*- Mode:C++ -*-
#pragma once

#include <stdint.h>

namespace akt {
  namespace json {
    class Visitor {
    public:
      virtual void object_begin() = 0;
      virtual void object_end() = 0;
      virtual void array_begin() = 0;
      virtual void array_end() = 0;
      virtual void member_name(const char *text) = 0;
      virtual void string(const char *text) = 0;
      virtual void literal_true() = 0;
      virtual void literal_false() = 0;
      virtual void literal_null() = 0;
      virtual void number(int32_t n) = 0;
      virtual void number(float n) = 0;
      virtual void error() = 0;
    };
  }
}


