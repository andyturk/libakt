// -*- Mode:C++ -*-

#pragma once

#include <stdint.h>

namespace akt {
  namespace bluetooth {
    extern const char hex_digits[16];

    struct BD_ADDR {
      uint8_t data[6];
      enum {
        PRETTY_SIZE = 19 // sizeof(data)*3 + 1
      };

      const char *pretty_print(char *buf) {
        char *p = buf;

        for (int i=5; i >= 0; --i) {
          *p++ = hex_digits[data[i] >> 4];
          *p++ = hex_digits[data[i] & 0x0f];
          if (i > 0) *p++ = ':';
        }
        *p++ = 0;

        return buf;
      }
    };
  };
};
