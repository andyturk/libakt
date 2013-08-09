// -*- Mode:C++ -*-

#pragma once

#include "akt/views/views.h"

namespace akt {
  namespace views {
    class ST7735 : public Canvas, protected SPIDisplay {
    public:
      Plane<uint16_t, 160, 128> buffer;

      ST7735(SPIDriver &d, const SPIConfig &c, uint16_t dc, uint16_t rs);

      virtual void reset();
      void flush(const Rect &r);
      virtual void init();

    protected:
      void send(uint8_t *data, size_t len);
      void send(uint8_t data);
      void set_window(coord x, coord y, coord w, coord h);
      void write_init_commands(uint8_t *data, size_t len);

      coord col_start, row_start;
      coord width, height;
    };
  };
};
