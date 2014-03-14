#include "json_fatfs.h"

using namespace akt::json;

FATFSReader::FATFSReader() :
  reader(token_buffer, sizeof(token_buffer))
{
}

bool FATFSReader::read_file(const char *filename) {
  FRESULT result;

  if ((result = f_open(&fil, filename, FA_READ | FA_OPEN_EXISTING)) != FR_OK) {
    return false;
  }

  reader.reset(this);

  UINT bytes_read;
  while (!reader.is_done()) {
    result = f_read(&fil, file_buffer, sizeof(file_buffer), &bytes_read);

    if (result != FR_OK) {
      error();
      f_close(&fil);
      return false;
    }

    reader.read(file_buffer, bytes_read);
  }

  f_close(&fil);

  return !reader.had_error();
}

FATFSWriter::FATFSWriter() {
}

bool FATFSWriter::write_file(const char *filename) {
  FRESULT result;

  if ((result = f_open(&fil, filename, FA_WRITE | FA_CREATE_ALWAYS)) != FR_OK) {
    return false;
  }

  return true;
}

void FATFSWriter::write(char c) {
  if (f_putc(c, &fil) != 1) {
    error();
  }
}

void FATFSWriter::write(const char *str) {
  if (f_puts(str, &fil) == -1) {
    error();
  }
}

void FATFSWriter::write(const char *bytes, unsigned len) {
  FRESULT result;
  UINT bytes_written;

  result = f_write(&fil, bytes, len, &bytes_written);
  if (result != FR_OK || len != bytes_written) {
    error();
  }
}

void FATFSWriter::close() {
  f_close(&fil);
}
