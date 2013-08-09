// -*- Mode:C++ -*-

#pragma once

#include <stdint.h>

namespace akt::bluetooth {
  struct UUID {
    uint8_t data[16];

    UUID() {}
    UUID(const char *s); // e.g., UUID("00001234-0000-1000-8000-00805F9B34FB")
    UUID(uint16_t shortened);
    UUID(const UUID &other);

    bool is_16bit() const;
    UUID &operator=(const uint8_t *other) {
      memcpy(data, other, sizeof(data));
      return *this;
    }
      UUID &operator=(uint16_t other);
    operator uint16_t() const {
      return data[12] + (data[13] << 8);
    }
    operator uint8_t const *() const {
      return data;
    }
    bool operator==(uint16_t other) const {
      return is_16bit() && (other == (uint16_t) *this);
    }
    bool operator!=(uint16_t other) const {
      return !(*this == other);
    }
    bool operator==(const UUID &other) const {
      return 0==compare(*this, other);
    }
    bool operator!=(const UUID &other) const {
      return !(*this == other);
    }
    const char *pretty_print();

    static int compare(const UUID &u1, const UUID &u2);
  };
};
