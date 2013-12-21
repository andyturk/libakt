// -*- Mode:C++ -*-
#pragma once

namespace akt {
  inline unsigned short isqrt(unsigned long a) {
    unsigned long rem = 0;
    unsigned int root = 0;
    int i;

    for (i = 0; i < 16; i++) {
      root <<= 1;
      rem <<= 2;
      rem += a >> 30;
      a <<= 2;

      if (root < rem) {
        root++;
        rem -= root;
        root++;
      }
    }

    return (unsigned short) (root >> 1);
  }
};
