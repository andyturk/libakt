#include "akt/bluetooth/h4.h"
#include "akt/assert.h"

#include "ch.h"
#include "hal.h"

namespace akt {
  namespace bluetooth {
    H4::H4UART::H4UART(UARTDriver &u, H4 *h4, uint32_t baud, uartflags_t cr3_flags) :
      UART(u, baud, cr3_flags),
      h4(*h4)
    {
    }

    void H4::H4UART::txend2() {
      if (h4.tx != 0) {                    // must have a packet
        // what about taking tx off the send queue?
        (h4.delegate ? h4.delegate : &h4)->sent_hci(*h4.tx);
        h4.tx = 0;                         // done transmitting that one

        if (!h4.send_queue.empty()) { // more to send?
          h4.tx = h4.send_queue.rbegin();
          sendI((uint8_t *) *h4.tx, h4.tx->remaining());
        }
      } else {
        // no tx packet means spurious tx interrupt
      }
    }

    void H4::H4UART::rxend() {
      h4.rx_state(&h4);
    }

    H4::H4(UARTDriver &u, uint32_t baud, uint32_t cr3_flags) :
      uart(u, this, baud, cr3_flags)
    {
      chEvtInit(&packets_received_event);
    }

    void H4::start() {
      chThdCreateStatic(rx_thread,
                        sizeof(rx_thread),
                        NORMALPRIO,
                        (msg_t (*)(void *)) &rx_thread_main,
                        this);
    }

    void H4::suspend() {
      uart.stop();
    }

    void H4::resume() {
      uart.start();
    }

    void H4::stop() {
      chEvtSignal((Thread *) &rx_thread, RX_STOP_EVENT);
    }

    msg_t H4::rx_thread_main() {
      const eventmask_t mask = (RX_PACKET_EVENT | RX_STOP_EVENT);

      chRegSetThreadName("H4::RX");
      for (;;) {
        eventmask_t events = chEvtWaitAnyTimeout(mask, MS2ST(250));

        if (events & RX_STOP_EVENT) {
          break;
        }

        if (events & RX_PACKET_EVENT) {
          palSetPad(GPIOC, 3);
          chThdSleepMilliseconds(20);
          palClearPad(GPIOC, 3);

          assert(!recv_queue.empty());
          Packet *p = recv_queue.rbegin();

          (delegate ? delegate : this)->recv_hci(*p);
          continue;
        }
      }

      return 0;
    }

    void H4::reset() {
      uart.stop();
      tx = rx = 0;
      uart.set_baud(115200);

      command_packets.reset();
      acl_packets.reset();
      send_queue.join(&send_queue);
      recv_queue.join(&recv_queue);

      uart.start();

      chSysLock();
      begin_new_packet();
      chSysUnlock();
    }

    void H4::send(Packet &p) {
      chSysLock();
      p.join(&send_queue);
      if (tx == 0) {
        tx = &p;
        uart.sendI((uint8_t *) *tx, tx->remaining());
      }
      chSysUnlock();
    }

    void H4::set_baud(uint32_t baud) {
      chSysLock();

      bool uart_was_active = (uart.get_state() == UART_READY);
      uart.stop();
      uart.set_baud(baud);
      if (uart_was_active) uart.start();

      chSysUnlock();
    }

    void H4::rx_packet_indicator_state() {
      indicator.flip();

      switch (*indicator) {
      case HCI::EVENT_PACKET :
        rx = command_packets.allocate();
        assert(rx != 0);

        rx->limit(1+1+1); // indicator, event code, param length
        *rx << *indicator;
        rx_state = &rx_event_header_state;
        break;

      case HCI::ACL_PACKET :
        rx = acl_packets.allocate();
        assert(rx != 0);

        rx->limit(1+4);
        *rx << *indicator;
        rx_state = &rx_acl_header_state;
        break;

      case HCI::GO_TO_SLEEP_IND :
        //debug("baseband wants to sleep!\n");
        rx->reset();
        break;

      case HCI::COMMAND_PACKET :
      case HCI::SYNCHRONOUS_DATA_PACKET :
      default :
        assert(false);
      }

      chSysLockFromIsr();
      uart.recvI((uint8_t *) *rx, rx->remaining());
      chSysUnlockFromIsr();
    }

    void H4::rx_event_header_state() {
      rx->skip(2); // just read in event code and param length
      uint8_t param_length = (*rx)[-1];

      if (param_length > 0) {
        chSysLockFromIsr();
        rx->limit(1+1+1 + param_length);
        rx_state = &rx_queue_received_packet_state;
        uart.recvI((uint8_t *) *rx, rx->remaining());
        chSysUnlockFromIsr();
      } else {
        // there are no params, so just chain to the next state
        rx_queue_received_packet_state();
      }
    }

    void H4::rx_acl_header_state() {
      *rx += 4; // just read in acl header
      uint16_t length = ((*rx)[-1] << 8) + (*rx)[-2];
      chSysLockFromIsr();
      rx->limit(1+4+length);
      rx_state = &rx_queue_received_packet_state;
      uart.recvI((uint8_t *) *rx, rx->remaining());
      chSysUnlockFromIsr();
    }

    void H4::rx_queue_received_packet_state() {
      rx->skip(rx->remaining());
      rx->flip();

      chSysLockFromIsr();
      rx->join(&recv_queue);
      chEvtSignalI((Thread *) &rx_thread, RX_PACKET_EVENT);
      begin_new_packet();
      chSysUnlockFromIsr();
    }

    void H4::begin_new_packet() {
      indicator.reset();
      rx = &indicator;
      rx_state = &rx_packet_indicator_state;

      uart.recvI((uint8_t *) indicator, indicator.remaining());
    }

    void H4::sent_hci(Packet &p) {
      p.deallocate();
    }

    void H4::recv_hci(Packet &p) {
      p.deallocate();
    }
  }
};
