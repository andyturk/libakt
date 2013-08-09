#pragma once

#include "akt/assert.h"
#include "akt/views/views.h"

namespace akt {
  namespace views {
    class PDC8544 : public Canvas, protected SPIDisplay {
    public:

      template<unsigned W, unsigned H>
        struct NokiaBitmap : public PlaneBase {
        uint8_t bytes[W][H];
      NokiaBitmap() : PlaneBase(Size(W,H)) {}

        virtual void set_pixel(Point p, pixel value) {
          assert(p.x >= 0 && p.x < size.w);
          assert(p.y >= 0 && p.y < size.h);
          uint8_t &byte = bytes[p.x][p.y/8];
          uint8_t mask = 1 << p.y%8;
          if (value) byte |= mask; else byte &= ~mask;
        }

        virtual pixel get_pixel(Point p) const {
          const uint8_t &byte = bytes[p.x][p.y/8];
          uint8_t mask = 1 << p.y%8;
          return (byte & mask) != 0;
        }
      };

      NokiaBitmap<84,48> buffer;

      PDC8544(SPIDriver *d, const SPIConfig &c, uint16_t dc, uint16_t rs);
      virtual void init();
      virtual void reset();
      void flush(const Rect &r);
    };
  };
};
