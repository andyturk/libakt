// -*- Mode:C++ -*-

#pragma once

namespace akt {
  template<class T, unsigned N>
  class Stack {
    T elements[N];
    unsigned tos; // top of stack

  public:
    Stack() : tos(0) { }

    void               reset()       { tos = 0; }
    bool               empty() const { return tos == 0; }
    bool                full() const { return tos == N; }
    unsigned space_remaining() const { return N - tos; }
    T                   &top()       { return elements[tos - 1]; }

    bool push(const T &element) {
      if (full()) return false;

      elements[tos++] = element;
      return true;
    }

    bool pop(T &element) {
      if (empty()) return false;

      element = elements[--tos];
      return true;
    }
  };
}
