#pragma once

#include "akt/assert.h"
#include "akt/views/views.h"

namespace akt {
  namespace views {
    class PDC8544 : public Canvas {
      enum {W=84, H=48};
      uint8_t banks[H/8][W];
      IO &io;

    protected:
      virtual void set_pixel(Point p, pixel value);
      virtual pixel get_pixel(Point p) const;
      
    public:
      PDC8544(IO &io);

      void reset();
      void flush(const Rect &r);
    };
  };
};
