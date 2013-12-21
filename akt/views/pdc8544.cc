#include "akt/views/pdc8544.h"
#include "ch.h"
#include "hal.h"

namespace akt {
  namespace views {
    PDC8544::PDC8544(IO &io) :
      Canvas(Size(W, H)),
      io(io)
    {
    }

    void PDC8544::reset() {
      // clear the frame buffer
      for (unsigned bank=0; bank < H/8; ++bank) {
        for (unsigned col=0; col < W; ++col) {
          banks[bank][col] = 0;
        }
      }

      static const uint8_t sequence[] = {
        VIEW_IO_UNSELECT,
        VIEW_IO_RESET(10),
        VIEW_IO_SLEEP(10),
        VIEW_IO_COMMANDS,

        VIEW_IO_SELECT,
        0x20 + 0x01, // set display active, horizontal addressing, H = 1
        0x80 + 80,   // set Vop = 80
        0x01 + 0x04, // set bias system = 4
        0x20,        // set display active, horizontal addressing, H = 0
        0x09,        // display all pixels on
        VIEW_IO_UNSELECT,
        VIEW_IO_END
      };

      io.interpret(sequence);
    }

    void PDC8544::set_pixel(Point p, pixel value) {
      assert(p.x >= 0 && p.x < size.w);
      assert(p.y >= 0 && p.y < size.h);

      uint8_t &byte = banks[p.y/8][p.x];
      uint8_t mask = 1 << p.y%8;
      if (value) byte |= mask; else byte &= ~mask;
    }

    pixel PDC8544::get_pixel(Point p) const {
      const uint8_t &byte = banks[p.y/8][p.x];
      uint8_t mask = 1 << p.y%8;
      return (byte & mask) != 0;
    }

    void PDC8544::flush(const Rect &r) {
      static const uint8_t sequence0[] = {
        VIEW_IO_COMMANDS,
        VIEW_IO_SELECT,
        0x20,            // power on, horizontal addressing, H=0
        0x0c,            // normal display configuration
        VIEW_IO_END
      };

      io.interpret(sequence0);

      for (int y=0; y < H/8; ++y) {
        const uint8_t sequence1[] {
          VIEW_IO_COMMANDS,
          (uint8_t) (0x40 + y),   // set y address
          0x80,                   // set x address=0
          VIEW_IO_DATA(0),
          VIEW_IO_END
        };

        io.interpret(sequence1);
        io.send(banks[y], sizeof(banks[y]));
      }

      static const uint8_t sequence2[] = {
        VIEW_IO_UNSELECT,
        VIEW_IO_END
      };

      io.interpret(sequence2);
    }
  };
};
