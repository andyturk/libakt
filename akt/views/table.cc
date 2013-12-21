#include "table.h"

using namespace akt::views;

Table::Table(CellDataInterface &data, FontBase &font, pixel fg, pixel bg) :
  View(),
  data(data),
  font(font),
  fg(fg), bg(bg),
  draw_border(false),
  draw_grid(false)
{
}

void Table::draw_self(Canvas &c) {
  c.fill_rect(frame, bg);
  Rect original_clip = c.clip;
  unsigned rows = data.rows();
  unsigned cols = data.cols();

  coord row_start = frame.min.y;
  for (unsigned row=0; row < rows; ++row) {
    unsigned vgaps = row < (rows-1) ? (rows-row) : 0;
    coord row_height = ((frame.max.y - row_start) - vgaps*GAP)/(rows-row);

    coord col_start = frame.min.x;
    for (unsigned col=0; col < cols; ++col) {
      unsigned hgaps = col < (cols-1) ? (cols-col) : 0;
      coord col_width = ((frame.max.x - col_start) - hgaps*GAP)/(cols-col);
      Rect cell_rect(Point(col_start, row_start),
                     Point(col_start + col_width, row_start + row_height));

      const char *contents = data.contents(row, col);

      if (contents && *contents) {
        Size s = font.measure(contents);
        c.clip = original_clip & cell_rect;
        c.draw_string(cell_rect.center() - s/2, contents, font, fg);
      }

      col_start += col_width + GAP;
    }

    row_start += row_height + GAP;
  }

  c.clip = original_clip;

  if (draw_border) {
    Point bottom_right(frame.max.x-1, frame.max.y-1);

    c.draw_line(frame.min, Point(bottom_right.x, frame.min.y), fg);
    c.draw_line(frame.min, Point(frame.min.x, bottom_right.y), fg);
    c.draw_line(bottom_right, Point(frame.min.x, bottom_right.y), fg);
    c.draw_line(bottom_right, Point(bottom_right.x, frame.min.y), fg);
  }

  if (draw_grid) {
    row_start = frame.min.y;
    for (unsigned row=0; row < rows; ++row) {
      unsigned vgaps = row < (rows-1) ? (rows-row) : 0;
      coord row_height = ((frame.max.y - row_start) - vgaps*GAP)/(rows-row);

      row_start += row_height;
      c.draw_line(Point(frame.min.x, row_start),
                  Point(frame.max.x, row_start), fg);

      row_start += GAP;
    }

    coord col_start = frame.min.x;
    for (unsigned col=0; col < cols; ++col) {
      unsigned hgaps = col < (cols-1) ? (cols-col) : 0;
      coord col_width = ((frame.max.x - col_start) - hgaps*GAP)/(cols-col);

      col_start += col_width;
      c.draw_line(Point(col_start, frame.min.y),
                  Point(col_start, frame.max.y), fg);

      col_start += GAP;
    }

  }
}

Size Table::good_size() const {
  coord height = data.rows()*font.size.h;

  if(data.rows() > 0) {
    height += data.rows()*GAP;
  }

  return Size(0, height);
}
