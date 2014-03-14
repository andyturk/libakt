// -*- Mode:C++ -*-

#pragma once

#include "akt/json/reader.h"
#include "akt/json/writer.h"

#include "ff.h"

namespace akt {
  namespace json {
    class FATFSReader : public Visitor {
      Reader reader;
      char token_buffer[256];
      char file_buffer[256];
      FIL fil;

    public:
      FATFSReader();

      // returns true if the file was read successfully
      bool read_file(const char *filename);
    };

    class FATFSWriter : public WriterBase {
      FIL fil;

    protected:
      virtual void write(char c) override;
      virtual void write(const char *str) override;
      virtual void write(const char *bytes, unsigned len) override;

    public:
      FATFSWriter();

      bool write_file(const char *filename);
      void close();
    };
  }
}
