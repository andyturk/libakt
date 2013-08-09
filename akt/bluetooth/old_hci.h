// -*- Mode:C++ -*-

#pragma once

#include "akt/pool.h"
#include "akt/ring.h"
#include "akt/bluetooth/uuid.h"
#include "akt/bluetooth/bluetooth_constants.h"
#include "akt/bluetooth/bd_addr.h"
#include "akt/bluetooth/packet.h"
#include "akt/bluetooth/h4.h"
#include "akt/bluetooth/script.h"

#include <stdint.h>
#include <stdarg.h>
#include <deque>

namespace akt::bluetooth {
  extern const char hex_digits[16];

  namespace HCI {
    class Connection : public Ring<Connection> {
      uint16_t handle;
    };
  };

  using namespace HCI;

  class HostController : public H4Controller {
  protected:
    PoolBase<Packet> *command_packets;
    PoolBase<Packet> *acl_packets;
    PoolBase<Connection> *connections;

    Ring<Connection> remotes;
    uint8_t command_packet_budget;

  public:
    BD_ADDR bd_addr;

  HostController(PoolBase<Packet> *cmd, PoolBase<Packet> *acl, PoolBase<Connection> *conn) :
    command_packets(cmd),
      acl_packets(acl),
      connections(conn)
      {}

    // H4Controller methods
    virtual void sent(Packet *p) {}
    virtual void received(Packet *p) {}

    virtual void initialize() {}
    virtual void periodic(uint32_t msec) {}
    virtual void send(Packet *p);
    virtual void receive(Packet *p) {
      // p->join(&incoming_packets);
    }

    friend class H4Tranceiver;
    friend class ATT_Channel;
  };

  class BBand : public HostController {
    class HCIScript : public CannedScript {
    protected:
      BBand &bb;

    public:
      HCIScript(BBand &b, uint8_t *bytes, uint16_t length);
      virtual bool command_complete(uint16_t opcode, Packet *p);
    };

    class WarmBootScript : public HCIScript {
    protected:
      virtual void send(Packet *p);

    public:
      WarmBootScript(BBand &b);
      virtual bool command_complete(uint16_t opcode, Packet *p);
    };

    UART &uart;
    IOPin &shutdown;
    Pool<HCI::Connection, 3> hci_connection_pool;
    HCIScript *script;

    void (*event_handler)(BBand *, uint8_t event, Packet *);
    void (*command_complete_handler)(BBand *, uint16_t opcode, Packet *);

    // initialization states
    void execute_commands(HCIScript &s);

    // void cold_boot(uint16_t opcode, Packet *p);
    // void upload_patch(uint16_t opcode, Packet *p);
    // void warm_boot(uint16_t opcode, Packet *p);
    void normal_operation(uint16_t opcode, Packet *p);
    void command_complete(uint16_t opcode, Packet *p);
  
    void standard_packet_handler(Packet *p);
    void default_event_handler(uint8_t event, Packet *p);
    void acl_packet_handler(uint16_t handle, uint8_t pb, uint8_t bc, Packet *p);
    void l2cap_packet_handler(Packet *p);
    void le_event_handler(uint8_t subevent, Packet *p);
    void att_packet_handler(Packet *p);

    struct {
      uint16_t expected_opcode;
      size_t offset;
      size_t length;
      const uint8_t *data;
    } patch_state;

  public:
    BBand(UART &u, IOPin &s);
    void initialize();
    void process_incoming_packets();
  };
};
