// -*- Mode:C++ -*-
#pragma once

#if defined(__APPLE__) || defined(_WIN32) || defined(__x86_64__)
#define USE_JSON_STREAMS
#endif

#include "akt/json/visitor.h"
#include "akt/stack.h"

#if defined(USE_JSON_STREAMS)
#include <ostream>
#include <sstream>
#endif

namespace akt {
  namespace json {
    class WriterBase : public Visitor {
      bool had_error, skip_next_comma;
      Stack<unsigned, 100> stack;

    protected:
      virtual void write(char c) = 0;
      virtual void write(const char *str) = 0;
      virtual void write(const char *bytes, unsigned len) = 0;

      void write_quoted(const char *str);
      void write_comma_if_necessary();

    public:
      WriterBase();

      virtual void reset();
      virtual void object_begin();
      virtual void object_end();
      virtual void array_begin();
      virtual void array_end();
      virtual void member_name(const char *text);
      virtual void string(const char *text);
      virtual void literal_true();
      virtual void literal_false();
      virtual void literal_null();
      virtual void num_int(int32_t n);
      virtual void num_float(float n);
      virtual void error();
    };

#if defined(USE_JSON_STREAMS)
    class StreamWriter : public WriterBase {
      std::ostream &out;

    public:
      StreamWriter(std::ostream &out) :
        WriterBase(),
        out(out)
      {
      }

      virtual void write(char c) override { out << c; }
      virtual void write(const char *str) override { out << str; }
      virtual void write(const char *bytes, unsigned len) override { out.write(bytes, len); }
    };

    class StringBufWriter : public StreamWriter {
      std::stringbuf buffer;
      std::ostream s;

    public:
      StringBufWriter() :
        StreamWriter(s),
        s(&buffer)
      {
      }

      std::string str() { return buffer.str(); }
    };
#endif
  }
}
