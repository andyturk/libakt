#pragma once

#include <cstdint>

namespace akt {
    class UniqueID {
    public:
        typedef union {
            uint8_t bytes[12];
            uint32_t words[3];
        } id_t;

        static void raw(id_t &result);
        static uint32_t hashed();
        static const char *hex();
    };
};
