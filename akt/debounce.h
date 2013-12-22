// -*- Mode:C++ -*-

#pragma once

/**
 * \file       debounce.h
 * \class      akt::Debounce
 * \brief      Removes contact noise from a set of raw hardware switches
 * 
 * \details    A single instance of the Debounce class can remove contact noise
 *             from a bank of up to 32 individual hardware switches. External
 *             code is responsible for reading the raw switch values and placing
 *             them into a uint32_t mask on a periodic basis (e.g., every 20 msec).
 *             The uint32_t should have a 1 bit for each switch that is
 *             pressed/active and a 0 bit otherwise.
 *             Each time the update_state() method is called, the Debounce
 *             instance recalculates the debounced state and calls
 *             state_changed() when any of the switches have changed.
 */

namespace akt {
  class Debounce {
  public:
    enum {
      DWELL = 5 // as in "dwell time"
    };

    Debounce() {
      reset();
    }

    void reset() {
      next = debounced_state = 0;
      for (unsigned i=0; i < DWELL; ++i) history[i] = 0;
    }

    uint32_t update(uint32_t raw_switch_values) {
      history[next++] = raw_switch_values;
      if (next == DWELL) next = 0;

      uint32_t new_state = 0xffffffff;
      for (unsigned i=0; i < DWELL; ++i) new_state &= history[i];

      uint32_t result;

      if ((result = (new_state ^ debounced_state))) {
        debounced_state = new_state;
      }

      return result;
    }

    uint32_t state() const {
      return debounced_state;
    }

  private:
    uint32_t debounced_state;
    uint32_t history[DWELL];
    unsigned next;
  };
};
