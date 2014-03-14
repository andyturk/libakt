// -*- Mode:C++ -*-
#pragma once

#include <stdint.h>

namespace akt {
  namespace json {
    class Visitor {
    public:
      Visitor() {}

      virtual void object_begin() {}
      virtual void object_end() {}
      virtual void array_begin() {}
      virtual void array_end() {}
      virtual void member_name(const char *text) {}
      virtual void string(const char *text) {}
      virtual void literal_true() {}
      virtual void literal_false() {}
      virtual void literal_null() {}
      virtual void num_int(int32_t n) {}
      virtual void num_float(float n) {}
      virtual void error() {}
    };
  }
}


