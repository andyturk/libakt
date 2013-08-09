// -*- Mode:C++ -*-

#pragma once

#include "akt/bluetooth/packet.h"
#include "akt/bluetooth/h4.h"
#include "akt/bluetooth/script.h"
#include "akt/bluetooth/l2cap.h"
#include "akt/bluetooth/script.h"

namespace akt {
  namespace bluetooth {

    class Script;

    class Connection : public Ring<Connection> {
      uint16_t handle;
    };

    class HostController : protected PacketDelegate {
      friend class Script;

      H4 &h4;
      Pool<Connection,4> connections;
      uint8_t command_packet_budget;
      Script *script;

    protected:
      virtual void recv_hci(Packet &p) override;
      virtual void sent_hci(Packet &p) override;
      Ring<Channel> channels;

      void script_complete();
      void recv_event(uint8_t event, Packet &p);
      void recv_le_event(uint8_t subevent, Packet &p);
      void recv_command_complete_event(uint16_t opcode, Packet &p);

    public:
      BD_ADDR bd_addr;

      HostController(H4 &h4);
      virtual void reset();
      void execute(Script &s);
      Channel *find_channel(uint16_t id);
      void send(Packet &p) {h4.send(p);}
    };
  };
};
