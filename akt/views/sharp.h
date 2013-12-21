// -*- Mode:C++ -*-

#pragma once

#include "akt/views/views.h"
#include <stdint.h>

namespace akt {
  namespace views {
    class SharpMemoryLCDBase : public Canvas {
    protected:
      // These constants are defined such that the SPI interface must
      // send the LSB of each byte first.
      enum {
        WRITE = 0x01,
        VCOM  = 0x02,
        CLEAR = 0x04,
        NOP   = 0x00
      };

      enum {
        SINGLE_LINE_FLUSH = 0
      };

      /*
       * each line on the display is represented by a 1- or 2-byte header
       * followed by (width+7)/8 data bytes and finally a one byte trailer.
       *
       * struct Line {
       *   struct {
       * #if SINGLE_LINE_FLUSH
       *     uint8_t command;
       * #endif
       *     uint8_t line_number;
       *   } prefix;
       *   uint8_t data[(W+7)/8];
       *   uint8_t trailer;
       * } __attribute__ ((packed));
       *
       */

      IO &io;
      display_orientation_t orientation;
      uint8_t &initial_command;
      uint8_t *lines;
      const unsigned line_data_length;
      uint8_t vcom;

    protected:
      SharpMemoryLCDBase(IO &io, Size s, uint8_t *buffer, display_orientation_t o) :
        Canvas(s),
        io(io),
        orientation(o),
        initial_command(buffer[0]),
        lines(&buffer[1]),
        line_data_length(SINGLE_LINE_FLUSH + 1 + (size.w + 7)/8 + 1),
        vcom(0)
      {
        for (int i=0; i < size.h; ++i) {
          uint8_t *line = lines + i*line_data_length;

          // setup command for the line
          if (SINGLE_LINE_FLUSH) *line++ = 0;   

          // setup line_number
          *line++ = i+1;

          for (int j=0; j < (size.w+7)/8; ++j) {
            *line++ = 0x55; // 50% gray
          }

          // setup trailer
          *line++ = 0;
        }
      }

    public:
      virtual void set_pixel(Point p, pixel value) {
        assert(p.x >= 0 && p.x < size.w);
        assert(p.y >= 0 && p.y < size.h);

        if (orientation == ROTATE180) {
          p.x = (size.w - p.x) - 1;
          p.y = (size.h - p.y) - 1;
        }

        uint8_t *line = lines + p.y*line_data_length;
        unsigned offset = 1 + SINGLE_LINE_FLUSH + p.x/8;
        uint8_t bit =  1 << (p.x & 7);

        if (value) {
          line[offset] |=  bit;
        } else {
          line[offset] &= ~bit;
        }
      }

      virtual pixel get_pixel(Point p) const {
        assert(p.x >= 0 && p.x < size.w);
        assert(p.y >= 0 && p.y < size.h);

        if (orientation == ROTATE180) {
          p.x = (size.w - p.x) - 1;
          p.y = (size.h - p.y) - 1;
        }

        uint8_t *line = lines + p.y*line_data_length;
        unsigned offset = 1 + SINGLE_LINE_FLUSH + p.x/8;
        uint8_t bit =  1 << (7 - (p.x & 0x0007));
        return (line[offset] & bit) ? 1 : 0;
      }

      virtual void flush(const Rect &r) {
        const static uint8_t sequence0[] = {
          VIEW_IO_PIN(IO::CS_PIN, true), // Sharp CS is active *high*
          VIEW_IO_SLEEP(6),
          VIEW_IO_END
        };

        const static uint8_t sequence1[] = {
          VIEW_IO_PIN(IO::CS_PIN, false), // Sharp CS is active *high*
          VIEW_IO_SLEEP(2),
          VIEW_IO_END
        };

        initial_command = WRITE | vcom;
        io.interpret(sequence0);
        io.send(&initial_command, 1 + size.h*line_data_length);
        io.interpret(sequence1);
      }

      void toggle_vcom() {
        vcom = vcom ? 0 : VCOM;
      }

      void set_display(bool value) {
        uint8_t sequence[] = {
          VIEW_IO_PIN(IO::ENABLE_PIN, value),
          VIEW_IO_END
        };

        io.interpret(sequence);
      }

      void reset() {
        uint8_t sequence[] = {
          VIEW_IO_PIN(IO::CS_PIN, true), // Sharp CS is active *high*
          (uint8_t) (CLEAR | vcom),
          0,
          0,
          VIEW_IO_PIN(IO::CS_PIN, false), // Sharp CS is active *high*
          VIEW_IO_END
        };

        io.interpret(sequence);
      }
    };

    class SharpMemoryLCD_128x128 : public SharpMemoryLCDBase {
      uint8_t buffer[1 +                                    // initial command
                     128*(SINGLE_LINE_FLUSH + 1 + 128/8 + 1) + // 128 lines of data
                     1];                                    // trailer
    public:
      SharpMemoryLCD_128x128(IO &io) :
        SharpMemoryLCDBase(io, Size(128, 128), buffer, ROTATE180)
      {
      }
    };
  };
};
