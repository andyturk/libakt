#include "akt/assert.h"
#include "akt/bluetooth/att.h"

#include <cstring>
#include <algorithm>

namespace akt::bluetooth {
  AttributeBase::AttributeBase(const UUID &t, void *d, uint16_t l) :
    type(t),
    handle(++next_handle),
    _data(d),
    length(l)
  {
    all_handles[handle] = this;
  }

  AttributeBase::AttributeBase(int16_t t, void *d, uint16_t l) :
    type(t),
    handle(++next_handle),
    _data(d),
    length(l)
  {
    all_handles[handle] = this;
  }

  uint16_t AttributeBase::find_by_type_value(uint16_t start, uint16_t type, void *value, uint16_t length) {
    assert(start > 0);
    for (uint16_t i=start; i < next_handle; ++i) {
      AttributeBase *attr = all_handles[i];

      if (attr->type != type) continue;
      if (length != attr->length) continue; // e.g., UUIDs could be either 2 or 16 bytes
      if (memcmp(attr->_data, value, length)) continue;

      return i;
    }

    return 0;
  }

  uint16_t AttributeBase::find_by_type(uint16_t start, const UUID &type) {
    assert(start > 0);
    for (uint16_t i=start; i < next_handle; ++i) {
      if (all_handles[i]->type == type) return i;
    }
    return 0;
  }

  uint16_t AttributeBase::group_end() {
    return handle;
  }

  int AttributeBase::compare(void *data, uint16_t len, uint16_t min_handle, uint16_t max_handle) {
    if (handle < min_handle) return -1;
    if (handle > max_handle) return 1;
    return compare(data, len);
  }

#ifdef DEBUG
  void AttributeBase::dump_attributes() {
    for (int i=1; i < next_handle; ++i) {
      AttributeBase *attr = all_handles[i];

      debug("%04x %s: (%d) ", attr->handle, attr->type.pretty_print(), attr->length);
      dump_hex_bytes((uint8_t *) attr->_data, attr->length);
      debug("\n");
    }
  }
#endif

  ATT_Channel::ATT_Channel(HostController &hc) :
    Channel(L2CAP::ATTRIBUTE_CID, hc),
    att_mtu(23)
  {
  }

  bool ATT_Channel::read_type() {
    switch (req->remaining()) {
    case 2 : {
      uint16_t short_uuid;
      *req >> short_uuid;
      type = short_uuid;
      break;
    }

    case 16 :
      assert(sizeof(type.data) == 16);
      req->read(type.data, sizeof(type.data));
      break;

    default :
      error(ATT::INVALID_PDU);
      return false;
    }

    return true;
  }

  bool ATT_Channel::read_handles() {
    *req >> h1 >> h2;
    if (h1 > h2 || h1 == 0) {
      error(ATT::INVALID_HANDLE);
      return false;
    }

    return true;
  }

  void ATT_Channel::error(uint8_t err) {
    rsp = req;
    rsp->l2cap() << (uint8_t) ATT::OPCODE_ERROR << req_opcode << h1 << err;  
  }

  bool ATT_Channel::is_grouping(const UUID &type) {
    return true;
  }

  void ATT_Channel::find_information() {
    rsp = req; // re-use request packet

    const uint16_t short_info = sizeof(uint16_t) + sizeof(uint16_t);
    const uint16_t long_info = sizeof(uint16_t) + sizeof(UUID);

    uint16_t info_length = short_info;

  restart:
    rsp->l2cap(att_mtu);
    *rsp << rsp_opcode;
    uint8_t *format = (uint8_t *) rsp;
    *rsp << (uint8_t) 0; // format placeholder

    for (uint16_t h=h1; h <= h2 && rsp->remaining() >= info_length; ++h) {
      AttributeBase *attr = AttributeBase::get(h);
      assert(attr != 0);

      if (info_length == short_info) {
        if (!attr->type.is_16bit()) {
          info_length = long_info;
          goto restart;
        }

        *rsp << attr->handle;
        *rsp << (uint16_t) attr->type;
      } else {
        *rsp << attr->handle;
        rsp->write(attr->type.data, sizeof(attr->type.data));
      }
    }

    *format = (info_length == short_info) ? 0x01 : 0x02;
  }

  void ATT_Channel::read_by_group_type() {
    if (!is_grouping(type)) {
      error(ATT::UNSUPPORTED_GROUP_TYPE);
      return;
    }

    rsp = req; // re-use request packet
    rsp->l2cap(att_mtu);

    *rsp << rsp_opcode;
    uint8_t &attribute_data_length = *(uint8_t *) *rsp;
    *rsp << (uint8_t) 0; // placeholder

    uint16_t attr_length = 0;
    uint16_t data_length = 0;

    /*
     * AttributeBase::find_by_type returns 0 when a handle can't be found.
     * This is less than the lowest expected value of h1.
     */
    uint16_t h = AttributeBase::find_by_type(h1, type);

    for (; rsp->remaining() > 2*sizeof(uint16_t) + 1;) {
      if (h < h1 || h > h2) break;
      AttributeBase *attr = AttributeBase::get(h);
      if (attr_length == 0) { // first matching attribute
        attr_length = attr->length;
      } else if(attr_length != attr->length) { // stop if lengths differ
        break;
      }

      *rsp << attr->handle << attr->group_end();
      data_length = std::min(attr_length, rsp->remaining());
      rsp->write((const uint8_t *) attr->_data, data_length);
    }
  
    if (attr_length == 0) {
      error(ATT::ATTRIBUTE_NOT_FOUND);
      return;
    } else {
      attribute_data_length = 2*sizeof(uint16_t) + data_length;
    }
  }

  void ATT_Channel::find_by_type_value() {
    uint16_t length;
    uint8_t *value;

    value = (uint8_t *) *req;
    length = req->remaining(); // is this right?

    // can't re-use the request packet because we need the data at the end
    rsp = controller.acl_packets->allocate();
    assert(rsp != 0);
    rsp->l2cap(req, att_mtu); // re-use existing L2CAP framing from request

    *rsp << rsp_opcode;
    uint16_t found_attribute_handle = 0, group_end_handle;
    uint16_t short_type = (uint16_t) type;

    /*
     * AttributeBase::find_by_type_value returns 0 when a handle can't be found.
     * This is less than the lowest expected value of h1.
     */
    uint16_t h = AttributeBase::find_by_type_value(h1, short_type, value, length);

    for (;;) {
      if (h < h1 || h > h2 || (rsp->remaining() < 2*sizeof(uint16_t))) break;

      found_attribute_handle = h;
      group_end_handle = AttributeBase::get(h)->group_end();
      uint16_t next_h = AttributeBase::find_by_type_value(h+1, short_type, value, length);

      if (found_attribute_handle == group_end_handle) { // not a grouping attribute
        if (next_h < h1 || next_h > h2) group_end_handle = 0xffff;
      }

      *rsp << found_attribute_handle << group_end_handle;
      h = next_h;
    }

    if (found_attribute_handle == 0) {
      error(ATT::ATTRIBUTE_NOT_FOUND);
      return;
    }
  }

  void ATT_Channel::read_by_type() {
    rsp = req; // re-use request packet
    rsp->l2cap(att_mtu);

    *rsp << (uint8_t) rsp_opcode;
    uint8_t &data_length = *(uint8_t *) *rsp;
    *rsp << (uint8_t) 0; // placeholder

    uint16_t attr_length = 0; // actual attribute length
    uint16_t h = h1;

    do {
      h = AttributeBase::find_by_type(h, type);
      if (h < h1 || h > h2) break;

      AttributeBase *attr = AttributeBase::get(h);
      assert(h == attr->handle);

      if (attr_length == 0) { // first matching attribute
        attr_length = attr->length;
        data_length = std::min(attr_length          + sizeof(uint16_t),
                               rsp->remaining() - sizeof(uint16_t));
      } else if(attr_length != attr->length) { // stop if lengths differ
        break;
      }

      *rsp << h;
      rsp->write((const uint8_t *) attr->_data, data_length - sizeof(uint16_t));
      h += 1;
    } while (rsp->remaining() >= data_length);
  
    if (attr_length == 0) {
      error(ATT::ATTRIBUTE_NOT_FOUND);
      return;
    }
  }

  void ATT_Channel::receive(Packet *p) {
    AttributeBase *attr = 0;

    req = p;
    rsp = 0;

    *req >> req_opcode;

    switch (req_opcode) {
    case ATT::OPCODE_ERROR : {
      uint8_t error_code;

      *p >> req_opcode >> h1 >> error_code;
      debug("ATT error for opcode: 0x%02x, handle: 0x%04x, error: 0x%02x\n",
            req_opcode, h1, error_code);
      break;
    }

    case ATT::OPCODE_EXCHANGE_MTU_REQUEST :
      rsp_opcode = ATT::OPCODE_EXCHANGE_MTU_RESPONSE;
      *req >> client_rx_mtu;
      rsp = req;
      rsp->l2cap() << rsp_opcode << att_mtu;
      debug("ATT: client MTU = %d\n", client_rx_mtu);
      break;

    case ATT::OPCODE_FIND_INFORMATION_REQUEST :
      rsp_opcode = ATT::OPCODE_FIND_INFORMATION_RESPONSE;
      if (!read_handles()) break;
      debug("ATT: find information %04x-%04x\n", h1, h2);
      find_information();
      break;

    case ATT::OPCODE_FIND_BY_TYPE_VALUE_REQUEST :
      rsp_opcode = ATT::OPCODE_FIND_BY_TYPE_VALUE_RESPONSE;
      if (!read_handles()) break;
      if (!read_type()) break;
      debug("ATT: find %04x-%04x by type: %s, value: (%d bytes)\n",
            h1, h2, type.pretty_print(), p->remaining());
      find_by_type_value();
      break;

    case ATT::OPCODE_READ_BY_TYPE_REQUEST :
      rsp_opcode = ATT::OPCODE_READ_BY_TYPE_RESPONSE;
      if (!read_handles()) break;
      if (!read_type()) break;
      debug("ATT: read %04x-%04x by type: %s\n", h1, h2, type.pretty_print());
      read_by_type();
      break;

    case ATT::OPCODE_READ_BY_GROUP_TYPE_REQUEST :
      rsp_opcode = ATT::OPCODE_READ_BY_GROUP_TYPE_RESPONSE;
      if (!read_handles()) break;
      if (!read_type()) break;
      debug("ATT: read %04x-%04x by group type: %s\n", h1, h2, type.pretty_print());
      read_by_group_type();
      break;

    case ATT::OPCODE_READ_REQUEST :
      rsp_opcode = ATT::OPCODE_READ_RESPONSE;
      *req >> h1;
      debug("ATT: read handle 0x%04x\n", h1);

      if (!(attr = AttributeBase::get(h1))) {
        error(ATT::INVALID_HANDLE);
      } else {
        rsp = req;
        rsp->l2cap(att_mtu) << rsp_opcode;
        rsp->write((uint8_t *) attr->_data, std::min(rsp->remaining(), attr->length));
      }
      break;
    
    case ATT::OPCODE_READ_BLOB_REQUEST :
      rsp_opcode = ATT::OPCODE_READ_BLOB_RESPONSE;
      *req >> h1;

      if (!(attr = AttributeBase::get(h1))) {
        error(ATT::INVALID_HANDLE);
      } else {
        rsp = req;
        rsp->l2cap(att_mtu) << rsp_opcode;
    
        if (attr->length < att_mtu) {
          error(ATT::ATTRIBUTE_NOT_LONG);
        } else if (offset >= attr->length) {
          error(ATT::INVALID_OFFSET);
        } else {
          rsp->write(((uint8_t *) attr->_data) + offset, std::min(rsp->remaining(), attr->length));
        }
      }
      break;

    default :
      debug("unrecognized att opcode: 0x%02x\n", req_opcode);
      break;
    }

    if (rsp != 0) {
      rsp->title = "response";
      send(rsp);
    }

    if (req != rsp) req->deallocate();

    req = rsp = 0;
  }
};
