// -*- Mode:C++ -*-

#pragma once

#include "akt/ring.h"
#include "akt/assert.h"

#include "ch.h"
#include "hal.h"

#include <algorithm>

namespace akt {
  namespace views {
    typedef int16_t coord;
    typedef uint16_t pixel;

    class FontBase;
    class Canvas;
    class Size;

    enum display_orientation_t {
      NORMAL,
      ROTATE180
    };

    struct Point {
      coord x, y;

      Point();
      Point(int x, int y);

      Point operator+(Point p) const;
      Point operator-(Point p) const;
      Point operator+(Size s)  const;
      Point operator-(Size s)  const;
      Point operator+(int n)   const;
      Point operator-(int n)   const;

      Point &operator +=(const Point &p);
    };

    inline bool operator==(const Point &p1, const Point &p2) {
      return p1.x == p2.x && p1.y == p2.y;
    }

    struct Size {
      coord w, h;

      Size();
      Size(int w, int h);
      inline bool empty() const {return w*h == 0;}
      inline Size operator/(int n) const {return Size(w/n, h/n);}
      inline Size operator*(int n) const {return Size(w*n, h*n);}
      inline Size operator+(int n) const {return Size(w+n, h+n);}
      inline Size operator-(int n) const {return Size(w-n, h-n);}
    };

    inline Point Point::operator+(Point p) const {return Point(x+p.x, y+p.y);}
    inline Point Point::operator-(Point p) const {return Point(x-p.x, y-p.y);}
    inline Point Point::operator+(Size s)  const {return Point(x+s.w, y+s.h);}
    inline Point Point::operator-(Size s)  const {return Point(x-s.w, y-s.h);}
    inline Point Point::operator+(int n)   const {return Point(x+n,   y+n);}
    inline Point Point::operator-(int n)   const {return Point(x-n,   y-n);}


    inline bool operator==(const Size &s1, const Size &s2) {
      return s1.w == s2.w && s1.h == s2.h;
    }

    class Rect {
      enum {
        INSIDE = 0,
        LEFT   = 1 << 0,
        RIGHT  = 1 << 1,
        BOTTOM = 1 << 2,
        TOP    = 1 << 3
      };
  
      int outcode(const Point &p) const;

    public:
      Point min, max;

      Rect();
      Rect(int x, int y, int w, int h);
      Rect(const Point &p0, const Point &p1);
      Rect(const Point &p, const Size &s);

      inline coord width()  const {return max.x - min.x;}
      inline coord height() const {return max.y - min.y;}
      inline Size  size()   const {return Size(width(), height());}
      inline bool  empty()  const {return width()*height() == 0;}
      inline bool  normal() const {return (max.x >= min.x) && (max.y >= min.y);}

      void normalize();
      bool intersects(const Rect &r) const;
      bool contains(const Point &p) const;
      bool contains(const Rect &r) const;

      Rect &operator +=(const Point &p);
      Rect &operator =(const Point &p);
      Rect operator &(const Rect &right) const;
      Rect operator +(Size s);
      Rect operator -(Size s);
      Point center() const;

      bool clip(Point &p0, Point &p1) const;
    };

    bool operator ==(const Rect &left, const Rect &right);
    Rect operator |(const Rect &left, const Rect &right);
    Rect center_size_within(const Rect &r, const Size &s);

#define BYTESWAP(u16) ((((u16) & 0xff) << 8) | ((u16) >> 8))
#define RGB565(r,g,b) (((((uint16_t) r) & 0xf8) << 8) | ((((uint16_t) g) & 0xfc) << 3) | (((uint16_t) b) >> 3))

    namespace rgb565 {
      enum color {
        BLACK        = BYTESWAP(RGB565(0,   0,   0)),
        NAVY         = BYTESWAP(RGB565(0,   0,   128)),
        DARK_GREEN   = BYTESWAP(RGB565(0,   128, 0)),
        DARK_CYAN    = BYTESWAP(RGB565(0,   128, 128)),
        MAROON       = BYTESWAP(RGB565(128, 0,   0)),
        PURPLE       = BYTESWAP(RGB565(128, 0,   128)),
        OLIVE        = BYTESWAP(RGB565(128, 128, 0)),
        LIGHT_GRAY   = BYTESWAP(RGB565(192, 192, 192)),
        DARK_GRAY    = BYTESWAP(RGB565(128, 128, 128)),
        BLUE         = BYTESWAP(RGB565(0,   0,   255)),
        GREEN        = BYTESWAP(RGB565(0,   255, 0)),
        CYAN         = BYTESWAP(RGB565(0,   255, 255)),
        RED          = BYTESWAP(RGB565(255, 0,   0)),
        MAGENTA      = BYTESWAP(RGB565(255, 0,   255)),
        YELLOW       = BYTESWAP(RGB565(255, 255, 0)),
        WHITE        = BYTESWAP(RGB565(255, 255, 255)),
        ORANGE       = BYTESWAP(RGB565(255, 165, 0)),
        GREEN_YELLOW = BYTESWAP(RGB565(173, 255, 47)),
      };
    };

    namespace b_and_w {
      enum color {
        WHITE = 0,
        BLACK = 1
      };
    };

    class FontBase {
    public:
      Size size;
      uint16_t offset;

      static const char *end_of_line(const char *str);
      virtual coord text_width(const char *str, unsigned len) const = 0;
      virtual coord text_height(unsigned line_count) const = 0;
      virtual Size measure(const char *str) const = 0;
      virtual void draw_char(Canvas *c, Point p, char ch, pixel value) const = 0;
      virtual void draw_string(Canvas *c, Point p, const char *str, pixel value) const = 0;
    };

    /*
     * This class handles fonts generated by the MikroElektronica GLCD Font Creator
     * program found at http://www.mikroe.com/glcd-font-creator/
     * 
     * The output of the program is an array of byte values that define the width
     * of each glyph in the font and which bits are set. Black pixels in the editor UI
     * correspond to '1' bits in the data, and '0' means white. The bit data for
     * each glyph is preceeded by a one byte integer specifying the rendered width
     * of that particular glyph. This allows proportionally spaced fonts.
     *
     * Glyphs are rasterized in column major order with the least significant bits
     * at the top. Consider the following 5x7 glyph for the numeral '1':
     *
     *
     *      0 0 1 0 0                 5 bytes of bit data
     *      0 1 1 0 0               ______________________
     *      0 0 1 0 0              v                      v
     *      0 0 1 0 0  ==> 0x04, 0x00, 0x42, 0x7f, 0x40, 0x00
     *      0 0 1 0 0        ^
     *      0 0 1 0 0         \
     *      0 1 1 1 0           1 byte of width        
     */

    class MikroFont : public FontBase {
      enum {
        GAP = 1, // minimum space between rendered glyphs
      };

      const uint8_t *data;
      uint16_t height_in_bytes;
      uint16_t glyph_stride;
      unsigned char first, last;

    public:
      typedef struct {
        uint8_t width;
        uint8_t data[];
      } glyph_t;

      MikroFont(const uint8_t *data, const Size &size, uint16_t offset, char first, char last);

      inline const glyph_t *glyph(unsigned char c) const {
        if (c < first || c > last) c = first;
        return (glyph_t *) (data + (c-first)*glyph_stride);
      }

      coord text_width(const char *str, unsigned len) const;
      virtual coord text_height(unsigned line_count) const;
      virtual Size measure(const char *str) const;
      uint16_t draw_char1(Canvas *c, Point p, char ch, pixel value) const;
      void draw_char(Canvas *c, Point p, char ch, pixel value) const;
      void draw_string(Canvas *c, Point p, const char *str, pixel value) const;
    };

    class Canvas {
    protected:
      virtual void set_pixel(Point p, pixel value) = 0;
      virtual pixel get_pixel(Point p) const = 0;

    public:
      const Size size;
      const Rect bounds;
      Rect clip;

      Canvas(Size s);

      virtual void init();
      virtual void reset();
      virtual void flush(const Rect &r) = 0;
              void flush() { flush(bounds); }
      virtual void fill_rect(const Rect &r, pixel value);
      virtual void draw_rect(const Rect &r, pixel value);
      virtual void draw_pixel(Point p, pixel value);
      virtual void draw_line(Point p0, Point p1, pixel value);
      virtual void draw_string(Point p, const char *str, const FontBase &f, pixel value);
    };

    class View : public Ring<View> {
    public:
      View *superview;
      View *subviews;
      Rect frame;

      View();
      virtual void set_frame(const Rect &r);
      void add_subview(View &v);
      void remove_from_superview();
      virtual void draw_self(Canvas &c);
      void draw_all(Canvas &c);
      int count_subviews() const;
      void remove_all_subviews();
      virtual Size good_size() const;
      virtual void invalidate(const Rect &r);
    };

    struct Style {
      FontBase &font;
      const pixel fg;
      const pixel bg;

      Style(FontBase &f, pixel fg, pixel bg) : font(f), fg(fg), bg(bg) {}
    };

    class StyledView : public View {
    protected:
      Style *style;

    public:
      StyledView() : style(0) {}
      virtual void set_style(Style &s) {style = &s;}
    };

    class Screen : public View {
      Rect dirty;

    public:
      Screen(Canvas &c);
      void init();
      void draw_all();
      void flush();
      virtual void invalidate(const Rect &r);
      void update_if_dirty();
      Canvas &root;
    };

    class IO {
    public:
      enum logical_pins {
        CS_PIN         =  1,
        DC_PIN         =  2,
        RESET_PIN      =  4,
        ENABLE_PIN     =  5,
        MODE_PIN       =  6,
        COM_PIN        =  7,
      };

      enum {
        SLEEP = 0x00,
        CS    = 0x01,
        CMDS  = 0x02,
        RESET = 0x03,
        DATA  = 0x04,
        PIN   = 0x05,
        END   = 0x06,
        ESC   = 0xff
      };

#define VIEW_IO_SLEEP(msec) akt::views::IO::ESC, akt::views::IO::SLEEP, msec
#define VIEW_IO_SELECT      akt::views::IO::ESC, akt::views::IO::CS,    1
#define VIEW_IO_UNSELECT    akt::views::IO::ESC, akt::views::IO::CS,    0
#define VIEW_IO_COMMANDS    akt::views::IO::ESC, akt::views::IO::CMDS
#define VIEW_IO_RESET(msec) akt::views::IO::ESC, akt::views::IO::RESET, msec
#define VIEW_IO_DATA(len)   akt::views::IO::ESC, akt::views::IO::DATA,  len
#define VIEW_IO_ESC         akt::views::IO::ESC, akt::views::IO::ESC
#define VIEW_IO_END         akt::views::IO::ESC, akt::views::IO::END
#define VIEW_IO_PIN(p,lvl)  akt::views::IO::ESC, akt::views::IO::PIN, p, lvl

      virtual void init();
      virtual void signal_pin(uint8_t pin, bool level) = 0;
      virtual void sleep(unsigned msec) = 0;
      virtual void send(const uint8_t *buffer, unsigned len) = 0;
      virtual void interpret(const uint8_t *buffer);
    };

    class SPIDisplay {
    public:
      virtual void init();
      virtual void reset();

    protected:
      SPIDriver &spi;
      SPIConfig config;
      uint16_t dc_bit;
      uint16_t reset_bit;

      inline void begin_spi() {spiSelect(&spi);}
      inline void end_spi() {spiUnselect(&spi);}
      inline void send_commands() {palClearPad(config.ssport, dc_bit);}
      inline void send_data() {palSetPad(config.ssport, dc_bit);}
      virtual void send(uint8_t *data, size_t len) {spiSend(&spi, len, data);}
      virtual void send(uint8_t data) {spiSend(&spi, 1, &data);}

      SPIDisplay(SPIDriver &d, const SPIConfig &c, uint16_t dc, uint16_t reset);
    };
  };
};
