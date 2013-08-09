// -*- Mode:C++ -*-

#pragma once

#include "akt/bluetooth/h4.h"

#include "ch.h"
#include "hal.h"

namespace akt {
  namespace bluetooth {

  class Pan13xx : public H4 {
  protected:
    GPIO_TypeDef *shutdown_port;
    uint32_t shutdown_pin;

  public:
    Pan13xx(UARTDriver &u, GPIO_TypeDef *port, uint32_t bit, uint32_t cr3_flags=0) :
      H4(u, 115200, cr3_flags),
      shutdown_port(port),
      shutdown_pin(bit)
    {
    }

    // call this once to initialize GPIO pins
    void initialize() {
    }

    void disable() {
      palClearPad(shutdown_port, shutdown_pin);
    }

    virtual void reset() {
      palClearPad(shutdown_port, shutdown_pin);
      chThdSleep(MS2ST(5)); // sleep 5 ms

      H4::reset();

      palSetPad(shutdown_port, shutdown_pin);
      chThdSleep(MS2ST(150)); // sleep 150 ms
    }
  };
  };
};

