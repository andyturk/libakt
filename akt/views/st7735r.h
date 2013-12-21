// -*- Mode:C++ -*-

#pragma once

#include "akt/views/views.h"

namespace akt {
  namespace views {
    class ST7735 : public Canvas {
      enum {W=160, H=128};
      uint16_t buffer[H][W];
      IO &io;
      coord col_start, row_start;

    protected:
      void set_window(coord x, coord y, coord w, coord h);
      virtual void set_pixel(Point p, pixel value);
      virtual pixel get_pixel(Point p) const;

    public:
      ST7735(IO &io);

      void reset();
      void flush(const Rect &r);
    };

#if 0
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
#endif
  };
};
