// -*- Mode:C++ -*-

#include "akt/views/views.h"
#include "ch.h"
#include "hal.h"

namespace akt {
  namespace views {

    void number_to_string(uint32_t number, char *string, unsigned int string_length) {
        while (string_length > 0) {
            string[--string_length] = (char)('0' + number % 10);
            number = number/10;
        }
    }


    Point::Point() {
    }

    Point::Point(int x, int y) :
      x(x),
      y(y)
    {}

    Point &Point::operator +=(const Point &p) {
      x += p.x;
      y += p.y;
      return *this;
    }

    Size::Size() {
    }

    Size::Size(int w, int h) :
      w(w),
      h(h)
    {}

    Rect::Rect() {
    }

    Rect::Rect(int x, int y, int w, int h) :
      min(x, y),
      max(x + w, y + h)
    {}

    Rect::Rect(const Point &p0, const Point &p1) :
      min(p0),
      max(p1)
    {
      normalize();
    }

    Rect::Rect(const Point &p, const Size &s) :
      min(p),
      max(p.x + s.w, p.y + s.h)
    {
      normalize();
    }

    void Rect::normalize() {
      Point p1(std::min(min.x, max.x), std::min(min.y, max.y));
      Point p2(std::max(min.x, max.x), std::max(min.y, max.y));
      min = p1;
      max = p2;
    }

    bool Rect::intersects(const Rect &r) const {
      Rect i = r & *this;
      return ((i.max.x - i.min.x) > 0) && ((i.max.y - i.min.y) > 0);
    }

    bool Rect::contains(const Point &p) const {
      return (p.x >= min.x) && (p.x <= max.x) &&
        (p.y >= min.y) && (p.y <= max.y);
    }

    bool Rect::contains(const Rect &r) const {
      return contains(r.min) && contains(r.max);
    }

    Rect &Rect::operator +=(const Point &p) {
      min += p;
      max += p;
      return *this;
    }

    Rect &Rect::operator=(const Point &p) {
      coord w = width(), h = height();
      min = p;
      max.x = min.x + w;
      max.y = min.y + h;
      return *this;
    }

    Rect Rect::operator +(Size s) {
      return Rect(min.x + s.w, min.y + s.h, max.x + s.w, max.y + s.h);
    }

    Rect Rect::operator -(Size s) {
      return Rect(min.x - s.w, min.y - s.h, max.x - s.w, max.y - s.h);
    }

    Point Rect::center() const {
      return Point((min.x + max.x)/2, (min.y + max.y)/2);
    }

    /*
     * Note that a point that lies on the right or bottom edges of
     * a rectangle generates a non-zero outcode even though the point
     * is technically "within" the rectangle. This is because the
     * main purpose of outcode() is to generate points suitable for
     * line drawing and points on these two edges will actually be
     * rendered outside the rectangle.
     */
    int Rect::outcode(const Point &p) const {
      int code = INSIDE;

      if (p.x <  min.x) code |= LEFT;
      if (p.x >= max.x) code |= RIGHT;
      if (p.y <  min.y) code |= TOP;
      if (p.y >= max.y) code |= BOTTOM;

      return code;
    }

    /*
     * This is a tweaked version of the Cohen-Sutherland algorithm
     * for clipping 2D lines.
     *
     * See http://en.wikipedia.org/wiki/Cohen-Sutherland_algorithm
     */
    bool Rect::clip(Point &p0, Point &p1) const {
      int out0 = outcode(p0), out1 = outcode(p1);

      while (true) {
        if ((out0 | out1) == INSIDE) return true;
        if (out0 & out1) return false;
        int out = out0 ? out0 : out1;
        int dx = (p1.x - p0.x), dy = (p1.y - p0.y);
        Point p;

        if (out & TOP) {
          p.x = p0.x + dx*(min.y - p0.y)/dy;
          p.y = min.y;
        } else if (out & BOTTOM) {
          p.x = p0.x + dx*(max.y - (p0.y + 1))/dy;
          p.y = max.y - 1;
        } else if (out & RIGHT) {
          p.y = p0.y + dy*(max.x - (p0.x + 1))/dx;
          p.x = max.x - 1;
        } else if (out & LEFT) {
          p.y = p0.y + dy*(min.x - p0.x)/dx;
          p.x = min.x;
        }

        if (out == out0) {
          out0 = outcode((p0 = p));
        } else {
          out1 = outcode((p1 = p));
        }
      }
    }

    bool operator==(const Rect &r1, const Rect &r2) {
      return r1.min == r2.min && r1.max == r2.max;
    }

    Rect Rect::operator &(const Rect &right) const {
      chDbgAssert(normal() && right.normal(), __FILE__, __LINE__);

      Rect i; // intersection

      if (max.x >= right.min.x && min.x <= right.max.x) {
        i.min.x = std::max(min.x, right.min.x);
        i.max.x = std::min(max.x, right.max.x);
      } else {
        i.min.x = max.x; // abnormal
        i.max.x = min.x;
      }

      if (max.y >= right.min.y && min.y <= right.max.y) {
        i.min.y = std::max(min.y, right.min.y);
        i.max.y = std::min(max.y, right.max.y);
      } else {
        i.min.y = max.y; // abnormal
        i.max.y = min.y;
      }

      return i;
    }

    Rect operator |(const Rect &left, const Rect &right) {
      chDbgAssert(left.normal() && right.normal(), __FILE__, __LINE__);

      Rect u; // union

      u.min.x = std::min(left.min.x, right.min.x);
      u.min.y = std::min(left.min.y, right.min.y);
      u.max.x = std::max(left.max.x, right.max.x);
      u.max.y = std::max(left.max.y, right.max.y);

      return u;
    }

    Rect center_size_within(const Rect &r, const Size &s) {
      Point upper_left = r.center() - s/2;
      return Rect(upper_left, upper_left + s);
    }

    const char *FontBase::end_of_line(const char *str) {
      if (!str) return 0;
      while (1) {
        switch (*str) {
        case '\0' :
        case '\n' :
          return str;

        default :
          str += 1;
        }
      }

      // not reached
      return 0;
    }

    MikroFont::MikroFont(const uint8_t *data, const Size &s, uint16_t offset, char first, char last) :
      data(data),
      first((unsigned char) first),
      last((unsigned char) last)
    {
      this->size = s;
      this->offset = offset;
      height_in_bytes = (size.h + 7)/8;
      glyph_stride = height_in_bytes*size.w + 1; // first byte in each glyph is width
    }

    coord MikroFont::text_width(const char *str, unsigned len) const {
      coord width = 0;

      for (unsigned i=0; i < len; ++i) {
        const glyph_t *g = glyph((unsigned char) str[i]);
        width += g->width;
      }

      if (len > 0) width += GAP*(len - 1);

      return width;
    }

    coord MikroFont::text_height(unsigned line_count) const {
      coord result = line_count*size.h;
      if (line_count > 1) result += (line_count - 1)*GAP;
      return result;
    }

    Size MikroFont::measure(const char *str) const {
      Size s(0,0);
      unsigned line_count = 0;

      while (1) {
        const char *end = end_of_line(str);
        coord w = text_width(str, end - str);
        if (w > s.w) s.w = w;
        s.h += size.h;
        line_count += 1;

        str = end;

        if (*str == '\0') {
          break;
        } else if (*str == '\n') {
          str++;
        }
      }

      if (line_count > 0) s.h += GAP*(line_count - 1);
      return s;
    }

    uint16_t MikroFont::draw_char1(Canvas *c, Point origin, char ch, pixel value) const {
      const glyph_t *g = glyph((unsigned char) ch);

      for (int x=0; x < g->width; ++x) {
        for (int y=0; y < size.h; ++y) {
          if (g->data[x*height_in_bytes + y/8] & (1 << (y & 7))) {
            Point p = Point(origin.x + x, origin.y + y);
            if (c->clip.contains(p)) c->draw_pixel(p, value);
          }
        }
      }

      return g->width + GAP;
    }

    void MikroFont::draw_char(Canvas *c, Point p, char ch, pixel value) const {
      draw_char1(c, p, ch, value);
    }

    void MikroFont::draw_string(Canvas *c, Point p0, const char *str, pixel value) const {
      Point p = p0;

      while (*str) {
        char ch = *str++;

        switch (ch) {
        case '\n' :
          p.x = p0.x;
          p.y += size.h;
          break;

        default :
          p.x += draw_char1(c, p, ch, value);
          break;
        }
      }
    }

    ImageBase::ImageBase(Size s) :
      size(s)
    {
    }

    XBMImage::XBMImage(int width, int height, const unsigned char *data, pixel color) :
      ImageBase(Size(width, height)),
      data(data),
      color(color)
    {
    }

    void XBMImage::draw(Canvas *c, Point p) const {
      const unsigned stride = (size.w + 7)/8;

      for (int y=0; y < size.h; ++y) {
        for (int x=0; x < size.w; ++x) {
          if (data[stride*y + x/8] & (0x1 << (x % 8))) {
            c->draw_pixel(Point(p.x + x, p.y + y), color);
          }
        }
      }
    }

    Canvas::Canvas(Size s) :
      size(s),
      bounds(Point(0, 0), s),
      clip(bounds)
    {
    }

    void Canvas::init() {
    }

    void Canvas::reset() {
      clip = bounds;
    }

    void Canvas::fill_rect(const Rect &r, pixel value) {
      Rect cr = r & clip;

      unsigned int xlim = cr.max.x, ylim = cr.max.y;
      for (unsigned int x=cr.min.x; x < xlim; ++x) {
        for (unsigned int y=cr.min.y; y < ylim; ++y) {
          set_pixel(Point(x, y), value);
        }
      }
    }

    void Canvas::draw_rect(const Rect &r, pixel value) {
      Point bottom_right(r.max.x-1, r.max.y-1);

      draw_line(r.min, Point(bottom_right.x, r.min.y), value);
      draw_line(r.min, Point(r.min.x, bottom_right.y), value);
      draw_line(bottom_right, Point(r.min.x, bottom_right.y), value);
      draw_line(bottom_right, Point(bottom_right.x, r.min.y), value);
    }

    void Canvas::draw_pixel(Point p, pixel value) {
      // check p+1 because the pixel extends down and right
      if (clip.contains(p+1)) set_pixel(p, value);
    }

#if USE_SIMPLIFIED_BRESENHAM
    void Canvas::draw_line(Point p0, Point p1, pixel value) {
      if (!clip.clip(p0, p1)) return;

      if (p0.x == p1.x) { // vertical line
        coord ymax = std::max(p0.y, p1.y);
        for (coord y=std::min(p0.y, p1.y); y <= ymax; ++y) set_pixel(Point(p0.x, y), value);
      } else if (p0.y == p1.y) {
        coord xmax = std::max(p0.x, p1.x);
        for (coord x=std::min(p0.x, p1.x); x <= xmax; ++x) set_pixel(Point(x, p0.y), value);
      } else {
        int dx = std::abs(p1.x-p0.x), dy = std::abs(p1.y-p0.y);
        int sx = (p0.x < p1.x) ? 1 : -1, sy = (p0.y < p1.y) ? 1 : -1;
        int err = dx-dy;

        while (true) {
          set_pixel(p0, value);
          if (p0.x == p1.x && p0.y == p1.y) return;
          int e2 = 2*err;
          if (e2 > -dy) {
            err -= dy;
            p0.x += sx;
          } else if (e2 < dx) {
            err += dx;
            p0.y += sy;
          }
        }
      }
    }
#else
    void Canvas::draw_line(Point p0, Point p1, pixel value) {
      if (!clip.clip(p0, p1)) return;

      if (p0.x == p1.x) { // vertical line
        coord ymax = std::max(p0.y, p1.y);
        for (coord y=std::min(p0.y, p1.y); y <= ymax; ++y) set_pixel(Point(p0.x, y), value);
      } else if (p0.y == p1.y) {
        coord xmax = std::max(p0.x, p1.x);
        for (coord x=std::min(p0.x, p1.x); x <= xmax; ++x) set_pixel(Point(x, p0.y), value);
      } else {
        bool steep = std::abs(p1.y - p0.y) > std::abs(p1.x - p0.x);

        if (steep) {
          std::swap(p0.x, p0.y);
          std::swap(p1.x, p1.y);
        }

        if (p0.x > p1.x) {
          std::swap(p0, p1);
        }

        coord dx = p1.x - p0.x;
        coord dy = std::abs(p1.y - p0.y);

        coord error = dx/2;
        coord y = p0.y;
        coord ystep = (p0.y < p1.y) ? 1 : -1;

        for (coord x=p0.x; x <= p1.x; ++x) {
          if (steep) {
            set_pixel(Point(y, x), value);
          } else {
            set_pixel(Point(x, y), value);
          }

          if ((error -= dy) < 0) {
            y += ystep;
            error += dx;
          }
        }
      }
    }
#endif


    void Canvas::draw_string(Point p, const char *str, const FontBase &f, pixel value) {
      f.draw_string(this, p, str, value);
    }


    void Canvas::draw_image(Point p, ImageBase &img) {
      img.draw(this, p);
    }

    View::View() :
      superview(0),
      subviews(0),
      frame(0, 0, 0, 0)
    {
    }

    void View::add_subview(View &sub) {
      chDbgAssert(sub.superview == 0, __FILE__, __LINE);

      if (subviews) {
        sub.join(subviews);
      } else {
        subviews = &sub;
      }
      sub.superview = this;
    }

    void View::set_frame(const Rect &r) {
      frame = r;
    }

    void View::remove_from_superview() {
      if (superview) {
        if (superview->subviews == this) {
          superview->subviews = empty() ? 0 : begin();
        }
        join(this);
        superview = 0;
      }
    }

    void View::draw_self(Canvas &c) {
    }

    void View::draw_all(Canvas &c) {
      Rect original_clip = c.clip;

      c.clip = frame & original_clip;
      draw_self(c);

      if (subviews) {
        View *sub = subviews;
        do {
          if (sub->frame.intersects(original_clip)) {
            c.clip = sub->frame & original_clip;
            sub->draw_all(c);
          }
          sub = (View *) sub->left;
        } while (sub != subviews);
      }

      c.clip = original_clip;
    }

    int View::count_subviews() const {
      int count=0;
      if (subviews) {
        View *sub = subviews;
        do {
          ++count;
          sub = (View *) sub->left;
        } while (sub != subviews);
      }
      return count;
    }

    void View::remove_all_subviews() {
      if (subviews) {
        View *sub = subviews, *last, *next;
        do {
          last = sub;
          next = (View *) sub->left;

          sub->superview = 0;
          sub->join(sub);
          sub = next;
        } while (next != last);

        subviews = 0;
      }
    }


    Size View::good_size() const {
      return Size(0,0);
    }

    void View::invalidate(const Rect &r) {
      if (superview) superview->invalidate(r & frame);
    }

    void IO::init() {
    }

    void IO::interpret(const uint8_t *buffer) {
      for (;;) {
        const uint8_t *buffer0 = buffer;

        // find next escape sequence
        while (*buffer != ESC) ++buffer;

        if (buffer0 < buffer) {
          unsigned len = buffer - buffer0;
          // send non-escaped bytes as is
          send(buffer0, len);

          // note that buffer has already been incremented to the
          // next escape code
        }

        // assert(*buffer == ESC);
        buffer += 1;

        // handle the escape
        switch (*buffer++) {

        case SLEEP : { // delay
          unsigned msec = (unsigned) *buffer++;
          sleep(msec);
          break;
        }

        case CS : // chip select (arg == 0 when display is *NOT* selected)
          signal_pin(CS_PIN, 0 == *buffer++);
          break;

        case CMDS : // commands follow
          signal_pin(DC_PIN, false);
          break;

        case RESET : // send a reset pulse (arg == pulse width in msec)
          signal_pin(RESET_PIN, true); // active low
          sleep(buffer[0]);
          signal_pin(RESET_PIN, false); // active low
          sleep(buffer[0]);
          signal_pin(RESET_PIN, true); // active low

          buffer += 1;
          break;

        case DATA : {
          unsigned len = *buffer++;
          signal_pin(DC_PIN, true);
          send(buffer, len);
          buffer += len;
          break;
        }

        case PIN :
          signal_pin(buffer[0], buffer[1]);
          buffer += 2;
          break;

        case END :
          return;

        case ESC :
          send(buffer - 1, 1);
          break;

        default :
          for (;;); // lock up!
        }
      }
    }

    Screen::Screen(Canvas &c) :
      is_dirty(false),
      dirty_rect(0, 0, 0, 0),
      root(c)
    {
    }

    void Screen::init() {
      root.init();
      frame = root.bounds;
    }

    void Screen::draw_all() {
      View::draw_all(root);
    }

    void Screen::flush() {
      root.flush(root.bounds);
    }

    void Screen::invalidate(const Rect &r) {
      if (!r.empty()) {
        if (is_dirty) {
          dirty_rect = dirty_rect | (r & root.bounds);
        } else {
          dirty_rect = r;
        }

        is_dirty = true;
      }
    }

    void Screen::update_if_dirty() {
      Rect dirty_rect_copy(dirty_rect);
      bool is_dirty_copy(is_dirty);

      dirty_rect = Rect(0, 0, 0, 0);
      is_dirty = false;

      if (is_dirty_copy) {
        root.clip = dirty_rect_copy;
        draw_all();
        root.flush(dirty_rect_copy);
      }
    }

    SPIDisplay::SPIDisplay(SPIDriver &d, const SPIConfig &c, uint16_t dc, uint16_t reset) :
      spi(d),
      config(c),
      dc_bit(dc),
      reset_bit(reset)
    {
    }

    void SPIDisplay::init() {
      spiStart(&spi, &config);
    }

    void SPIDisplay::reset() {
    }
  };
};
