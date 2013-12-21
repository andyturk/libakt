#pragma once

#include "akt/views/views.h"

namespace akt {
  namespace views {

    class Table : public View {
    public:
      enum {
        GAP = 2
      };

      struct CellDataInterface {
        virtual unsigned rows() const = 0;
        virtual unsigned cols() const = 0;
        virtual const char *contents(unsigned row, unsigned col) const = 0;
      };

      Table(CellDataInterface &data, FontBase &font, pixel fg, pixel bg);

      virtual void draw_self(Canvas &c);
      virtual Size good_size() const;

    public:
      CellDataInterface &data;
      FontBase &font;
      pixel fg, bg;
      bool draw_border;
      bool draw_grid;
    };
  };
};
