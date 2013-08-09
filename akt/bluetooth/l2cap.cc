#include "akt/assert.h"
#include "akt/bluetooth/hci.h"
#include "akt/bluetooth/l2cap.h"

namespace akt {
  namespace bluetooth {
  void Channel::recv_acl_data(Packet &p) {
    debug("received data for channel 0x%04x\n", channel_id);
    p.deallocate();
  }
};
