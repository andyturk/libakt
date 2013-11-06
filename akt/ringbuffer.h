// -*- Mode:C++ -*-

#pragma once

#include <cstdlib>
#include <stdint.h>
#include <cstddef>

#include "akt/assert.h"

namespace akt {
  template<class T> class RingBuffer {
    T *const storage, *const limit;
    T *write_position, *read_position;

  public:
    RingBuffer(T *const s, size_t capacity) :
      storage(s),
      limit(storage + capacity),
      write_position(s),
      read_position(s)
    {}
  
    void flush() {
      write_position = read_position = storage;
    }

    size_t read_capacity() const {
      if (write_position >= read_position) {
        return (size_t) (write_position - read_position);
      } else {
        return (size_t) ((limit - read_position) + (write_position - storage));
      }
    }

    size_t contiguous_read_capacity() const {
      if (write_position >= read_position) {
        return (size_t) (write_position - read_position);
      } else {
        return (size_t) (limit - read_position);
      }
    }

    size_t write_capacity() const {
      if (write_position >= read_position) {
        return (size_t) (((limit - storage) - 1) - (write_position - read_position));
      } else {
        return (size_t) ((read_position - write_position) - 1);
      }
    }

    size_t read(T *dst, size_t n) {
      size_t actually_read = 0;

      while (n > 0) {
        size_t run;

        if (write_position >= read_position) {
          run = write_position - read_position;
        } else {
          run = limit - read_position;
        }

        if (run == 0) break;
        if (n < run) run = n;

        for(uint32_t i=run; i > 0; --i) *dst++ = *read_position++;
        if (read_position == limit) read_position = storage;

        n -= run;
        actually_read += run;
      };

      return actually_read;
    }

    inline T &peek(int offset) const {
      ptrdiff_t capacity = limit - storage;
      assert(abs(offset) < capacity);
      T *p = read_position + offset;
      if (p < storage) p += capacity;
      if (p > limit) p -= capacity;
      return *p;
    }

    void skip(uint32_t offset) {
      size_t cap = read_capacity();

      if (offset > cap) offset = cap;

      read_position += offset;
      if (read_position >= limit) read_position -= (limit - storage);
    }

    inline T &poke(int offset) const {
      ptrdiff_t capacity = limit - storage;
      assert(abs(offset) < capacity);
      if (offset < 0) offset += capacity;
      T *p = write_position + offset;
      if (p > limit) p -= capacity;
      if (p < storage) p += capacity;
      return *p;
    }

    size_t write(const T *src, size_t n) {
      size_t written = 0;

      while (n > 0) {
        size_t run;

        if (write_position >= read_position) {
          run = limit - write_position;
        } else {
          run = (read_position - write_position) - 1;
        }

        if (run == 0) break;
        if (n < run) run = n;

        for (uint32_t i=run; i > 0; --i) *write_position++ = *src++;
        if (write_position == limit) write_position = storage;

        n -= run;
        written += run;
      };

      return written;
    }
  };
};
