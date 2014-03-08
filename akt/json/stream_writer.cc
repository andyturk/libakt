// -*- Mode:C++ -*-

#include "akt/json/stream_writer.h"

using namespace akt::json;
using namespace std;

StreamWriter::StreamWriter(ostream &out) :
  out(out)
{
}

void StreamWriter::reset() {
  stack.reset();
}

void StreamWriter::object_begin() {
  stack.push(0);
  out << "{";
}

void StreamWriter::object_end() {
  unsigned count;
  stack.pop(count);
  out << "}";
}

void StreamWriter::array_begin() {
  stack.push(0);
  out << "[";
}

void StreamWriter::array_end() {
  unsigned count;
  stack.pop(count);
  out << "]";
}

void StreamWriter::member_name(const char *text) {
  out << text << ':';
}

void StreamWriter::string(const char *text) {
  out << '"' << text << '"';
  if (!stack.empty() && stack.top() > 0) out << ',';
}

void StreamWriter::literal_true() {
  out << "true";
  if (!stack.empty() && stack.top() > 0) out << ',';
}

void StreamWriter::literal_false() {
  out << "false";
  if (!stack.empty() && stack.top() > 0) out << ',';
}

void StreamWriter::literal_null() {
  out << "null";
  if (!stack.empty() && stack.top() > 0) out << ',';
}

void StreamWriter::number(int32_t n) {
  out << n;
  if (!stack.empty() && stack.top() > 0) out << ',';
}

void StreamWriter::number(float n) {
  out << n;
  if (!stack.empty() && stack.top() > 0) out << ',';
}

void StreamWriter::error() {
  out << "ERROR";
}
