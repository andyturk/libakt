// -*- Mode:C++ -*-

#include "ch.h"
#include "hal.h"
#include "board.h"

#include "akt/bluetooth/hci.h"
#include "debug.h"

namespace akt {
  namespace bluetooth {
    HostController::HostController(H4 &h4) :
      h4(h4),
      script(0)
    {
      h4.set_delegate(*this);
    }

    void HostController::reset() {
      h4.reset();
    }

    void HostController::recv_hci(Packet &p) {
      uint8_t packet_indicator;

      p >> packet_indicator;

      switch (packet_indicator) {
      case EVENT_PACKET : {
        uint8_t event, parameter_length __attribute__ ((unused));

        p >> event >> parameter_length;

        if (event == EVENT_COMMAND_COMPLETE) {
          uint16_t opcode;

          p >> command_packet_budget >> opcode;
          recv_command_complete_event(opcode, p);
        } else {
          recv_event(event, p);
        }
        break;
      }
    
      case ACL_PACKET : {
        p.skip(3*sizeof(uint16_t)); // acl handle, acl length, l2cap length
        uint16_t cid;

        p >> cid;
        Channel *channel = find_channel(cid);

        if (channel) {
          channel->recv_acl_data(p);
        } else {
          debug("ACL packet for unknown channel: 0x%04x\n", cid);
          p.deallocate();
        }
        break;
      }

      case COMMAND_PACKET :
      case SYNCHRONOUS_DATA_PACKET :
      default :
        debug("discarding unknown packet of type %d\n", packet_indicator);
        p.deallocate();
      }
    }

    void HostController::sent_hci(Packet &p) {
      p.deallocate();
    }

    void HostController::recv_event(uint8_t event, Packet &p) {
      switch (event) {
      case EVENT_DISCONNECTION_COMPLETE : {
        uint16_t handle;
        uint8_t status, reason;

        p >> status >> handle >> reason;
        handle &= 0x0fff;

        debug("disconnected 0x%04x (with status: 0x%02x) because 0x%02x\n", handle, status, reason);
        debug("re-enabling LE advertising\n");
        p.hci(OPCODE_LE_SET_ADVERTISE_ENABLE) << (uint8_t) 0x01;
        h4.send(p);
        return; // avoid deallocation of p
      }
    
      case EVENT_LE_META_EVENT : {
        uint8_t subevent;
        p >> subevent;
        recv_le_event(subevent, p);
        break;
      }

      case EVENT_NUMBER_OF_COMPLETED_PACKETS : {
        uint8_t number_of_handles;

        p >> number_of_handles;
        // uint16_t *connection_handle = (uint16_t *) (uint8_t *) *p;
        p.skip(number_of_handles*sizeof(uint16_t));
        // uint8_t *num_of_completed_packets = (uint8_t *) *p;

        for (uint8_t i=0; i < number_of_handles; ++i) {
          /*
            debug("connection 0x%04x completed %d packets\n",
            connection_handle[i],
            num_of_completed_packets[i]);
          */
        }
        break;
      }

      default :
        debug("discarding event 0x%02x\n", event);
      }

      p.deallocate();
    }

    void HostController::recv_le_event(uint8_t subevent, Packet &p) {
      switch(subevent) {
      case LE_EVENT_CONNECTION_COMPLETE : {
        BD_ADDR peer_address;
        uint8_t status, role, peer_address_type, master_clock_accuracy;
        uint16_t connection_handle, conn_interval, conn_latency, supervision_timeout;

        p >> status >> connection_handle >> role;
        p >> peer_address_type;
        p.write(peer_address.data, sizeof(peer_address.data));
        p >> conn_interval >> conn_latency;
        p >> supervision_timeout >> master_clock_accuracy;

        const char *type;

        switch (peer_address_type) {
        case 0 :
          type = "public";
          break;

        case 1 :
          type = "random";
          break;

        default :
          type = "unknown";
          break;
        }

        const char *role_name;
    
        switch (role) {
        case 0 :
          role_name = "master";
          break;

        case 1 :
          role_name = "slave";
          break;

        default :
          role_name = "unknown";
          break;
        }

        char buf[BD_ADDR::PRETTY_SIZE];
        const char *addr = peer_address.pretty_print(buf);
        debug("connection to %s completed with status %d\n", addr, status);
        debug("  handle = 0x%04x, addr_type = %s\n", connection_handle, type);
        debug("  role = %s, interval = %d, latency = %d, timeout = %d\n",
              role_name, conn_interval, conn_latency, supervision_timeout);
        debug("  clock_accuracy = %d\n", master_clock_accuracy);
        break;
      }
      case LE_EVENT_ADVERTISING_REPORT :
      case LE_EVENT_CONNECTION_UPDATE_COMPLETE :
      case LE_EVENT_READ_REMOTE_USED_FEATURES_COMPLETE :
      case LE_EVENT_LONG_TERM_KEY_REQUEST :
      default :
        debug("ignoring unrecognized LE event: 0x%02x\n", subevent);
        break;
      }
    }

    void HostController::recv_command_complete_event(uint16_t opcode, Packet &p) {
      if (script && script->recv_command_complete_event(opcode, p)) return;

      switch (opcode) {
      case OPCODE_READ_LOCAL_VERSION_INFORMATION : {
        uint8_t hci_version, lmp_version;
        uint16_t hci_revision, manufacturer_name, lmp_subversion;

        p >> hci_version >> hci_revision >> lmp_version >> manufacturer_name >> lmp_subversion;
        assert(hci_version == SPECIFICATION_4_0);
        break;
      }

      case OPCODE_READ_BD_ADDR : {
        p.read(bd_addr.data, sizeof(bd_addr.data));

        debug("bd_addr = ");
        for (int i=5; i >= 0; --i) debug("%02x:", bd_addr.data[i]);
        debug("\n");
        break;
      }

      case OPCODE_READ_BUFFER_SIZE_COMMAND : {
        uint16_t acl_data_length, num_acl_packets, num_synchronous_packets;
        uint8_t synchronous_data_length;

        p >> acl_data_length >> synchronous_data_length;
        p >> num_acl_packets >> num_synchronous_packets;

        debug("acl: %d @ %d, synchronous: %d @ %d\n",
              num_acl_packets, acl_data_length,
              num_synchronous_packets, synchronous_data_length);
        break;
      }

      case OPCODE_READ_PAGE_TIMEOUT : {
        uint16_t timeout;

        p >> timeout;
        uint32_t usec = timeout*625;
        debug("page timeout = %d.%d msec\n", usec/1000, usec%1000);
        break;
      }

      case OPCODE_LE_READ_BUFFER_SIZE : {
        debug("buffer size read\n");
        uint16_t le_data_packet_length;
        uint8_t num_le_packets;

        p >> le_data_packet_length >> num_le_packets;
        debug("le_data_packet_length = %d, num_packets = %d\n", le_data_packet_length, num_le_packets);
        break;
      }

      case OPCODE_LE_READ_SUPPORTED_STATES : {
        debug("supported states read\n");

        uint8_t states[8];
        p.read(states, sizeof(states));
        debug("states = 0x");
        for (int i=0; i < 8; ++i) debug("%02x", states[i]);
        debug("\n");
        break;
      }
      }
      p.deallocate();
    }

    Channel *HostController::find_channel(uint16_t id) {
      for (Channel::Iterator i = channels.begin(); i != channels.end(); ++i) {
        if (i->channel_id == id) return i;
      }

      return 0;
    }

    void HostController::execute(Script &s) {
      palSetPad(GPIOC, GPIOC_LED1);

      assert(script == 0);

      script = &s;
      s.execute(this);
      script = 0;

      palClearPad(GPIOC, GPIOC_LED1);
    }
  };
};
