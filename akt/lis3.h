// -*- Mode:C++ -*-

#pragma once

#include "ch.h"
#include "hal.h"

namespace akt {
  enum lis3dh_register {
    INFO1 = 0x0d,
    INFO2 = 0x0e,
    WHO_AM_I = 0x0f,
    CTRL_REG3 = 0x23,
    CTRL_REG4 = 0x20,
    CTRL_REG5 = 0x24,
    CTRL_REG6 = 0x25,
    STATUS = 0x27,
    OUT_T = 0x0c,
    OFF_X = 0x10,
    OFF_Y = 0x11,
    OFF_Z = 0x12,
    CS_X = 0x13,
    CS_Y = 0x14,
    CS_Z = 0x15,
    LC_L = 0x16,
    LC_H = 0x17,
    STAT = 0x18,
    VFC_1 = 0x1b,
    VFC_2 = 0x1c,
    VFC_3 = 0x1d,
    VFC_4 = 0x1e,
    THRS3 = 0x1f,
    OUT_X_L = 0x28,
    OUT_X_H = 0x29,
    OUT_Y_L = 0x2a,
    OUT_Y_H = 0x2b,
    OUT_Z_L = 0x2c,
    OUT_Z_H = 0x2d,
    FIFO_CTRL = 0x2e,
    FIFO_SRC = 0x2f,
    CTRL_REG1 = 0x21,
    ST1_X = 0x40,
    TIM4_1 = 0x50,
    TIM3_1 = 0x51,
    TIM2_1 = 0x52,
    TIM1_1 = 0x54,
    THRS2_1 = 0x56,
    THRS1_1 = 0x57,
    MASK1_B = 0x59,
    MASK1_A = 0x5a,
    SETT1 = 0x5b,
    PR1 = 0x5c,
    TC1 = 0x5d,
    OUTS1 = 0x5f,
    PEAK1 = 0x19,
    CTRL_REG2 = 0x22,
    ST2_x = 0x60,
    TIM4_2 = 0x70,
    TIM3_2 = 0x71,
    TIM2_2 = 0x72,
    TIM1_2 = 0x74,
    THRS2_2 = 0x76,
    THRS1_2 = 0x77,
    MASK2_B = 0x79,
    MASK2_A = 0x7a,
    SETT2 = 0x7b,
    PR2 = 0x7c,
    TC2 = 0x7d,
    OUTS2 = 0x7f,
    PEAK2 = 0x1a,
    DES2 = 0x78,

    LIS3_READ_BIT = 0x80
  };

  struct xyz_t {
    int16_t x, y, z;
  };

#define UNUSED(begin,end) uint8_t unused_ ## begin ## _ ## end[1+(end)-(begin)]

  struct lis3dsh_registers {
    uint8_t address;
    UNUSED(0x00, 0x0b);
    uint8_t out_t;
    uint8_t info1;
    uint8_t info2;
    uint8_t who_am_i;

    // 0x10
    uint8_t off_x;
    uint8_t off_y;
    uint8_t off_z;
    uint8_t cs_x;
    uint8_t cs_y;
    uint8_t cs_z;
    uint8_t lc_l;
    uint8_t lc_h;
    uint8_t stat;
    uint8_t peak1;
    uint8_t peak2;
    uint8_t vfc_1;
    uint8_t vfc_2;
    uint8_t vfc_3;
    uint8_t vfc_4;
    uint8_t thrs3;

    // 0x20
    uint8_t ctrl_reg4;
    uint8_t ctrl_reg1;
    uint8_t ctrl_reg2;
    uint8_t ctrl_reg3;
    uint8_t ctrl_reg5;
    uint8_t ctrl_reg6;
    UNUSED(0x26,0x26);
    uint8_t status;
    uint8_t out_x_l;
    uint8_t out_x_h;
    uint8_t out_y_l;
    uint8_t out_y_h;
    uint8_t out_z_l;
    uint8_t out_z_h;
    uint8_t fifo_ctrl;
    uint8_t fifo_src;
    
    // 0x30
    UNUSED(0x30, 0x3f);

    // 0x40
    uint8_t st1[16];

    // 0x50
    uint8_t tim4_1;
    uint8_t tim3_1;
    uint16_t tim2_1;
    uint16_t tim1_1;
    uint8_t thrs2_1;
    uint8_t thrs1_1;
    uint8_t des1; // not in datasheet?
    uint8_t mask1_b;
    uint8_t mask1_a;
    uint8_t sett1;
    uint8_t pr1;
    uint16_t tc1;
    uint8_t outs1;

    // 0x60
    uint8_t st2[16];

    // 0x70
    uint8_t tim4_2;
    uint8_t tim3_2;
    uint16_t tim2_2;
    uint16_t tim1_2;
    uint8_t thrs2_2;
    uint8_t thrs1_2;
    uint8_t des2;
    uint8_t mask2_b;
    uint8_t mask2_a;
    uint8_t sett2;
    uint8_t pr2;
    uint16_t tc2;
    uint8_t outs2;
  } __attribute__ ((packed));

  class LIS3 {
    
  protected:
    SPIDriver &spi;
    SPIConfig config;

  public:
    LIS3(SPIDriver &driver, ioportid_t port, uint16_t pad) :
      spi(driver)
    {
      config.ssport = port;
      config.sspad = pad;

      config.cr1 = ( SPI_CR1_CPOL | // clock idles high
                     SPI_CR1_CPHA | // data on second clock transition
                     0);            // fastest clock speed
    }

    void start() {spiStart(&spi, &config);}
    void stop() {spiStop(&spi);}

    // register oriented read
    template<class T> void read(uint8_t reg, T &buf) {
      spiSelect(&spi);
      spiSend(&spi, sizeof(reg), &reg);
      spiReceive(&spi, sizeof(buf), &buf);
      spiUnselect(&spi);
    }

    // register oriented read
    template<class T> void write(uint8_t reg, T &buf) {
      spiSelect(&spi);
      spiSend(&spi, sizeof(reg), &reg);
      spiSend(&spi, sizeof(buf), &buf);
      spiUnselect(&spi);
    }

    void read_xyz(struct xyz_t &xyz) {
      uint8_t addr = OUT_X_L | LIS3_READ_BIT;

      spiSelect(&spi);
      spiSend(&spi, sizeof(addr), &addr);
      spiExchange(&spi, sizeof(xyz), &xyz, &xyz);
      spiUnselect(&spi);
    }

    void read_all(lis3dsh_registers &registers) {
      registers.address = 0x00 | LIS3_READ_BIT;
      spiSelect(&spi);
      spiExchange(&spi, sizeof(registers), &registers, &registers);
      spiUnselect(&spi);
    }

    uint8_t read8(lis3dh_register addr) {
      uint8_t txbuf[2] = {(uint8_t) (addr | LIS3_READ_BIT), 0xff}, rxbuf[2];

      spiSelect(&spi);
      spiExchange(&spi, sizeof(txbuf), txbuf, rxbuf);
      spiUnselect(&spi);

      return rxbuf[1];
    }

    int16_t read16(lis3dh_register addr) {
      uint8_t txbuf[3] = {(uint8_t) (addr | LIS3_READ_BIT), 0xff, 0xff}, rxbuf[3];

      spiSelect(&spi);
      spiExchange(&spi, sizeof(txbuf), txbuf, rxbuf);
      spiUnselect(&spi);

      return (int16_t) (rxbuf[2] + (rxbuf[1] << 8));
    }

    void write8(lis3dh_register addr, uint8_t data) {
      uint8_t txbuf[2] = {(uint8_t) addr, data};

      spiSelect(&spi);
      spiSend(&spi, sizeof(txbuf), txbuf);
      spiUnselect(&spi);
    }
  };
};
