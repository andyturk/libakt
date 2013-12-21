#include "akt/views/ssd1306.h"

#include "hal.h"
#include "ch.h"

namespace akt {
  namespace views {
    SSD1306::SSD1306(IO &io) :
      Canvas(Size(W, H)),
      io(io)
    {
      for (unsigned page=0; page < H/8; ++page) {
        for (unsigned col=0; col < W; ++col) {
          pages[page][col] = 0;
        }
      }
    }

    void SSD1306::reset() {
      static const uint8_t sequence[] = {
        VIEW_IO_UNSELECT,
        VIEW_IO_RESET(1),
        VIEW_IO_SLEEP(10),
        VIEW_IO_COMMANDS,
        VIEW_IO_SELECT,
        0xae,          // display off
        0xd5, 0x80,    // clock divide ratio
        0xa8, 0x3f,    // multiplex ratio
        0xd3, 0x00,    // display offset
        0x40,          // display start line
        0x8d, 0x14,    // charge pump
        0x20, 0x00,    // horizontal memory mode
        0xa1,          // segment remap
        0xc8,          // com output scan direction
        0xda, 0x12,    // com pins hardware configuration (2nd byte = 0x02 for 32 rows)
        0x81, 0x8f,    // contrast control
        0xd9, 0xf1,    // pre-charge period
        0xdb, 0x40,    // VCOMH deselect level
        0xa4,          // display pixels from RAM
        0xa6,          // display normal (1==on)
        0xaf,          // display on
        VIEW_IO_UNSELECT,
        VIEW_IO_END
      };

      io.interpret(sequence);
    }

    void SSD1306::test() {
      static const uint8_t sequence[] = {
        VIEW_IO_COMMANDS,
        VIEW_IO_SELECT,

        0x21, 0, 127,  // set column start=0, end=127
        0x22, 0, 7,    // set page start=0, end=7

        VIEW_IO_DATA(8),  // send eight bytes
        0xaa, 0x55, 0xaa, 0x55,
        0xaa, 0x55, 0xaa, 0x55,

        VIEW_IO_COMMANDS,
        VIEW_IO_UNSELECT,
        VIEW_IO_END
      };

      io.interpret(sequence);
    }

    void SSD1306::test1() {
      draw_pixel(Point(0, 0), 1);
      draw_pixel(Point(10, 10), 1);

      for (int i=15; i < 128; i += 16) {
        draw_line(Point(i, 0), Point(i, 10 + i/8), 1);
      }

      draw_line(Point(0, 0), Point(127, 63), 1);
      draw_line(Point(0, 63), Point(127, 0), 1);
    }

    void SSD1306::set_pixel(Point p, pixel value) {
      assert(p.x >= 0 && p.x < (int16_t) W);
      assert(p.y >= 0 && p.y < (int16_t) H);

      uint8_t mask = 0x01 << (p.y % 8);

      if (value) {
        pages[p.y/8][p.x] |=  mask;
      } else {
        pages[p.y/8][p.x] &= ~mask;
      }
    }

    pixel SSD1306::get_pixel(Point p) const {
      assert(p.x >= 0 && p.x < (int16_t) W);
      assert(p.y >= 0 && p.y < (int16_t) H);

      uint8_t mask = 0x01 << (p.y % 8);

      return (pages[p.y/8][p.x] & mask) ? 1 : 0;
    }

    void SSD1306::flush() {
      static const uint8_t sequence0[] = {
        VIEW_IO_COMMANDS,
        VIEW_IO_SELECT,

        0x21, 0, 127,  // set column start=0, end=127
        0x22, 0, 7,    // set page start=0, end=7

        VIEW_IO_DATA(0),  // setup for DMA
        VIEW_IO_END
      };

      io.interpret(sequence0);
      io.send((const uint8_t *) pages, sizeof(pages));

      static const uint8_t sequence1[] = {
        VIEW_IO_UNSELECT,
        VIEW_IO_END
      };

      io.interpret(sequence1);
    }
  }
}
