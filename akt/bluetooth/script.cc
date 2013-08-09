#include "akt/bluetooth/script.h"
#include "akt/assert.h"

namespace akt {
  namespace bluetooth {
    Script::Script(const uint8_t *bytes, uint16_t length) :
      host_controller(0),
      bytes((uint8_t *) bytes, length),
      last_opcode(0),
      baud_rate(0),
      max_baud_rate(0),
      state(&next_command)
    {
      chMtxInit(&script_complete_lock);
      chCondInit(&script_complete_condition);
    }

    bool Script::recv_command_complete_event(uint16_t opcode, Packet &p) {
      if (opcode == last_opcode) {
        uint8_t status;

        p >> status;
        //debug("opcode 0x%04x status=0x%02x\n", opcode, status);
        last_opcode = 0;
        p.deallocate();
        next();
        return true;
      } else {
        return false;
      }
    }

    bool Script::recv_command_status_event(uint16_t opcode, Packet &p) {
      if (opcode == last_opcode) {
        p.deallocate();
        return true;
      } else {
        return false;
      }
    }

    void Script::restart() {
      bytes.rewind();
      last_opcode = 0;
    }

    void Script::complete() {
      chMtxLock(&script_complete_lock);
      chCondSignal(&script_complete_condition);
      chMtxUnlock();
    }

    Packet *Script::next_command() {
      if (bytes.remaining() > 0) {
        assert(bytes.remaining() >= 4);
        assert(last_opcode == 0);
        assert(host_controller != 0);

        Packet *p = host_controller->h4.alloc_command_packet();
        assert(p != 0);

        uint8_t indicator, parameter_length, *start;
    
        start = (uint8_t *) bytes;
        bytes >> indicator >> last_opcode >> parameter_length;

        assert(bytes.remaining() >= parameter_length);

        p->reset();
        p->write(start, 4 + parameter_length);
        p->flip();

        host_controller->h4.send(*p);

        if (last_opcode == OPCODE_PAN13XX_CHANGE_BAUD_RATE) {
          assert(parameter_length == sizeof(baud_rate));
          bytes >> baud_rate;

          if (max_baud_rate && (baud_rate > max_baud_rate)) {
            baud_rate = max_baud_rate;
          }
        } else {
          bytes += parameter_length;
        }

        return p;
      } else {
        complete();
        return 0;
      }
    }

    bool Script::execute(HostController *h) {
      assert(h != 0);

      chMtxLock(&script_complete_lock);
      assert(host_controller == 0);
      host_controller = h;
      next();
      chCondWait(&script_complete_condition);
      chMtxUnlock();

      return true;
    }
  }
};
