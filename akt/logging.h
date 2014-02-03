#pragma once

#include "akt/ringbuffer.h"
#include "akt/thread.h"

#include "ch.h"
#include "hal.h"
#include "ff.h"

#include <stdarg.h>

namespace akt {
  /**
   * The LogBase class provides basic thread-safe functionality for logging.
   * Each instance maintains a first-in, first-out (FIFO) buffer of logged data.
   * Callers wishing to record log information add data to this FIFO in a
   * thread safe fashion. An internal thread reads data from the FIFO and writes
   * it out somewhere else (e.g., to a console or file).
   *
   * LogBase instances use a monitor pattern to ensure that writes to the FIFO
   * are serialized and atomic.
   * See: http://en.wikipedia.org/wiki/Monitor_(synchronization))
   *
   * Subclassers must implement the save() method which is responsible for
   * reading accumulated log data from the FIFO. Subclassers may also implement
   * the idle() method, which is called periodically when there is no other
   * activity. Both save() and idle() are called in the context of the
   * OutputThread but while the mutex is unlocked.
   */
  class LogBase {
  protected:
    Mutex mutex;
    CondVar not_empty;
    akt::RingBuffer<char> fifo;
            
    // This helper thread pulls data out of the fifo in the background
    class OutputThread : public akt::ChibiThread<512> {
      LogBase &log;
      enum {IDLE_TIMEOUT_MS = 500};

    public:
      OutputThread(LogBase &log, const char *name);
      virtual msg_t run() override;
    } output_thread;

    // These methods are called by the OutputThread outside of the mutex lock
    virtual size_t save(const char *bytes, size_t len) = 0;
    virtual void idle() {}

  public:
    LogBase(const char *name, void *storage, size_t len);

    void start();
    int printf(const char *format, ...);
    size_t write(const char *bytes, size_t len);
    virtual void flush();
    virtual bool is_logging() const;

    size_t bytes_lost;
  };

  /**
   * ConsoleLog instances provide a thread-safe logging mechanism that writes
   * messages out to a ChibiOS serial port or other kind of sequential stream.
   */
  class ConsoleLog : public LogBase {
    BaseSequentialStream *tty;
    char buffer[32];

  protected:
    virtual size_t save(const char *bytes, size_t len) override;

  public:
    ConsoleLog(const char *name, BaseSequentialStream *tty);
    virtual bool is_logging() const override;
  };

  /**
   * FileLog instances provide a thread-safe logging mechanism that writes
   * messages out to a text file on a FATFS filesystem.
   */
  class FileLog : public LogBase {
    FIL file;
    char *const buffer;
    const size_t buffer_size;
    size_t bytes_since_sync;
    FRESULT sync_result, write_result;

  protected:
    virtual size_t save(const char *bytes, size_t len) override;
    virtual void idle() override;

  public:
    // name will be the name of the background thread, not the name of the file
    FileLog(const char *name, char *buffer, size_t size);

    size_t log_file_size() const;
    bool open(const char *path);
    virtual void flush() override;
    void close();
    virtual bool is_logging() const override;
  };
};
