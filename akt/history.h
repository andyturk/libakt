// -*- Mode:C++ -*-
#pragma once

#include <cstring>

namespace akt {
  template<class T, unsigned N>
  class History {
    T data[N];
    bool full;
    unsigned idx;

  public:
    enum {SIZE=N};

    History() {
      reset();
    }

    unsigned count() const {
      return full ? N : idx;
    }

    void reset() {
      idx = 0;
      full = false;
      for (unsigned i=0; i < N; ++i) data[i] = 0;
    }

    History &operator+=(T sample) {
      data[idx++] = sample;
      if (idx >= N) {
        idx = 0;
        full = true;
      }

      return *this;
    }

    T &operator[](int n) {
      return data[(idx - (n + 1)) % N];
    }
  };
};
