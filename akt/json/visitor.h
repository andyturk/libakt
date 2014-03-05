// -*- Mode:C++ -*-
#pragma once

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
      virtual void keyword(const char *text) = 0;
      virtual void number(const char *text) = 0;
      virtual void error() = 0;
    };
  }
}


