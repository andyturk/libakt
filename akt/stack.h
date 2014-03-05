// -*- Mode:C++ -*-

#pragma once

namespace akt {
  template<class T, unsigned N>
  class Stack {
    T elements[N];
    unsigned top;

  public:
    Stack() : top(0) { }

    void               reset()       { top = 0; }
    bool               empty() const { return top == 0; }
    bool                full() const { return top == N; }
    unsigned space_remaining() const { return N - top; }

    bool push(const T &element) {
      if (full()) return false;

      elements[top++] = element;
      return true;
    }

    bool pop(T &element) {
      if (empty()) return false;

      element = elements[--top];
      return true;
    }
  };
}
