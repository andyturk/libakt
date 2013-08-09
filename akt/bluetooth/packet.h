// -*- Mode:C++ -*-

#pragma once

#include "akt/bluetooth/bluetooth_constants.h"
#include "akt/bluetooth/bd_addr.h"
#include "akt/pool.h"
#include "akt/flipbuffer.h"
#include "akt/ring.h"
#include "akt/assert.h"

#include <stdint.h>
#include <cstring>

namespace akt {
  namespace bluetooth {
    using namespace HCI;

    class Packet : public Ring<Packet>, public FlipBuffer<uint8_t> {
    public:
      const char *title;
      Ring<Packet> *free_packets;

    Packet() :
      title(0),
        free_packets(0)
          {}

    Packet(uint8_t *buf, uint16_t len) :
      FlipBuffer(buf, len),
      title(0),
      free_packets(0)
    {}

      void deallocate() {
        assert(free_packets != 0);
        join(free_packets);
      }

      Packet &operator<<(uint8_t x) {
        FlipBuffer<uint8_t>::operator<<(x);
        return *this;
      }

      Packet &operator>>(uint8_t &x) {
        FlipBuffer<uint8_t>::operator>>(x);
        return *this;
      }

      Packet &operator<<(uint16_t x) {
        assert(pos+sizeof(x) <= lim);
        storage[pos++] = x & 0xff;
        storage[pos++] = x >> 8;
        return *this;
      }

      Packet &operator>>(uint16_t &x) {
        assert(pos+sizeof(x) <= lim);
        x = (storage[pos+0] <<  0) + (storage[pos+1] <<  8);
        pos += 2;
        return *this;
      }

      Packet &operator<<(uint32_t x) {
        assert(pos+sizeof(x) <= lim);
        storage[pos++] = x & 0xff;
        storage[pos++] = x >> 8;
        storage[pos++] = x >> 16;
        storage[pos++] = x >> 24;
        return *this;
      }

      Packet &operator>>(uint32_t &x) {
        assert(pos+sizeof(x) <= lim);
        x = (storage[pos+0] <<  0) + (storage[pos+1] <<  8)
          + (storage[pos+2] << 16) + (storage[pos+3] << 24);
        pos += 4;
        return *this;
      }


      operator uint8_t *() {return storage + pos;}

      void prepare_for_tx() {

        if (pos != 0) flip();

        switch (storage[0]) {
        case COMMAND_PACKET :
          seek(3);
          *this << (uint8_t) (lim - 4); // command length
          break;

        case ACL_PACKET :
          seek(3);
          *this << (uint16_t) (lim - 5); // acl length
          *this << (uint16_t) (lim - 9); // l2cap length
          break;
        }

        seek(0);

        if (title) {
#ifdef DEBUG
          debug("%s:\n", title);
          dump();
#endif
          title = 0;
        }
      }

      enum {
        HCI_HEADER_SIZE = 1,
        ACL_HEADER_SIZE = HCI_HEADER_SIZE + 4,
        L2CAP_HEADER_SIZE = ACL_HEADER_SIZE + 4
      };

      Packet &uart(enum packet_indicator ind) {
        reset();
        return *this << (uint8_t) ind;
      }

      Packet &hci(HCI::opcode c) {
        return uart(COMMAND_PACKET) << (uint16_t) c << (uint8_t) 0;
      }

      Packet &hci(HCI::event e) {
        return uart(EVENT_PACKET) << (uint8_t) e;
      }

      Packet &acl(uint16_t handle, uint8_t pb, uint8_t bc) {
        return uart(ACL_PACKET) << (uint16_t) ((bc << 14) + (pb << 12) + handle) << (uint16_t) 0;
      }

      Packet &l2cap(uint16_t handle, uint16_t cid) {
        return acl(handle, 0x02, 0x00) << (uint16_t) 0 << cid;
      }

      /*
       * The following two methods re-use existing L2CAP framing information.
       * The packet is reset, but the L2CAP framing is either preserved
       * in place or copied from another packet. This includes the
       * source/destination handle and the channel ID. The position is left
       * at the first L2CAP payload byte. Length fields in the framing are not
       * modified here and must be set (via prepare_for_tx) before the packet
       * can be sent.
       */
      Packet &l2cap(uint16_t lim=0) {
        reset(lim);
        seek(L2CAP_HEADER_SIZE);
        return *this;
      }

      Packet &l2cap(Packet *other, uint16_t lim=0) {
        reset(lim);
        write(other->storage, L2CAP_HEADER_SIZE);
        return *this;
      }

#ifdef DEBUG
      void dump() {
        void dump_hex_bytes(uint8_t *, size_t);

        dump_hex_bytes((uint8_t *) *this, remaining());
        debug("\n");
      }
#endif
    };

    template<unsigned int size>
      class SizedPacket : public Packet {
      uint8_t _storage[size];
    public:
    SizedPacket() : Packet(_storage, size) {}
    };

    template<unsigned int packet_size, unsigned int packet_count>
      class PacketPool : public Pool<SizedPacket<packet_size>, packet_count> {
    public:
    PacketPool() : Pool<SizedPacket<packet_size>, packet_count>() {
        for (unsigned int i=0; i < packet_count; ++i) {
          this->pool[i].free_packets = (Ring<Packet> *) &this->available;
        }
      }
    };
  };
};
