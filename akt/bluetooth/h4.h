// -*- Mode:C++ -*-

#pragma once

#include "akt/assert.h"
#include "akt/uart.h"
#include "akt/bluetooth/packet.h"

#include "ch.h"

namespace akt {
  namespace bluetooth {

    class PacketDelegate {
    public :
      virtual void recv_hci(Packet &p) = 0;
      virtual void sent_hci(Packet &p) = 0;
    };

    class H4 : protected PacketDelegate {
      enum {
        RX_PACKET_EVENT = 0x01,
        RX_STOP_EVENT   = 0x02
      };

      friend class Script;
      friend class H4UART;

      class H4UART : public UART {
        H4 &h4;
        virtual void txend2();
        virtual void rxend();

      public:
        H4UART(UARTDriver &u, H4 *h4, uint32_t baud = 115200, uartflags_t cr3_flags = 0);
      } uart;

      Ring<Packet> send_queue, recv_queue;
      PacketDelegate *delegate;
      Packet *tx, *rx;
      void (*rx_state)(H4 *self);
      SizedPacket<1> indicator;
      WORKING_AREA(rx_thread, 1024);
      EventSource packets_received_event;

      msg_t rx_thread_main();
      void begin_new_packet();

      void rx_packet_indicator_state();
      void rx_event_header_state();
      void rx_acl_header_state();
      void rx_queue_received_packet_state();

    protected:
      virtual void recv_hci(Packet &p) override;
      virtual void sent_hci(Packet &p) override;

    public:
      PacketPool<259, 4> command_packets;
      PacketPool<1000, 4> acl_packets;
      
      H4(UARTDriver &u, uint32_t baud, uartflags_t cr3_flags);

      void start();
      void resume();
      void stop();
      void suspend();

      virtual void reset();
      virtual void send(Packet &p);
      virtual void set_baud(uint32_t baud);

      void set_delegate(PacketDelegate &d) {
        delegate = &d;
      }

      Packet *alloc_command_packet() {
        return command_packets.allocate();
      }

      Packet *alloc_acl_packet() {
        return acl_packets.allocate();
      }
    };
  };
};
