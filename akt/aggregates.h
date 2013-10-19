// -*- Mode:C++ -*-

#pragma once

#include <limits>

namespace akt {
  template<class T>
    struct Extrema {
      struct {
        T value;
        int index;
      } max, min;
      int count;

      Extrema() {reset();}

      void reset() {
        max.value = std::numeric_limits<T>::min();
        max.index = -1;
        min.value = std::numeric_limits<T>::max();
        min.index = -1;
        count = 0;
      }

      Extrema &operator+=(T value) {
        if (value >= max.value) {
          max.value = value;
          max.index = count;
        }

        if (value <= min.value) {
          min.value = value;
          min.index = count;
        }

        count += 1;
        return *this;
      }
    };

  template<class T>
    struct MaxMinMean : public Extrema<T> {
    decltype (Extrema<T>::max.value + Extrema<T>::min.value) sum;

  public:
    MaxMinMean &operator+=(T value) {
      Extrema<T>::operator+=(value);
      sum += value;

      return *this;
    }

    void reset() {
      Extrema<T>::reset();
      sum = 0;
      this->count = 0;
    }

    T range() const {return this->max.value - this->min.value;}
    T mean() const {return this->sum/this->count;}
    T median() const {return this->min.value + range()/2;}

    T truncated_mean() const {
      switch (this->count) {
      case 0  : return 0;
      case 1  : return sum;
      case 2  : return sum/2;
      default : return (sum - (this->max.value + this->min.value))/(this->count - 2);
      }
    }
  };

  template<class T, unsigned N>
    class Histogram {
  public:
    unsigned buckets[N];
    const T min, max;
    unsigned count, highest;
    int highest_index;

    Histogram(T min, T max) : min(min), max(max) {reset();}
    Histogram() :
      Histogram(std::numeric_limits<T>::min(), std::numeric_limits<T>::max())
    {}

    void reset() {
      for (unsigned i=0; i < N; ++i) buckets[i] = 0;
      count = 0;
      highest = 0;
      highest_index = -1;
    }

    Histogram &operator+=(T sample) {
      unsigned bucket;

      if (sample <= min) {
        bucket = 0;
      } else if (sample >= max) {
        bucket = N-1;
      } else {
        bucket = ((sample-min)*N)/(max - min);
      }

      buckets[bucket] += 1;
      count += 1;

      if (buckets[bucket] > highest) {
        highest = buckets[bucket];
        highest_index = bucket;
      }

      return *this;
    }
  };

};
