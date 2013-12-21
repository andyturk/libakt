// -*- Mode:C++ -*-

#include "akt/views/st7735r.h"

namespace akt {
  namespace views {
    enum command {
      NOP     = 0x00,
      SWRESET = 0x01,
      RDDID   = 0x04,
      RDDST   = 0x09,
      SLPIN   = 0x10,
      SLPOUT  = 0x11,
      PTLON   = 0x12,
      NORON   = 0x13,
      INVOFF  = 0x20,
      INVON   = 0x21,
      DISPOFF = 0x28,
      DISPON  = 0x29,
      CASET   = 0x2A,
      RASET   = 0x2B,
      RAMWR   = 0x2C,
      RAMRD   = 0x2E,
      PTLAR   = 0x30,
      COLMOD  = 0x3A,
      MADCTL  = 0x36,
      FRMCTR1 = 0xB1,
      FRMCTR2 = 0xB2,
      FRMCTR3 = 0xB3,
      INVCTR  = 0xB4,
      DISSET5 = 0xB6,
      PWCTR1  = 0xC0,
      PWCTR2  = 0xC1,
      PWCTR3  = 0xC2,
      PWCTR4  = 0xC3,
      PWCTR5  = 0xC4,
      VMCTR1  = 0xC5,
      RDID1   = 0xDA,
      RDID2   = 0xDB,
      RDID3   = 0xDC,
      RDID4   = 0xDD,
      GMCTRP1 = 0xE0,
      GMCTRN1 = 0xE1,
      PWCTR6  = 0xFC,
    };

    static const uint8_t st7735r_reset[] = {
      VIEW_IO_UNSELECT,
      VIEW_IO_COMMANDS,
      VIEW_IO_RESET(200),
      VIEW_IO_SLEEP(200),
      VIEW_IO_END
    };

    static const uint8_t st7735b_init[] = {
      // reset
      SWRESET,
      VIEW_IO_SLEEP(50),
      // out of sleep mode
      SLPOUT,
      // use 16-bit color
      COLMOD, 0x05,
      VIEW_IO_SLEEP(10),
      // fastest refresh, 6 line from porch, 3 lines back porch
      FRMCTR1, 0x00, 0x06, 0x03,
      VIEW_IO_SLEEP(10),
      // row addr/col addr, bottom to top refresh
      MADCTL, 0x08,
      // Display settings, 1 clk cycle nonoverlap, 2 cycle gate rise, 3 cycle osc equalize, fix on VTL
      DISSET5, 0x15, 0x02,
      // Display inversion control, line inversion
      INVCTR, 0x0,
      // Power control, GVDD = 4.7v, 1.0uA
      PWCTR1, 0x02, 0x70,
      // Power control, VGH = 14.7v, VGL = -7.35v
      PWCTR2, 0x05,
      // Power control, opamp current small, boost freq
      PWCTR3, 0x01, 0x02,
      // Power control
      PWCTR6, 0x11, 0x15,
      // Magic
      GMCTRP1, 0x09, 0x16, 0x09, 0x20, 0x21, 0x1B, 0x13, 0x19, 0x17, 0x15, 0x1E, 0x2B, 0x04, 0x05, 0x02, 0x0E,
      // Magic
      GMCTRN1, 0x0B, 0x14, 0x08, 0x1E, 0x22, 0x1D, 0x18, 0x1E, 0x1B, 0x1A, 0x24, 0x2B, 0x06, 0x06, 0x02, 0x0F,
      VIEW_IO_SLEEP(10),
      // Column addr set
      CASET, 0x00, 0x02, 0x00, 0x81,
      // Row addr set
      RASET, 0x00, 0x02, 0x00, 0x81,
      // Normal display on
      NORON,
      // Main screen turn on
      DISPON,
      VIEW_IO_SLEEP(250),
      VIEW_IO_SLEEP(250),
      VIEW_IO_END
    };

    static const uint8_t st7735r_init_1_red_or_green[] = {
      // reset
      SWRESET,
      VIEW_IO_SLEEP(150),
      // out of sleep mode, 255 = 500ms delay
      SLPOUT,
      VIEW_IO_SLEEP(250),
      VIEW_IO_SLEEP(250),
      // frame rate control, normal mode = fosc/(1x2+40)*(LINE+2c+2d)
      FRMCTR1, 0x01, 0x2c, 0x2d,
      // frame rate control, idle mode = fosc/(1x2+40) * (LINE+2c+2d)
      FRMCTR2, 0x01, 0x2c, 0x2d,
      // frame rate control, partial mode (3 bytes for dot inversion mode, 3 bytes for line inversion mode)
      FRMCTR3, 0x01, 0x2c, 0x2d, 0x01, 0x2c, 0x2d,
      // display inversion control: none
      INVCTR, 0x07,
      // power control (??, -4.6v, auto mode)
      PWCTR1, 0xa2, 0x02, 0x84,
      // power control (VGH25 = 2.4c, VGSEL = -10, VGH = 3*AVDD)
      PWCTR2, 0xc5,
      // power control (opamp current: small, boost freq)
      PWCTR3, 0x0a, 0x00,
      // power control (BCLK/2, opamp current small & medium low)
      PWCTR4, 0x8a, 0x2a,
      // power control
      PWCTR5, 0x8a, 0xee,
      // power control
      VMCTR1, 0x0e,
      // no inversion
      INVOFF,
      // memory access control, row addr/ col addr, bottom to top refresh
      MADCTL, 0xa8, // 0x68
      // use 16-bit color
      COLMOD, 0x05,
      VIEW_IO_END
    };

    static const uint8_t st7735r_init_2_green_only[] = {
      // column addr set, XSTART = 0, XEND = 127
      CASET, 0x00, 0x02, 0x00, (0x7f + 0x02),
      // row addr set, XSTART = 0, XEND = 159
      RASET, 0x00, 0x01, 0x00, (0x9f + 0x01),
    };

    static const uint8_t st7735r_init_2_red_only[] = {
      // column addr set, XSTART = 0, XEND = 127
      CASET, 0x00, 0x00, 0x00, 0x7f,
      // column addr set, XSTART = 0, XEND = 159
      RASET, 0x00, 0x00, 0x00, 0x9f,
    };

    static const uint8_t st7735r_init_3_red_or_green[] = {
      // magic
      GMCTRP1, 0x02, 0x1c, 0x07, 0x12, 0x37, 0x32, 0x29, 0x2d, 0x29, 0x25, 0x2B, 0x39, 0x00, 0x01, 0x03, 0x10,
      // magic
      GMCTRN1, 0x03, 0x1d, 0x07, 0x06, 0x2E, 0x2C, 0x29, 0x2D, 0x2E, 0x2E, 0x37, 0x3F, 0x00, 0x00, 0x02, 0x10,
      // normal display on
      NORON,
      VIEW_IO_SLEEP(10),
      // main screen on
      DISPON,
      VIEW_IO_SLEEP(100),
      VIEW_IO_END
    };


    ST7735::ST7735(IO &io) :
      Canvas(Size(160, 128)),
      io(io),
      col_start(0),
      row_start(0)
    {
    }

    void ST7735::reset() {
      io.interpret(st7735r_reset);
      io.interpret(st7735r_init_1_red_or_green);
      io.interpret(st7735r_init_2_red_only);
      io.interpret(st7735r_init_3_red_or_green);
    }

    void ST7735::set_pixel(Point p, pixel value) {
      assert(p.x >= 0 && p.x < size.w);
      assert(p.y >= 0 && p.y < size.h);
      buffer[p.y][p.x] = value;
    }

    pixel ST7735::get_pixel(Point p) const {
      assert(p.x >= 0 && p.x < size.w);
      assert(p.y >= 0 && p.y < size.h);
      return buffer[p.y][p.x];
    }

    void ST7735::set_window(coord x, coord y, coord w, coord h) {
      // these enums are just placeholers for values to be filled in later
      enum {XSTART, XEND, YSTART, YEND};

      static uint8_t sequence[] = {
        /*  0 */ VIEW_IO_COMMANDS,
        /*  2 */ VIEW_IO_SELECT,
        /*  5 */ CASET,
        /*  6 */ VIEW_IO_DATA(4), 0x00, XSTART, 0x00, XEND,
        /* 13 */ VIEW_IO_COMMANDS,
        /* 15 */ RASET,
        /* 18 */ VIEW_IO_DATA(4), 0x00, YSTART, 0x00, YEND,
        /* 23 */ VIEW_IO_COMMANDS,
        /* 25 */ RAMWR,
        /* 26 */ VIEW_IO_DATA(0),
        /* 29 */ VIEW_IO_END,
        /* 31 */
      };

      uint8_t &xstart (sequence[7]);
      uint8_t &xend   (sequence[9]);
      uint8_t &ystart (sequence[17]);
      uint8_t &yend   (sequence[19]);

      assert(sizeof(sequence) == 31);

      xstart = x + col_start;
      xend   = x + col_start + w;
      ystart = y + row_start;
      yend   = y + row_start + h;

      io.interpret(sequence);

      /*
      send_commands();
      send((uint8_t) CASET);
      send_data();
      send((uint8_t) 0);
      send((uint8_t) (x + col_start)); // XSTART
      send((uint8_t) 0);
      send((uint8_t) (x + col_start + w)); // XEND

      send_commands();
      send((uint8_t) RASET);
      send_data();
      send((uint8_t) 0);
      send((uint8_t) (y + row_start)); // YSTART
      send((uint8_t) 0);
      send((uint8_t) (y + row_start + h)); // YEND

      send_commands();
      send((uint8_t) RAMWR);
      send_data();
      */
    }

    void ST7735::flush(const Rect &r) {
      coord xlim = r.max.x, ylim = r.max.y, w = r.width();
      set_window(r.min.x, r.min.y, xlim-1, ylim-1);
      for (int y=r.min.y; y < ylim; ++y) {
        io.send((uint8_t *) &buffer[y][r.min.x], w*sizeof(uint16_t));
      }

      static const uint8_t sequence[] = {
        VIEW_IO_UNSELECT,
        VIEW_IO_END
      };

      io.interpret(sequence);
    }
  };
};
