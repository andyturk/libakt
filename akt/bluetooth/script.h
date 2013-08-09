// -*- Mode:C++ -*-

#pragma once

#include "akt/bluetooth/hci.h"
#include "akt/assert.h"

#include "ch.h"

#include <stdint.h>

namespace akt {
  namespace bluetooth {
    class HostController;

    class Script {
      HostController *host_controller;
      Packet bytes;
      uint16_t last_opcode;
      uint32_t baud_rate, max_baud_rate;
      void (*state)(Script *);
      Packet *next_command();

      Mutex script_complete_lock;
      CondVar script_complete_condition;

      virtual void complete();

    public:
      Script(const uint8_t *bytes, uint16_t length);

      bool is_complete() const {
        return bytes.remaining() == 0 && last_opcode == 0;
      }

      void next() {
        (*state)(this);
      }

      bool is_pending() const {
        return last_opcode != 0;
      }

      void set_max_baud_rate(uint32_t baud) {
        max_baud_rate = baud;
      }

      virtual bool execute(HostController *h);
      virtual bool recv_command_complete_event(uint16_t opcode, Packet &p);
      virtual bool recv_command_status_event(uint16_t opcode, Packet &p);
      virtual void restart();
    };
  };
};
