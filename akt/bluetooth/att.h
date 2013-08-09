// -*- Mode:C++ -*-

#pragma once

#include "akt/bluetooth/l2cap.h"
#include "akt/bluetooth/packet.h"
#include "akt/ring.h"

#include <stdint.h>

namespace akt::bluetooth {
  class AttributeBase : public Ring<AttributeBase> {
    enum {MAX_ATTRIBUTES = 20};
    static uint16_t next_handle;
    static AttributeBase *all_handles[MAX_ATTRIBUTES];

  public:
    UUID type;
    uint16_t handle;
    void *_data;
    uint16_t length;

    AttributeBase(const UUID &t, void *d, uint16_t l);
    AttributeBase(int16_t t, void *d, uint16_t l);

    static AttributeBase *get(uint16_t h) {return (h < next_handle) ? all_handles[h] : 0;}
    static uint16_t find_by_type_value(uint16_t start, uint16_t type, void *value, uint16_t length);
    static uint16_t find_by_type(uint16_t start, const UUID &type);
    int compare(void *data, uint16_t len);
    int compare(void *data, uint16_t len, uint16_t min_handle, uint16_t max_handle);
    virtual uint16_t group_end();

    static void dump_attributes();
  };

  template<typename T> class Attribute : public AttributeBase {
    T data;

  public:
  Attribute(const UUID &u) : AttributeBase(u, &data, sizeof(data)) {}
  Attribute(uint16_t u) : AttributeBase(u, &data, sizeof(data)) {}
  Attribute(const UUID &u, const T &val) : AttributeBase(u, &data, sizeof(data)), data(val) {}
  Attribute(uint16_t u, const T &val) : AttributeBase(u, &data, sizeof(data)), data(val) {}

    Attribute &operator=(const T &rhs) {data = rhs; return *this;};
  };

  template<> class Attribute<const char *> : public AttributeBase {
  public:
  Attribute(const UUID &u) : AttributeBase(u, 0, 0) {}
  Attribute(uint16_t u) : AttributeBase(u, 0, 0) {}

  Attribute &operator=(const char *rhs) {_data = (void *) rhs; length = strlen(rhs); return *this;};
  };

  class ATT_Channel : public Channel {
    void error(uint8_t err);
    bool read_handles();
    bool read_type();
    bool is_grouping(const UUID &type);

    void find_information();
    void find_by_type_value();
    void read_by_type();
    void read_by_group_type();

    uint16_t client_rx_mtu, att_mtu;

    // parsed PDU parameters
    uint8_t req_opcode, rsp_opcode;
    uint16_t h1, h2, offset;
    UUID type;
    Packet *req;
    Packet *rsp;

  public:
    ATT_Channel(HostController &hc);
    void receive(Packet *p);
  };
};
