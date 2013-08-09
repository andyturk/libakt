// -*- Mode:C++ -*-

#pragma once

#include "ch.h"
#include "hal.h"

namespace akt {
  class UART : protected UARTConfig {
  protected:
    UARTDriver &uart;

    static void txend1_cb(UARTDriver *u);
    static void txend2_cb(UARTDriver *u);
    static void rxend_cb(UARTDriver *u);
    static void rxchar_cb(UARTDriver *u, uint16_t c);
    static void rxerr_cb(UARTDriver *u, uartflags_t e);

    virtual void txend1() {}
    virtual void txend2() {}
    virtual void rxend() {}
    virtual void rxchar(uint16_t c) {}
    virtual void rxerr(uartflags_t err) {}
    
  public:
    UART(UARTDriver &u, uint32_t baud = 115200, uartflags_t cr3_flags = 0);

    uartstate_t get_state() const       {return uart.state;}
    void set_baud(uint32_t baud);

    void start();
    void stop();

    void send (void *bytes, size_t len) {uartStartSend    (&uart, len, (uint8_t *) bytes);}
    void sendI(void *bytes, size_t len) {uartStartSendI   (&uart, len, (uint8_t *) bytes);}
    void recv (void *bytes, size_t len) {uartStartReceive (&uart, len, (uint8_t *) bytes);}
    void recvI(void *bytes, size_t len) {uartStartReceiveI(&uart, len, (uint8_t *) bytes);}
  };
};
