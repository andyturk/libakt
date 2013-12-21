// -*- Mode:C++ -*-

#pragma once

#include "akt/views/views.h"
#include <stdint.h>

#include "hal.h"
#include "ch.h"

namespace akt {
  namespace views {
    class SSD1306 : public Canvas {
      enum {W=128, H=64};
      uint8_t pages[H/8][W];
      IO &io;

    public:
      SSD1306(IO &io);

      void reset();
      void test();
      void test1();
      void flush();
      void flush(const Rect &r) { flush(); }

    protected:
      virtual void set_pixel(Point p, pixel value) override;
      virtual pixel get_pixel(Point p) const override;
    };
  }
}
