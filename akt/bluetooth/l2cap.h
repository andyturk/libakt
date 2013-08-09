// -*- Mode:C++ -*-

#pragma once

#include "akt/bluetooth/hci.h"
#include "akt/ring.h"

#include <stdint.h>

namespace akt {
  namespace bluetooth {
  class Channel : public Ring<Channel> {

  public:
    uint16_t channel_id, mtu;

    virtual void recv_acl_data(Packet &p);
  };
  };
};
