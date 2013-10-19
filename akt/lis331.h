// -*- Mode:C++ -*-

#pragma once

#include "ch.h"
#include "hal.h"

namespace akt {
  class LIS3Base {
  protected:
    SPIDriver &spi;
    SPIConfig config;

    enum {
      READ     = 0x80,
      WRITE    = 0x00,
      SINGLE   = 0x00,
      MULTIPLE = 0x40
    };

  public:
    LIS3Base(SPIDriver &driver, ioportid_t port, uint16_t pad) :
      spi(driver)
    {
      config.ssport = port;
      config.sspad = pad;

      config.cr1 = ( SPI_CR1_CPOL | // clock idles high
                     SPI_CR1_CPHA | // data on second clock transition
                     0x02         | // SPI Master
                     0x07 << 3);    // slowest clock speed
    }

    void start() { spiStart(&spi, &config); }
    void stop() { spiStop(&spi); }
    void acquire() { spiAcquireBus(&spi); }
    void release() { spiReleaseBus(&spi); }

    SPIDriver *get_driver() const { return &spi; }

    // register oriented read
    template<class T> void read(uint8_t reg, T &buf) {
      reg |= READ | MULTIPLE;
      spiSelect(&spi);
      spiSend(&spi, sizeof(reg), &reg);
      spiReceive(&spi, sizeof(buf), &buf);
      spiUnselect(&spi);
    }

    void read(uint8_t reg, void *buf, size_t len) {
      reg |= READ | MULTIPLE;
      spiSelect(&spi);
      spiSend(&spi, sizeof(reg), &reg);
      spiReceive(&spi, len, buf);
      spiUnselect(&spi);
    }

    // register oriented read
    template<class T> void write(uint8_t reg, T &buf) {
      reg |= WRITE | MULTIPLE;
      spiSelect(&spi);
      spiSend(&spi, sizeof(reg), &reg);
      spiSend(&spi, sizeof(buf), &buf);
      spiUnselect(&spi);
    }

    void write(uint8_t reg, const void *buf, size_t len) {
      reg |= WRITE | MULTIPLE;
      spiSelect(&spi);
      spiSend(&spi, sizeof(reg), &reg);
      spiSend(&spi, len, buf);
      spiUnselect(&spi);
    }
  };

  namespace lis331 {
    enum register_t {
      WHO_AM_I        = 0x0f,
      CTRL_REG1       = 0x20,
      CTRL_REG2       = 0x21,
      CTRL_REG3       = 0x22,
      CTRL_REG4       = 0x23,
      CTRL_REG5       = 0x24,
      HP_FILTER_RESET = 0x25,
      REFERENCE       = 0x26,
      STATUS_REG      = 0x27,
      OUT_X_L         = 0x28,
      OUT_X_H         = 0x29,
      OUT_Y_L         = 0x2a,
      OUT_Y_H         = 0x2b,
      OUT_Z_L         = 0x2c,
      OUT_Z_H         = 0x2d,
      INT_CFG1        = 0x30,
      INT1_SOURCE     = 0x31,
      INT1_THS        = 0x32,
      INT1_DURATION   = 0x33,
      INT2_CFG        = 0x34,
      INT2_SOURCE     = 0x35,
      INT2_THS        = 0x36,
      INT2_DURATION   = 0x37
    };

    enum ctrl_reg1_t {
      X_EN          = 0x01,
      Y_EN          = 0x02,
      Z_EN          = 0x04,
      POWER_DOWN    = 0x00,
      POWER_NORMAL  = 0x20,
      POWER_LOW_0_5 = 0x40,
      POWER_LOW_1   = 0x60,
      POWER_LOW_2   = 0x80,
      POWER_LOW_5   = 0xa0,
      POWER_LOW_10  = 0xc0,
      ODR_50        = 0x00,
      ODR_100       = 0x01,
      ODR_400       = 0x10,
      ODR_1000      = 0x11
    };

    enum ctrl_reg2_t {
      BOOT          = 0x80,
      HPM1          = 0x40,
      HPM2          = 0x20,
      FDS           = 0x10,
      HPEN2         = 0x08,
      HPEN1         = 0x04,
      HPCF1         = 0x02,
      HPCF0         = 0x01
    };

    enum ctrl_reg3_t {
      IHL           = 0x80,
      PP_OD         = 0x40,
      LIR2          = 0x20,
      I2_CFG1       = 0x10,
      I2_CFG0       = 0x08,
      LIR1          = 0x04,
      I1_CFG1       = 0x02,
      I1_CFG0       = 0x01,

      PUSH_PULL     = 0x00,
      OPEN_DRAIN    = 0x40,

      I1_SRC        = 0x00,
      I1_OR_I2_SRC  = 0x01,
      I1_DATA_READY = 0x02,
      I1_BOOT       = 0x03,

      I2_SRC        = 0x00,
      I2_OR_I1_SRC  = 0x08,
      I2_DATA_READY = 0x10,
      I2_BOOT       = 0x18
    };

    enum ctrl_reg4_t {
      BDU           = 0x80,
      BLE           = 0x40,
      FS1           = 0x20,
      FS0           = 0x10,
      STSIGN        = 0x08,
      ST            = 0x02,
      SIM           = 0x01,

      SPI_3WIRE     = 0x01,
      SPI_4WIRE     = 0x00
    };

    enum ctrl_reg5_t {
      TURN_ON_0     = 0x01,
      TURN_ON_1     = 0x02
    };

    enum int_cfg_t {
      AOI       = 0x80,
      SIX_D     = 0x40,
      ZHIE      = 0x20,
      ZLIE      = 0x10,
      YHIE      = 0x08,
      YLIE      = 0x04,
      XHIE      = 0x02,
      XLIE      = 0x01
    };

    enum int_src_t {
      IA        = 0x40,
      ZH        = 0x20,
      ZL        = 0x10,
      YH        = 0x08,
      YL        = 0x04,
      XH        = 0x02,
      XL        = 0x01
    };

    enum status_reg_t {
      XYZOR  = 0x80,
      ZOR    = 0x40,
      YOR    = 0x20,
      XOR    = 0x10,
      ZYXDA  = 0x08,
      ZDA    = 0x04,
      YDA    = 0x02,
      XDA    = 0x01
    };
  };

  class LIS331 : public LIS3Base {
  public:
    LIS331(SPIDriver &driver, ioportid_t port, uint16_t pad) :
      LIS3Base(driver, port, pad)
    {
    }
  };
};
