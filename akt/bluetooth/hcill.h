// -*- Mode:C++ -*-

#pragma once

#include "akt/bluetooth/packet.h"

namespace akt {
  namespace bluetooth {
    class eHCILL {
      SizedPacket<1> hcill_indicator;
 
    public:
      void bb_sleep_ind() {

      }
      void bb_wakeup_ind();
      void bb_wakeup_ack();
    };
  };
};
