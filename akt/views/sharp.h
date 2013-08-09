// -*- Mode:C++ -*-

#pragma once

#include "akt/views/views.h"
#include <stdint.h>

namespace akt {
  namespace views {
    template<unsigned W, unsigned H>
    class SharpMemoryLCD : public Canvas {
      // These constants are defined such that the SPI interface must
      // send the LSB of each byte first.
      enum {
        WRITE_LINE = 0x01,
        VCOM       = 0x02,
        CLEAR      = 0x04,
        NOP        = 0x00
      };

      struct Line {
        uint8_t  command;
        uint8_t  number;
        uint8_t  data[(W+7)/8];
        uint16_t trailer;
      } __attribute__ ((packed));

      struct Plane : PlaneBase {
        Line lines[H]; 

      public:
        Plane() : PlaneBase(Size(W, H)) {
          for (unsigned i=0; i < H; ++i) {
            lines[i].number = i+1;
            lines[i].trailer = 0;
            for (unsigned j=0; j < (W+7)/8; ++j) {
              lines[i].data[j] = 0x55;
            }
          }
        }

        virtual void set_pixel(Point p, pixel value) {
          assert(p.x >= 0 && p.x < (int16_t) W);
          assert(p.y >= 0 && p.y < (int16_t) H);

          unsigned offset = p.x/8;
          uint8_t bit =  1 << (p.x & 7);

          if (value) {
            lines[p.y].data[offset] |=  bit;
          } else {
            lines[p.y].data[offset] &= ~bit;
          }
        }

        virtual pixel get_pixel(Point p) const {
          assert(p.x >= 0 && p.x < (int16_t) W);
          assert(p.y >= 0 && p.y < (int16_t) H);

          unsigned offset = p.x/8;
          uint8_t bit =  1 << (7 - (p.x & 0x0007));
          return (lines[p.y].data[offset] & bit) ? 1 : 0;
        }
      } plane;

      struct SharpSPIConfig : public SPIConfig {
        SharpMemoryLCD *lcd;
      } config;

      uint16_t display_pin;

      static void end_cb(SPIDriver *driver) {
        ((SharpSPIConfig *) driver->config)->lcd->end();
      }

      void send_command(uint8_t command) {
        if (vcom) command |= VCOM;
        spiSend(&spi, 1, &command);
      }

    protected:
      virtual void end() {}

      uint8_t vcom;
      SPIDriver &spi;

    public:
      SharpMemoryLCD(SPIDriver &d, ioportid_t port, uint16_t nss, uint16_t disp) :
        Canvas(&plane, Size(W, H)),
        display_pin(disp),
        vcom(false),
        spi(d)
      {
        config.end_cb = 0; // &end_cb;
        config.ssport = port;
        config.sspad = nss;
        config.lcd = this;

        config.cr1 = (SPI_CR1_BIDIOE   | // output only
                      SPI_CR1_SSM      | // software slave select
                      SPI_CR1_MSTR     | // master mode
                      SPI_CR1_LSBFIRST | // send low bits first
                      0 /* ,SPI_CR1_BR*/);       // BR bits == 1, slowest for now
      }

      void toggle_vcom() {
        vcom = vcom ? 0 : VCOM;
      }

      void send_vcom() {
        uint8_t sequence[3];

        sequence[0] = NOP | vcom;
        sequence[1] = 0;
        sequence[2] = 0;

        palSetPad(config.ssport, config.sspad);
        spiSend(&spi, sizeof(sequence), sequence);
        palClearPad(config.ssport, config.sspad);
      }

      void set_display(bool value) {
        if (value) {
          palSetPad(config.ssport, display_pin);
        } else {
          palClearPad(config.ssport, display_pin);
        }
      }

      virtual void init() {
        spiStart(&spi, &config);
      }

      virtual void reset() {
        uint8_t sequence[3];

        sequence[0] = CLEAR | vcom;
        sequence[1] = 0;
        sequence[2] = 0;

        palSetPad(config.ssport, config.sspad);
        spiSend(&spi, sizeof(sequence), sequence);
        palClearPad(config.ssport, config.sspad);
      }

      virtual void flush(const Rect &r) {
        flush();
      }

      void flush() {
        for (unsigned i=0; i < H; ++i) {
          palSetPad(config.ssport, config.sspad);
          plane.lines[i].command = WRITE_LINE | vcom;
          spiSend(&spi, sizeof(Line), (uint8_t *) &plane.lines[i]);
          palClearPad(config.ssport, config.sspad);
        }

      }
    };
  };
};
