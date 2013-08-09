// -*- Mode:C++ -*-

#pragma once

namespace akt {
  class RingBase {
  protected:
    RingBase *left;
    RingBase *right;

    static void join(RingBase *dst, RingBase *src) { // src joins dst
      // remove this from its current ring
      src->left->right = src->right;
      src->right->left = src->left;

      // form a singleton ring so that join(x,x) works
      src->left = src->right = src;

      // set up local pointers
      src->right = dst->right;
      src->left = dst;

      // splice src into dest
      dst->right->left = src;
      dst->right = src;
    }

  public:
    RingBase() { left = right = this; }
    bool empty() const { return left == this; } // only need to check one side
  };

  template<typename T>
    class Ring : public RingBase {
  public:
    class Iterator {
      Ring *p;

    public:
    Iterator() : p(0) {}
    Iterator(T *init) : p(init) {}
      Iterator &operator=(T *init) { p = init; }
      T &operator*() const { return *(T *) p; }
      T *operator->() const { return (T *) p; }
      operator T*() const { return (T *) p; }
      Iterator &operator++() { p = (Ring *) p->right; return *this; }
      Iterator &operator--() { p = (Ring *) p->left; return *this; }
      //bool operator==(const T *x) const { return p == (Ring *) x; }
      //bool operator!=(const T *x) const { return p != (Ring *) x; }
    };

  Ring() : RingBase() {}

    void join(Ring *other) { RingBase::join(other, this); }
    void join(Ring &other) { RingBase::join(&other, this); }
    T *end() { return (T *) this; }
    T *begin() { return (T *) right; }
    T *rbegin() { return (T *) left; } // reverse begin
  };
};
