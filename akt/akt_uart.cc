#include "akt/uart.h"
#include "akt/assert.h"

namespace akt {
  void bottleneck() {
  }

  void UART::set_baud(uint32_t baud) {
    assert(uart.rxstate == UART_RX_IDLE);
    assert(uart.txstate == UART_TX_IDLE);
    speed = baud;
  }

  void UART::txend1_cb(UARTDriver *d) {
    bottleneck();
    UART *u = (UART *) d->config;
    u->txend1();
  }

  void UART::txend2_cb(UARTDriver *d) {
    bottleneck();
    UART *u = (UART *) d->config;
    u->txend2();
  }

  void UART::rxend_cb(UARTDriver *d) {
    bottleneck();
    UART *u = (UART *) d->config;
    u->rxend();
  }

  void UART::rxchar_cb(UARTDriver *d, uint16_t c) {
    bottleneck();
    UART *u = (UART *) d->config;
    u->rxchar(c);
  }

  void UART::rxerr_cb(UARTDriver *d, uartflags_t e) {
    bottleneck();
    UART *u = (UART *) d->config;
    u->rxerr(e);
  }

  UART::UART(UARTDriver &u, uint32_t baud, uint32_t cr3_flags) :
    UARTConfig(),
    uart(u)
  {
    UARTConfig::txend1_cb = &UART::txend1_cb;
    UARTConfig::txend2_cb = &UART::txend2_cb;
    UARTConfig::rxend_cb  = &UART::rxend_cb;
    UARTConfig::rxchar_cb = &UART::rxchar_cb;
    UARTConfig::rxerr_cb  = &UART::rxerr_cb;

    speed = baud;
    cr1 = 0;
    cr2 = 0;
    cr3 = cr3_flags;
  }

  void UART::start() {
    uartStart(&uart, this);
  }

  void UART::stop() {
    if (uart.state == UART_READY) {
      uartStopSend(&uart);
      uartStopReceive(&uart);
      uartStop(&uart);
    }
  }
};
