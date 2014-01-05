#include "akt/unique.h"

#include "hal.h"

#include <stddef.h>

using namespace akt;

enum {
  STM32F4_UNIQUE_ID_REGISTER   = 0x1fff7a10u,
  STM32F103_UNIQUE_ID_REGISTER = 0x1ffff7e8u
};

#if defined(STM32F4XX)
#define UNIQUE_ID_REGISTER STM32F4_UNIQUE_ID_REGISTER
#elif defined(STM32F1XX)
#define UNIQUE_ID_REGISTER STM32F103_UNIQUE_ID_REGISTER
#else
#error "unique.cc doesn't know which type of mcu"
#endif

// copied from http://www.partow.net/programming/hashfunctions/index.html
unsigned int rs_hash(const uint8_t *in, size_t len) {
    unsigned int b    = 378551;
    unsigned int a    = 63689;
    unsigned int hash = 0;

    for(size_t i = 0; i < len; i++) {
        hash = hash * a + in[i];
        a    = a * b;
    }

    return hash;
}

void UniqueID::raw(id_t &result) {
    const id_t *hw_reg = (const id_t *) UNIQUE_ID_REGISTER;

    result.words[0] = hw_reg->words[0];
    result.words[1] = hw_reg->words[1];
    result.words[2] = hw_reg->words[2];
}

uint32_t UniqueID::hashed() {
    const id_t *hw_reg = (const id_t *) UNIQUE_ID_REGISTER;

    return rs_hash(hw_reg->bytes, sizeof(hw_reg->bytes));
}

const char *UniqueID::hex() {
    static char result[7];

    if (result[0] == 0) {
        uint32_t h = hashed();

        for(int i=0; i < 6; ++i) {
            uint32_t ch = h % 36; // 36 == A-Z plus 0-9

            if (ch < 10) {
                result[i] = '0' + ch;
            } else {
                result[i] = 'A' + ch - 10;
            }

            h = h/36;
        }

        result[6] = '\0';
    }

    return result;
}
