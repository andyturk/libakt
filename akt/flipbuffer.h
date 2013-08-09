// -*- Mode:C++ -*-

#pragma once

#include "akt/assert.h"

#include <cstring>
#include <stdint.h>

/**
 * @file    flipbuffer.h
 * @brief   Fixed-length read/write buffer class
 *
 * @details The FlipBuffer class manages a fixed-sized block of memory
 *          that can be either read from or written to. It's designed to
 *          eliminate unecessary copying when the data is passed from the
 *          producer to the consumer. First, the producer fills the buffer
 *          with data. The flip method must be called before the data in
 *          the buffer can be used.
 */

namespace akt {
  template<class T> class FlipBuffer {
  protected:
    T *storage;
    unsigned cap, pos, lim;

  public:
  FlipBuffer() : storage(0), cap(0), pos(0), lim(0) {}
  FlipBuffer(T *s, unsigned c) : storage(s), cap(c), pos(0), lim(c) {}
    unsigned capacity() const {return cap;}
    unsigned position() const {return pos;}
    unsigned tell() const {return pos;}
    unsigned limit() const {return lim;}
    unsigned remaining() const {return lim - pos;}

    T &operator   *() const {return storage  [pos];}
    operator    T *() const {return storage + pos;}
    operator void *() const {return storage + pos;}

    void init(T *s, unsigned c) {
      storage = s;
      lim = cap = c;
      pos = 0;
    }

    void limit(unsigned l) {
      assert(l <= cap);
      lim = l;
    }

    void reset(unsigned l=0) {
      assert(l <= cap);
      pos = 0;
      lim = (l == 0) ? cap : l;
    }

    void flip() {
      lim = pos;
      pos = 0;
    }

    void rewind(unsigned p = 0) {
      pos = p;
    }

    void seek(unsigned p) {
      assert(p <= lim);
      pos = p;
    }
    
    T operator++() { // prefix form
      assert(pos < lim-1);
      return storage[++pos];
    }

    T operator++(int dummy) { // postfix form
      assert(pos < lim);
      return storage[pos++];
    }

    T &operator[](int offset) const {
      assert(pos+offset <= lim);
      return storage[pos+offset];
    }

    FlipBuffer &operator+=(int offset) {
      assert(pos+offset <= lim);
      pos += offset;
      return *this;
    }
    
    void /*__attribute__ ((deprecated))*/ skip(int offset) {
      assert(pos+offset <= lim);
      pos += offset;
    }
  
    FlipBuffer &operator<<(T x) {
      assert(pos+sizeof(x) <= lim);
      storage[pos++] = x;
      return *this;
    }

    FlipBuffer &operator>>(T &x) {
      assert(pos+sizeof(x) <= lim);
      x = storage[pos++];
      return *this;
    }

    FlipBuffer &read(T *p, unsigned len) {
      assert(pos+len <= lim);
      memcpy(p, storage+pos, len);
      pos += len;
      return *this;
    }

    FlipBuffer &write(const T *p, unsigned len) {
      assert(pos+len <= lim);
      memcpy(storage+pos, p, len);
      pos += len;
      return *this;
    }

    /*
    FlipBuffer &operator<<(unsigned x) {
      assert(pos+sizeof(x) <= lim);
      storage[pos++] = x & 0xff;
      storage[pos++] = x >> 8;
      return *this;
    }

    FlipBuffer &operator>>(unsigned &x) {
      assert(pos+sizeof(x) <= lim);
      x = storage[pos+0] + (storage[pos+1] << 8);
      pos += 2;
      return *this;
    }

    FlipBuffer &operator<<(uint32_t x) {
      assert(pos+sizeof(x) <= lim);
      storage[pos++] = x & 0xff;
      storage[pos++] = x >> 8;
      storage[pos++] = x >> 16;
      storage[pos++] = x >> 24;
      return *this;
    }

    FlipBuffer &operator>>(uint32_t &x) {
      assert(pos+sizeof(x) <= lim);
      x = (storage[pos+0] <<  0) + (storage[pos+1] <<  8)
        + (storage[pos+2] << 16) + (storage[pos+3] << 24);
      pos += 4;
      return *this;
    }


    FlipBuffer &operator>>(char *str) {
      do {
        assert(pos <= lim);
        *str++ += storage[pos];
      } while (storage[pos++]);
      return *this;
    }

    FlipBuffer &operator<<(const char *str) {
      do {
        assert(pos <= lim);
        storage[pos] = *str++;
      } while (storage[pos++]);
      return *this;
    }
    */
  };

  template<class T, unsigned S> class SizedFlipBuffer : public FlipBuffer<T> {
  protected:
    T data[S];
  public:
    SizedFlipBuffer() : FlipBuffer<T>(data, S) {}
  };
};
