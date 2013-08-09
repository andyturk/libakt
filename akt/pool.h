// -*- Mode:C++ -*-

#pragma once

#include "akt/assert.h"
#include "akt/ring.h"

#include <stdint.h>

namespace akt {
  template<class T> class PoolBase {
  public:
    Ring<T> available;
    const uint32_t capacity;

    PoolBase(uint32_t cap) : capacity(cap) {}

    T *allocate() {
      T *p = available.begin();
      if (p == available.end()) return 0;

      p->join(p);
      p->reset();

      return p;
    }

    void deallocate(T *p) {
      assert(p != 0);
      ((Ring<T> *) p)->join(&available);
    }
  };

  template<class T, unsigned int S> class Pool : public PoolBase<T> {
  protected:
    T pool[S];

  public:
    Pool() :
      PoolBase<T>(S) {
      reset();
    }

    void reset() {
      for (unsigned int i=0; i < S; ++i) {
        Ring<T> *p = (Ring<T> *) (pool + i);
        p->join(&this->available);
      }
    }
  };
};
