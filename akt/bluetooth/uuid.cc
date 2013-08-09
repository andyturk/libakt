#include "akt/bluetooth/uuid.h"

#include <stddef.h>

namespace akt::bluetooth {
  const uint8_t bluetooth_uuid[16] = {
    0xfb, 0x34, 0x9b, 0x5f,
    0x80, 0x00, 0x00, 0x80,
    0x00, 0x10, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00
  };

  static int from_hex(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return 10 + (c - 'a');
    if (c >= 'A' && c <= 'F') return 10 + (c - 'A');
    return -1;
  }

  UUID &UUID::operator=(uint16_t other) {
    memcpy(data, bluetooth_uuid, sizeof(data));
    data[13] = (uint8_t) (other >> 8);
    data[12] = (uint8_t) (other & 0x00ff);
    return *this;
  }

  int UUID::compare(const UUID &u1, const UUID &u2) {
    return memcmp(u1.data, u2.data, sizeof(UUID::data));
  }

  UUID::UUID(uint16_t shortened) {
    memcpy(data, bluetooth_uuid, sizeof(bluetooth_uuid));
    data[13] = (uint8_t) (shortened >> 8);
    data[12] = (uint8_t) (shortened & 0x00ff);
  }

  UUID::UUID(const UUID &other) {
    for (unsigned int i=0; i < sizeof(data); ++i) data[i] = other.data[i];
  }

  UUID::UUID(const char *s) {
    for (unsigned int i=0; i < sizeof(data); ++i) data[i] = 0;
       
    uint8_t *p = data;
    int hi, lo;

    for (int i=0; i < 4; ++i) {
      hi = from_hex(*s++);
      lo = from_hex(*s++);
      if (hi < 0 || lo < 0) return;
      *p++ = (hi << 8) + lo;
    }

    if (*p++ != '-') return;

    for (int j=0; j < 3; ++j) {
      for (int i=0; i < 4; ++i) {
        hi = from_hex(*s++);
        lo = from_hex(*s++);
        if (hi < 0 || lo < 0) return;
        *p++ = (hi << 8) + lo;
      }

      if (*p++ != '-') return;
    }

    for (int i=0; i < 6; ++i) {
      hi = from_hex(*s++);
      lo = from_hex(*s++);
      if (hi < 0 || lo < 0) return;
      *p++ = (hi << 8) + lo;
    }
  }

  bool UUID::is_16bit() const {
    return !memcmp(data, bluetooth_uuid, 12) && !memcmp(data + 14, bluetooth_uuid + 14, 2);
  }

  extern const char hex_digits[16];

  void to_hex(char *dst, uint8_t byte) {
    dst[0] = hex_digits[byte >> 4];
    dst[1] = hex_digits[byte & 0x0f];
  }

  const char *UUID::pretty_print() {
    static char buffer[40];
    char *p = buffer;

    if (is_16bit()) {
      for (int i=13; i >= 12; --i, p += 2) to_hex(p, data[i]);
    } else {
      for (int i=15; i >= 12; --i, p += 2) to_hex(p, data[i]); *p++ = '-';
      for (int i=11; i >= 10; --i, p += 2) to_hex(p, data[i]); *p++ = '-';
      for (int i= 9; i >=  8; --i, p += 2) to_hex(p, data[i]); *p++ = '-';
      for (int i= 7; i >=  6; --i, p += 2) to_hex(p, data[i]); *p++ = '-';
      for (int i= 5; i >=  0; --i, p += 2) to_hex(p, data[i]);
    }

    return buffer;
  }
};
