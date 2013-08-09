// -*- Mode:C++ -*-

#pragma once

#include "akt/bluetooth/att.h"

namespace akt::bluetooth {
  struct CharacteristicDecl : public AttributeBase {
    struct {
      uint8_t properties;
      uint16_t handle;
      union {
        uint8_t full_uuid[sizeof(UUID)];
        uint16_t short_uuid;
      };
    } __attribute__ ((packed)) _decl ;

    CharacteristicDecl(uint16_t uuid);
    CharacteristicDecl(const UUID &uuid);
  };

  template<typename T>
    struct Characteristic : public CharacteristicDecl {
    Attribute<T> value;
  Characteristic(const UUID &uuid) : CharacteristicDecl(GATT::CHARACTERISTIC), value(uuid) {
      _decl.handle = value.handle;
    }
  Characteristic(uint16_t uuid) : CharacteristicDecl(GATT::CHARACTERISTIC), value(uuid) {
      _decl.handle = value.handle;
    }
    Characteristic &operator=(const T &rhs) {value = rhs; return *this;}
  };

  struct GATT_Service : public Attribute<uint16_t> {
    CharacteristicDecl changed;

    GATT_Service();
    virtual uint16_t group_end();
  };


  struct GAP_Service : public Attribute<uint16_t> {
    Characteristic<const char *> device_name;
    Characteristic<uint16_t> appearance;

    GAP_Service(const char *name, uint16_t a = 0);
    virtual uint16_t group_end();
  };
};
