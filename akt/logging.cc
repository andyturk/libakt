#include "akt/logging.h"

#include <stdarg.h>
#include <cstdio>

#include "ch.h"
#include "hal.h"
#include "chprintf.h"

using namespace akt;

LogBase::LogBase(const char *name, void *storage, size_t len) :
  fifo((char *) storage, len),
  output_thread(*this, name),
  bytes_lost(0)
{
  chMtxInit(&mutex);
  chCondInit(&not_empty);
}

void LogBase::start() {
  output_thread.start();
}

int LogBase::printf(const char *format, ...) {
  static char buf[256];
  va_list args;
  int count;

  va_start(args, format);
  count = vsnprintf(buf, sizeof(buf), format, args);
  write(buf, (size_t) count);
  va_end(args);

  return count;
}

size_t LogBase::write(const char *bytes, size_t len) {
  size_t written = 0;

  chMtxLock(&mutex);
  if (len > 0) {
    written = fifo.write(bytes, len);

    if (written < len) {
      bytes_lost += len - written;
    }

    if (written > 0) {
      chCondSignal(&not_empty);
    }
  }

  chMtxUnlock();

  return written;
}

void LogBase::flush() {
  chMtxLock(&mutex);

  // there will be at most two contiguous chunks in the ring buffer,
  // so try to write both.
  for (int i=0; i < 2; ++i) {
    size_t available, saved=0;

    available = fifo.contiguous_read_capacity();
    if (available > 0) {
      saved = save(&fifo.peek(0), available);
      fifo.skip(saved);
    }

    if (saved < available) {
      // couldn't write everything
      break;
    }
  }

  fifo.flush();
  chMtxUnlock();
}

bool LogBase::is_logging() const {
  return false;
}

LogBase::OutputThread::OutputThread(LogBase &log, const char *name) :
  akt::ChibiThread<512>(name),
  log(log)
{}

msg_t LogBase::OutputThread::run() {
  for (;;) {
    chMtxLock(&log.mutex);
    msg_t reason = chCondWaitTimeout(&log.not_empty,
                                     MS2ST(IDLE_TIMEOUT_MS));

    // if the wait timed out, the mutex won't be locked, so do that now
    if (reason == RDY_TIMEOUT) chMtxLock(&log.mutex);

    // save whatever we can, regardless of whether someone wrote to
    // the FIFO or there was a timeout
    size_t available, saved = 0;
    available = log.fifo.contiguous_read_capacity();

    if (available > 0) {
      chMtxUnlock();
      saved = log.save(&log.fifo.peek(0), available);
      chMtxLock(&log.mutex);

      log.fifo.skip(saved);
    }

    if (saved < available) {
      // data was lost
    }

    chMtxUnlock();

    if (reason == RDY_TIMEOUT) log.idle();
  }
  return 0;
}

ConsoleLog::ConsoleLog(const char *name, BaseSequentialStream *tty) :
  LogBase(name, buffer, sizeof(buffer)),
  tty(tty)
{}

size_t ConsoleLog::save(const char *bytes, size_t len) {
  return chSequentialStreamWrite(tty, (uint8_t *) bytes, len);
}

bool ConsoleLog::is_logging() const {
  return true;
}

FileLog::FileLog(const char *name, char *buffer, size_t len) :
  LogBase(name, buffer, len),
  buffer(buffer),
  buffer_size(len),
  bytes_since_sync(0)
{
  file.fs = 0;
}

size_t FileLog::log_file_size() const {
  return (size_t) f_tell(&file);
}

bool FileLog::open(const char *path) {
  if (file.fs != 0) return false;

  FRESULT result = f_open(&file, path, (FA_WRITE | FA_OPEN_ALWAYS));

  if (result == FR_OK) {
    result = f_lseek(&file, f_size(&file));

    if (result == FR_OK) {
      return true;
    }
  }

  file.fs = 0; // just to be sure
  return false;
}

size_t FileLog::save(const char *bytes, size_t len) {
  UINT written;

  //palSetPad(GPIOE, GPIOE_LED4);
  FRESULT result = f_write(&file, bytes, len, &written);

  if (result != FR_OK) {
    write_result = result;
  }

  bytes_since_sync += written;

  // should be 512?
  if (bytes_since_sync > 512) {
    result = f_sync(&file);

    if (result != FR_OK) {
      sync_result = result;
    }

    bytes_since_sync = 0;
  }

  return (size_t) written;
}

void FileLog::idle() {
  if (file.fs != 0) {
    FRESULT result = f_sync(&file);

    if (result != FR_OK) {
      sync_result = result;
    }
	bytes_since_sync = 0;
  }
}

void FileLog::flush() {
  LogBase::flush();

  if (file.fs != 0) {
    FRESULT result = f_sync(&file);

    if (result != FR_OK) {
      sync_result = result;
    }
  }

  bytes_since_sync = 0;
}

void FileLog::close() {
  flush();

  if (file.fs != 0) {
    FRESULT result = f_close(&file);
    (void) result;
    file.fs = 0;
  }
}

bool FileLog::is_logging() const {
  return file.fs != 0;
}
