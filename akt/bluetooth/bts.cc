#ifndef __arm__
#include "bts.h"

#include <cstdio>
#include <iostream>
#include <fstream>
#include <iomanip>

using namespace std;
using namespace akt::bluetooth;

namespace akt {
  namespace bluetooth {
    Script::Script(uint8_t *bytes, uint16_t length) :
      script(bytes, length)
    {
    }


    void Script::reset(const uint8_t *bytes, uint16_t length) {
      script_header h;

      script.init((uint8_t *) bytes, length);
      assert(script.remaining() >= sizeof(h));
      script.read((uint8_t *) &h, sizeof(h));
      header(h);
    }

    void Script::header(script_header &h) {
      if (h.magic != BTSB) error("bad magic");
    }


    void Script::send(Packet &action) {
    }

    void Script::expect(uint32_t msec, Packet &action) {
    }

    void Script::configure(uint32_t baud, flow_control control) {
    }

    void Script::call(const char *filename) {
    }

    void Script::comment(const char *text) {
    }

    void Script::error(const char *reason) {
    }

    void Script::done() {
    }

    void Script::play_next_action() {
      if (script.remaining() == 0) {
        done();
      } else if(script.remaining() < sizeof(command_header)) {
        error("bad script");
      } else {
        uint8_t *start = (uint8_t *) script;
        uint16_t action, length;

        script >> action >> length;
        command.init(start, length + sizeof(command_header));
        script += length;
        command.seek(sizeof(command_header));

        switch (action) {
        case SEND_COMMAND : {
          if (command[0] == HCI::COMMAND_PACKET) {
            uint8_t indicator;
            uint16_t p = command.tell();
            command >> indicator >> last_opcode;
            //debug("sending 0x%04x\n", last_opcode);
            command.seek(p);
          }
          send(command);
          break;
        }

        case WAIT_EVENT : {
          uint32_t msec, length2;

          command >> msec >> length2;
          expect(msec, command);
          break;
        }

        case SERIAL_PORT_PARAMETERS : {
          configuration config;
          command >> config.baud >> config.control;
          command.rewind();
          configure(config.baud, (flow_control) config.control);
          break;
        }

        case RUN_SCRIPT : {
          call((char *) (uint8_t *) command);
          break;
        }

        case REMARKS : {
          comment((char *) (uint8_t *) command);
          break;
        }

        default :
          error("bad action");
          return;
        }
      }
    }

    SourceGenerator::SourceGenerator(const char *n, uint8_t *bytes, uint16_t length) :
      Script(bytes, length),
      name(n),
      out(&cout)
    {
      *out << "extern \"C\" const uint8_t " << name << "[] = {\n";
    }

    SourceGenerator::SourceGenerator(const char *name) :
      Script(0, 0),
      name(name),
      out(&cout)
    {
      *out << "extern \"C\" const uint8_t " << name << "[] = {\n";
    }

    SourceGenerator::~SourceGenerator() {
      *out << "};\n";
      *out << "extern \"C\" const uint32_t " << name << "_size = sizeof(" << name << ");\n";
    }

    void SourceGenerator::emit(const uint8_t *bytes, uint16_t size) {
      reset(bytes, size);
      while (!is_complete()) play_next_action();
    }

    void SourceGenerator::as_hex(const uint8_t *bytes, uint16_t size, const char *start) {
      const uint8_t *limit = bytes + size;

      *out << hex << setfill('0');
      while (bytes < limit) {
        if (start) *out << start;
        for (int i=0; i < 16 && bytes < limit; ++i)
          *out << "0x" << setw(2) << (int) *bytes++ << ", ";
        *out << "\n";
      }
      *out << dec;
    }

    bool omit_opcode(uint16_t opcode) {
      switch (opcode) {
      case OPCODE_SLEEP_MODE_CONFIGURATIONS :
      case OPCODE_HCILL_PARAMETERS :
      case OPCODE_PAN13XX_CHANGE_BAUD_RATE :
        return true;

      default :
        return false;
      }
    }

    void SourceGenerator::send(Packet &action) {
      command_header ch = {SEND_COMMAND, action.remaining()};

      uint16_t pos = action.tell();

      uint8_t indicator;
      bool omitted = false;

      action >> indicator;

      if (indicator == COMMAND_PACKET) {
        uint16_t opcode;
        action >> opcode;

        if (omit_opcode(opcode)) {
          omitted = true;
          *out << "/* omitting \n";
        }
      }

      action.rewind(pos);
      as_hex((uint8_t *) action, action.remaining(), "  ");

      if (omitted) {
        *out << " */";
      }
      *out << endl;
    }

    void SourceGenerator::expect(uint32_t msec, Packet &action) {
      *out << "// within " << msec << " msec expect:" << endl;
      as_hex((uint8_t *) action, action.remaining(), "// ");
      *out << endl;
    }

    void SourceGenerator::configure(uint32_t baud, flow_control control) {
      const char *c;

      switch (control) {
      case NONE : c = "NONE"; break;
      case HARDWARE : c = "HARDWARE"; break;
      case NO_CHANGE : c = "NO_CHANGE"; break;
      default :
        cerr << "bad flow control specification\n";
        exit(1);
      }

      *out << "// set baud to " << baud << " with flow control: " << c << endl;
      //command.rewind();
      //as_hex((uint8_t *) command, command.remaining(), "  ");
    }

    void SourceGenerator::call(const char *filename) {
      cerr << "script chaining not supported\n";
      exit(1);
    }

    void SourceGenerator::comment(const char *text) {
      *out << "// " << text << endl;
    }

    void SourceGenerator::error(const char *msg) {
      cerr << msg;
      exit(1);
    }
  };
};


void file_as_bytes(const char *file, uint8_t *&value, size_t &size) {
  value = 0;
  size = 0;

  ifstream bts_file(file, ios::in | ios::binary | ios::ate);

  if (bts_file.is_open()) {
    bts_file.seekg(0, ios::end);
    size = bts_file.tellg();
    bts_file.seekg(0, ios::beg);

    value = new uint8_t[size];
    bts_file.read((char *) value, size);
  }
}

void oem_cc2564_script(const char *file) {
  uint8_t *raw_patch;
  size_t raw_patch_size;

  file_as_bytes(file, raw_patch, raw_patch_size);

  if (raw_patch == 0 || raw_patch_size == 0) {
    cerr << "couldn't load " << file << endl;
    exit(1);
  }

  akt::bluetooth::SourceGenerator code("bluetooth_init_cc2564", raw_patch, raw_patch_size);
  akt::bluetooth::SizedPacket<259> p;


  // send OEM patch
  code.comment(file);
  code.emit(raw_patch, raw_patch_size);
  delete raw_patch;
}

void cold_boot_script() {
  akt::bluetooth::SourceGenerator code("cold_boot");
  akt::bluetooth::SizedPacket<259> p;

  code.comment("Reset Pan13XX");
  p.hci(HCI::OPCODE_RESET);
  p.prepare_for_tx();
  code.send(p);

  code.comment("Read local version info");
  p.hci(HCI::OPCODE_READ_LOCAL_VERSION_INFORMATION);
  p.prepare_for_tx();
  code.send(p);

  // code.comment("Change baud rate to 921600 since we'll be downloading large patches");
  // (p.hci(HCI::OPCODE_PAN13XX_CHANGE_BAUD_RATE) << (uint32_t) 921600L).flip();
  // code.send(p);
}

void warm_boot_script() {
  SourceGenerator code("warm_boot");
  SizedPacket<259> p;

  code.comment("Disable sleep modes");
  p.hci(HCI::OPCODE_SLEEP_MODE_CONFIGURATIONS);
  p << (uint8_t) 0x01; // big sleep enable
  p << (uint8_t) 0x00; // deep sleep enable
  p << (uint8_t) 0x00; // deep sleep mode (0xff = no change)
  p << (uint8_t) 0xff; // output I/O select (0xff = no change)
  p << (uint8_t) 0xff; // output pull enable (0xff = no change)
  p << (uint8_t) 0xff; // input pull enable (0xff = no change)
  p << (uint8_t) 0xff; // input I/O select (0xff = no change)
  p << (uint16_t) 100; // reserved -- must be 0?
  p.prepare_for_tx();
  code.send(p);

  code.comment("get BD_ADDR");
  p.hci(HCI::OPCODE_READ_BD_ADDR);
  p.prepare_for_tx();
  code.send(p);

  code.comment("get buffer size info");
  p.hci(HCI::OPCODE_READ_BUFFER_SIZE_COMMAND);
  p.prepare_for_tx();
  code.send(p);

  code.comment("write page timeout");
  (p.hci(HCI::OPCODE_WRITE_PAGE_TIMEOUT) << (uint16_t) 0x2000);
  p.prepare_for_tx();
  code.send(p);

  code.comment("read page timeout");
  (p.hci(HCI::OPCODE_READ_PAGE_TIMEOUT));
  p.prepare_for_tx();
  code.send(p);

  code.comment("write local name");
  char local_name[248];
  strncpy(local_name, "Super Whizzy Gizmo 1.1", sizeof(local_name));
  p.hci(HCI::OPCODE_WRITE_LOCAL_NAME_COMMAND);
  p.write((uint8_t *) local_name, sizeof(local_name));
  p.prepare_for_tx();
  code.send(p);

  code.comment("write scan enable");
  (p.hci(HCI::OPCODE_WRITE_SCAN_ENABLE) << (uint8_t) 0x03);
  p.prepare_for_tx();
  code.send(p);

  code.comment("write class of device");
  code.comment("see http://bluetooth-pentest.narod.ru/software/bluetooth_class_of_device-service_generator.html");
  p.hci(HCI::OPCODE_WRITE_CLASS_OF_DEVICE);
  p << (uint32_t) 0x0098051c;
  p.prepare_for_tx();
  code.send(p);

  code.comment("set event mask");
  p.hci(HCI::OPCODE_SET_EVENT_MASK) << (uint32_t) 0xffffffff << (uint32_t) 0x20001fff;
  p.prepare_for_tx();
  code.send(p);

  code.comment("write le host support");
  p.hci(HCI::OPCODE_WRITE_LE_HOST_SUPPORT) << (uint8_t) 1 << (uint8_t) 1;
  p.prepare_for_tx();
  code.send(p);

  code.comment("le set event mask");
  p.hci(HCI::OPCODE_LE_SET_EVENT_MASK) << (uint32_t) 0x0000001f << (uint32_t) 0x00000000;
  p.prepare_for_tx();
  code.send(p);

  code.comment("le read buffer size");
  p.hci(HCI::OPCODE_LE_READ_BUFFER_SIZE);
  p.prepare_for_tx();
  code.send(p);

  code.comment("le read supported states");
  p.hci(HCI::OPCODE_LE_READ_SUPPORTED_STATES);
  p.prepare_for_tx();
  code.send(p);

  BD_ADDR dummy_addr;
  code.comment("le set advertising parameters");
  p.hci(HCI::OPCODE_LE_SET_ADVERTISING_PARAMETERS);
  p << (uint16_t) 0x0800;
  p << (uint16_t) 0x4000;
  p << (uint8_t) 0x00;
  p << (uint8_t) 0x00;
  p << (uint8_t) 0x00;
  p.write((uint8_t *) &dummy_addr, sizeof(dummy_addr));
  p << (uint8_t) 0x07;
  p << (uint8_t) 0x00;
  p.prepare_for_tx();
  code.send(p);

  // https://www.bluetooth.org/Technical/AssignedNumbers/generic_access_profile.htm
  // AD #1, data type (0x01) <<Flags>>
  // AD #2, data type (0x02) <<Incomplete list of 16-bit service UUIDs>>

  /*
    uint8_t num_parameters = 0;
    uint8_t advertising_data[] = {02, 01, 05, 03, 02, 0xf0, 0xff, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0};
  */

  {
    code.comment("le set advertising data");
    p.hci(HCI::OPCODE_LE_SET_ADVERTISING_DATA);
    uint8_t *start0 = (uint8_t *) p;

    p << (uint8_t) 0; // dummy

    Packet adv((uint8_t *) p, 31);
    uint8_t *start;

    // data item 1
    start = (uint8_t *) adv;
    adv << (uint8_t) 0; // dummy length
    adv << (uint8_t) GAP::FLAGS;
    adv << (uint8_t) (GAP::LE_LIMITED_DISCOVERABLE_MODE | GAP::BR_EDR_NOT_SUPPORTED);
    *start = (((uint8_t *) adv) - start) - 1;

    // data item 2
    start = (uint8_t *) adv;
    adv << (uint8_t) 0; // dummy length
    adv << (uint8_t) GAP::INCOMPLETE_16BIT_UUIDS;
    adv << (uint16_t) 0xfff0; // demo UUID not in the Bluetooth spec
    *start = (((uint8_t *) adv) - start) - 1;
  
    *start0 = adv.tell();

    // zero pad to 31 bytes
    while (adv.tell() < 31) adv << (uint8_t) 0;
    assert(adv.tell() == 31);
  
    p += 31;
    p.prepare_for_tx();
    code.send(p);
  }

  {
    code.comment("le set scan response data");
    p.hci(OPCODE_LE_SET_SCAN_RESPONSE_DATA);
    uint8_t *start0 = (uint8_t *) p;

    p << (uint8_t) 0; // dummy

    Packet adv((uint8_t *) p, 31); // a piece of the main packet
    uint8_t *start;

    // data item 1
    start = (uint8_t *) adv;
    adv << (uint8_t) 0; // dummy length
    adv << (uint8_t) GAP::FLAGS;
    adv << (uint8_t) (GAP::LE_LIMITED_DISCOVERABLE_MODE | GAP::BR_EDR_NOT_SUPPORTED);
    *start = (((uint8_t *) adv) - start) - 1;

    // data item 2
    start = (uint8_t *) adv;
    adv << (uint8_t) 0; // dummy length
    adv << (uint8_t) GAP::INCOMPLETE_16BIT_UUIDS;
    adv << (uint16_t) 0xfff0; // demo UUID not in the Bluetooth spec
    *start = (((uint8_t *) adv) - start) - 1;

    *start0 = adv.tell();

    // zero pad to 31 bytes
    while (adv.tell() < 31) adv << (uint8_t) 0;
    assert(adv.tell() == 31);

    p += 31;
    p.prepare_for_tx();
    code.send(p);
  }

  code.comment("enable le advertising");
  p.hci(HCI::OPCODE_LE_SET_ADVERTISE_ENABLE) << (uint8_t) 0x01;
  p.prepare_for_tx();
  code.send(p);
}

int main(int argc, char *argv[]) {
  if (argc < 3) {
    cerr << "usage: bts <bts file> <decl name>\n";
    exit(1);
  }

  cout << "#include <stdint.h>\n";

  oem_cc2564_script(argv[1]);
  cold_boot_script();
  warm_boot_script();

  return 0;
}
#endif
