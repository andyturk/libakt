// -*- Mode:C++ -*-

#include "akt/json/writer.h"
#include <cstdio>

using namespace akt::json;
using namespace std;

WriterBase::WriterBase() :
  had_error(false)
{
}

void WriterBase::reset() {
  stack.reset();
  had_error = false;
}

void WriterBase::object_begin() {
  if (had_error) return;
  write_comma_if_necessary();

  if (stack.full()) {
    error();
  } else {
    stack.push(0);
    write("{");
  }
}

void WriterBase::object_end() {
  if (had_error) return;

  if (stack.empty()) {
    error();
  } else {
    unsigned count;
    stack.pop(count);
    write("}");
    if (!stack.empty()) stack.top()++;
  }
}

void WriterBase::array_begin() {
  if (had_error) return;
  write_comma_if_necessary();

  if (stack.full()) {
    error();
  } else {
    stack.push(0);
    write("[");
  }
}

void WriterBase::array_end() {
  if (had_error) return;

  if (stack.empty()) {
    error();
  } else {
    unsigned count;
    stack.pop(count);
    write("]");
    if (!stack.empty()) stack.top()++;
  }
}

void WriterBase::member_name(const char *text) {
  if (had_error) return;
  write_comma_if_necessary();

  write('"');
  write_quoted(text);
  write("\" : ");

  skip_next_comma = true;
}

void WriterBase::string(const char *text) {
  if (had_error) return;
  write_comma_if_necessary();

  write('"');
  write_quoted(text);
  write('"');

  if (!stack.empty()) stack.top()++;
}

void WriterBase::write_quoted(const char *str) {
  // this needs to change
  write(str);
}

void WriterBase::literal_true() {
  if (had_error) return;
  write_comma_if_necessary();

  write("true");

  if (!stack.empty()) stack.top()++;
}

void WriterBase::literal_false() {
  if (had_error) return;
  write_comma_if_necessary();

  write("false");

  if (!stack.empty()) stack.top()++;
}

void WriterBase::literal_null() {
  if (had_error) return;
  write_comma_if_necessary();

  write("null");

  if (!stack.empty()) stack.top()++;
}

void WriterBase::number(int32_t n) {
  if (had_error) return;
  write_comma_if_necessary();

  char buffer[20];

  snprintf(buffer, sizeof(buffer), "%d", n);
  write(buffer);

  if (!stack.empty()) stack.top()++;
}

void WriterBase::number(float n) {
  if (had_error) return;
  write_comma_if_necessary();

  char buffer[20];

  snprintf(buffer, sizeof(buffer), "%g", n);
  write(buffer);

  if (!stack.empty()) stack.top()++;
}

void WriterBase::error() {
  if (had_error) return;

  write("\nERROR");
  had_error = true;
}

void WriterBase::write_comma_if_necessary() {
  if (!skip_next_comma && !stack.empty() && stack.top() > 0) write(", ");
  skip_next_comma = false;
}
