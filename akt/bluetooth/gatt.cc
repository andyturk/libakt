#include "akt/bluetooth/gatt.h"

CharacteristicDecl::CharacteristicDecl(uint16_t uuid) :
  AttributeBase(GATT::CHARACTERISTIC, &_decl, sizeof(_decl))
{
  _decl.properties = 0;
  _decl.handle = 0;
  _decl.short_uuid = (uint16_t) uuid;
  length = sizeof(uint8_t) + sizeof(uint16_t) + sizeof(uint16_t);
}

CharacteristicDecl::CharacteristicDecl(const UUID &uuid) :
  AttributeBase(GATT::CHARACTERISTIC, &_decl, sizeof(_decl))
{
  _decl.properties = 0;
  _decl.handle = 0;
  memcpy(_decl.full_uuid, (const uint8_t *) uuid, sizeof(UUID));
  length = sizeof(uint8_t) + sizeof(uint16_t) + sizeof(UUID);
}

GATT_Service::GATT_Service() :
  Attribute<uint16_t>(GATT::PRIMARY_SERVICE, GATT::GENERIC_ATTRIBUTE_PROFILE),
  changed(GATT::SERVICE_CHANGED)
{
  changed._decl.properties = GATT::INDICATE | GATT::WRITE_WITHOUT_RESPONSE | GATT::READ;
}

uint16_t GATT_Service::group_end() {
  return changed.handle;
}

GAP_Service::GAP_Service(const char *name, uint16_t a) :
  Attribute<uint16_t>(GATT::PRIMARY_SERVICE, GATT::GENERIC_ACCESS_PROFILE),
  device_name(GATT::DEVICE_NAME),
  appearance(GATT::APPEARANCE)
{
  appearance = a;
  device_name = name;
}

uint16_t GAP_Service::group_end() {
  return appearance.handle;
}
