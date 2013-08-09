#include "akt/views/pdc8544.h"
#include "ch.h"
#include "hal.h"

namespace akt {
  namespace views {
    enum pcd8544_masks {
      HORIZ = 0x00,
      VERT  = 0x02,
      H0    = 0x00,
      H1    = 0x01,
      PD0   = 0x00,
      PD1   = 0x04,
    };

    enum pcd8544_display_mode {
      BLANK      = 0x00,
      NORMAL     = 0x04,
      ALL        = 0x01,
      INVERSE    = 0x05
    };

    enum pcd8544_instruction {
      // H=0 or H=1
      NOP        = 0x00,
      FUNCTION   = 0x20, // + {PD0, PD1} + {HORIZ, VERT} + {H0, H1}

      // H=0
      DISPLAY    = 0x08, // + pdc8544_display_mode
      SET_Y      = 0x40, // + 0 <= y <= 5
      SET_X      = 0x80, // + 0 <= x <= 83
  
      // H=1
      TEMP       = 0x04, // + 0 <= t <= 3
      BIAS       = 0x10, // + 0 <= b <= 7
      SET_V      = 0x80, // + 0 <= v <= 127
    };

    PDC8544::PDC8544(SPIDriver *d, const SPIConfig &c, uint16_t dc, uint16_t rs) :
      Canvas(&buffer, Size(84, 48)),
      SPIDisplay(d, c, dc, rs)
    {
    }

    void PDC8544::init() {
      spiStart(spi, &config);
    }

    void PDC8544::reset() {
      palSetPad(config.ssport, reset_bit);
      chThdSleepMilliseconds(100);
      palClearPad(config.ssport, reset_bit);
      chThdSleepMilliseconds(100);
      palSetPad(config.ssport, reset_bit);

      begin_spi();
      send_commands();
      send(FUNCTION + H1);
      send(BIAS + 0x04);
      send(SET_V + 64);
      send(FUNCTION + H0);
      send(DISPLAY + NORMAL);
      send(FUNCTION + H0);
      end_spi();
    }

    void PDC8544::flush(const Rect &r) {
      begin_spi();

      send_commands();
      send(FUNCTION + H0 + VERT);

      int y = r.min.y/8;
      for (int x=r.min.x; x < r.max.x; ++x) {
        send_commands();
        send(SET_X + x);
        send(SET_Y + y);
        send_data();
        send(buffer.bytes[x] + y, (r.max.y + 7 - r.min.y)/8);
      }
  
      end_spi();
    }
  };
};
