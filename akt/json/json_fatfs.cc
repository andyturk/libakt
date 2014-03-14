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

  f_close(&file);
}

FATFSWriter::FATFSWriter() {
}

bool FATFSWriter::write_file(const char *filename) {
  FRESULT result;

  if ((result = f_open(&fil, filename, FA_WRITE | FA_CREATE_ALWAYS)) != FR_OK) {
    return false;
  }
}

void FATFSWriter::write(char c) {
  FRESULT result;

  if ((result = f_putc(c, &fil)) != FR_OK) {
    error();
  }
}

void FATFSWriter::write(const char *str) {
  FRESULT result;

  if ((result = f_puts(str, &fil)) != FR_OK) {
    error();
  }
}

void FATFSWriter::write(const char *bytes, unsigned len) {
  FRESULT result;

  if ((result = f_write(&fil, bytes, len)) != FR_OK) {
    error();
  }
}

void FATFSWriter::close() {
  f_close(&fil);
}
