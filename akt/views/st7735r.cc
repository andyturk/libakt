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
      END     = 0xFF, // not sent to the device
    };

#define CMD(opcode,argc,delay) opcode, argc, delay

    /*
      static uint8_t st7735b_init[] = {
      // reset
      CMD(SWRESET, 0, 50),
      // out of sleep mode, 255 = 500ms delay
      CMD(SLPOUT, 0, 255),
      // use 16-bit color
      CMD(COLMOD, 1, 10), 0x05,
      // fastest refresh, 6 lines from port, 3 lines back porch
      CMD(FRMCTR1, 3, 10), 0x00, 0x06, 0x03,
      // Memory access ctrl (directions), Row addr/col addr, bottom to top refresh
      CMD(MADCTL, 1, 0), 0x08,
      // Display settings, 1 clk cycle nonoverlap, 2 cycle gate rise, 3 cycle osc equalize, fix on VTL
      CMD(DISSET5, 2, 0), 0x15, 0x02,
      // Display inversion control, line inversion
      CMD(INVCTR, 1, 0), 0x0,
      // Power control, GVDD = 4.7v, 1.0uA
      CMD(PWCTR1, 2, 10), 0x02, 0x70,
      // Power control, VGH = 14.7v, VGL = -7.35v
      CMD(PWCTR2, 1, 0), 0x05,
      // Power control, opamp current small, boost freq
      CMD(PWCTR3, 2, 0), 0x01, 0x02,
      // Power control, VCOMH = 4v, VCOML = -1.1v
      CMD(VMCTR1, 2, 10), 0x3c, 0x38,
      // Power control
      CMD(PWCTR6, 2, 0), 0x11, 0x15,
      // Magic
      CMD(GMCTRP1, 16, 0), 0x09, 0x16, 0x09, 0x20, 0x21, 0x1B, 0x13, 0x19, 0x17, 0x15, 0x1E, 0x2B, 0x04, 0x05, 0x02, 0x0E,
      // Magic
      CMD(GMCTRN1, 16, 10), 0x0B, 0x14, 0x08, 0x1E, 0x22, 0x1D, 0x18, 0x1E, 0x1B, 0x1A, 0x24, 0x2B, 0x06, 0x06, 0x02, 0x0F,
      // Column addr set
      CMD(CASET, 4, 0), 0x00, 0x02, 0x00, 0x81,
      // Row addr set
      CMD(RASET, 4, 0), 0x00, 0x02, 0x00, 0x81,
      // Normal display on
      CMD(NORON, 0, 10),
      // Main screen turn on
      CMD(DISPON, 0, 255),
      };
    */

    static uint8_t st7735r_init_1_red_or_green[] = {
      // reset
      CMD(SWRESET, 0, 150),
      // out of sleep mode, 255 = 500ms delay
      CMD(SLPOUT, 0, 255),
      // frame rate control, normal mode = fosc/(1x2+40)*(LINE+2c+2d)
      CMD(FRMCTR1, 3, 0), 0x01, 0x2c, 0x2d,
      // frame rate control, idle mode = fosc/(1x2+40) * (LINE+2c+2d)
      CMD(FRMCTR2, 3, 0), 0x01, 0x2c, 0x2d,
      // frame rate control, partial mode (3 bytes for dot inversion mode, 3 bytes for line inversion mode)
      CMD(FRMCTR3, 6, 0), 0x01, 0x2c, 0x2d, 0x01, 0x2c, 0x2d,
      // display inversion control: none
      CMD(INVCTR, 1, 0), 0x07,
      // power control (??, -4.6v, auto mode)
      CMD(PWCTR1, 3, 0), 0xa2, 0x02, 0x84,
      // power control (VGH25 = 2.4c, VGSEL = -10, VGH = 3*AVDD)
      CMD(PWCTR2, 1, 0), 0xc5,
      // power control (opamp current: small, boost freq)
      CMD(PWCTR3, 2, 0), 0x0a, 0x00,
      // power control (BCLK/2, opamp current small & medium low)
      CMD(PWCTR4, 2, 0), 0x8a, 0x2a,
      // power control
      CMD(PWCTR5, 2, 0), 0x8a, 0xee,
      // power control
      CMD(VMCTR1, 1, 0), 0x0e,
      // no inversion
      CMD(INVOFF, 0, 0),
      // memory access control, row addr/ col addr, bottom to top refresh
      CMD(MADCTL, 1, 0), 0xa8, // 0x68
      // use 16-bit color
      CMD(COLMOD, 1, 0), 0x05,
    };

#if 0
    static uint8_t st7735r_init_2_green_only[] = {
      // column addr set, XSTART = 0, XEND = 127
      CMD(CASET, 4, 0), 0x00, 0x02, 0x00, (0x7f + 0x02),
      // row addr set, XSTART = 0, XEND = 159
      CMD(RASET, 4, 0), 0x00, 0x01, 0x00, (0x9f + 0x01),
    };
#endif

    static uint8_t st7735r_init_2_red_only[] = {
      // column addr set, XSTART = 0, XEND = 127
      CMD(CASET, 4, 0), 0x00, 0x00, 0x00, 0x7f,
      // column addr set, XSTART = 0, XEND = 159
      CMD(RASET, 4, 0), 0x00, 0x00, 0x00, 0x9f,
    };

    static uint8_t st7735r_init_3_red_or_green[] = {
      // magic
      CMD(GMCTRP1, 16, 0), 0x02, 0x1c, 0x07, 0x12, 0x37, 0x32, 0x29, 0x2d, 0x29, 0x25, 0x2B, 0x39, 0x00, 0x01, 0x03, 0x10,
      // magic
      CMD(GMCTRN1, 16, 0), 0x03, 0x1d, 0x07, 0x06, 0x2E, 0x2C, 0x29, 0x2D, 0x2E, 0x2E, 0x37, 0x3F, 0x00, 0x00, 0x02, 0x10,
      // normal display on
      CMD(NORON, 0, 10),
      // main screen on
      CMD(DISPON, 0, 100),
    };

    void ST7735::init() {
      spiStart(&spi, &config);
    }

    void ST7735::write_init_commands(uint8_t *data, size_t len) {
      uint8_t *limit = data + len;

      begin_spi();
      while (data < limit) {
        send_commands();
        send(data++, 1);

        size_t argc = *data++;
        uint8_t delay = *data++;

        if (argc > 0) {
          send_data();
          send(data, argc);
          data += argc;
        }

        switch (delay) {
        case 255 :
          chThdSleepMilliseconds(500);
          break;

        default :
          chThdSleepMilliseconds(delay);

        case 0:
          break;
        }
      }
      end_spi();
    }

    ST7735::ST7735(SPIDriver &d, const SPIConfig &c, uint16_t dc, uint16_t rs) :
      Canvas(&buffer, Size(160, 128)),
      SPIDisplay(d, c, dc, rs)
    {
      col_start = row_start = 0;
    }

    void ST7735::reset() {
      palSetPad(config.ssport, reset_bit);
      palClearPad(config.ssport, dc_bit);
      chThdSleepMilliseconds(200);
      spiSelect(&spi);
      palClearPad(config.ssport, reset_bit);
      chThdSleepMilliseconds(200);
      palSetPad(config.ssport, reset_bit);
      spiUnselect(&spi);
      chThdSleepMilliseconds(200);
  
      write_init_commands(st7735r_init_1_red_or_green, sizeof(st7735r_init_1_red_or_green));
      write_init_commands(st7735r_init_2_red_only, sizeof(st7735r_init_2_red_only));
      write_init_commands(st7735r_init_3_red_or_green, sizeof(st7735r_init_3_red_or_green));
    }

    void ST7735::send(uint8_t *data, size_t len) {
      spiSend(&spi, len, data);
    }

    void ST7735::send(uint8_t data) {
      spiSend(&spi, 1, &data);
    }

    void ST7735::set_window(coord x, coord y, coord w, coord h) {
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
    }

    void ST7735::flush(const Rect &r) {
      begin_spi();
      coord xlim = r.max.x, ylim = r.max.y, w = r.width();
      set_window(r.min.x, r.min.y, xlim-1, ylim-1);
      for (int y=r.min.y; y < ylim; ++y) {
        send((uint8_t *) (buffer.storage + y*buffer.size.w + r.min.x), w*sizeof(uint16_t));
      }
      end_spi();
    }
  };
};
