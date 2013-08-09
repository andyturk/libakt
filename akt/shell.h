// -*- Mode:C++ -*-

#pragma once

#include "akt/thread.h"

namespace akt {
  class ShellBase : public ThreadBase {
  protected:
    enum {
      LINE_SIZE = 80,
      MAX_ARGS = 5
    };

    typedef struct {
      void (*fn)(ShellBase *shell, int argc, char *argv[]);
      const char *name;
    } command_entry;

    SerialDriver *sd;
    BaseSequentialStream *tty;

    char line[LINE_SIZE];
    unsigned int ll; // line length
    char *argv[MAX_ARGS];
    unsigned int argc;
    const char *prompt;

    void readline();
    void parseline();

    void puts(const char *msg);
    void list_commands();

    const command_entry *find(const char *name);
    const command_entry *command_table;
    static const command_entry default_command_table[];

    void info(int argc, char *argv[]);
    void help(int argc, char *argv[]);
    void threads(int argc, char *argv[]);
    void memory(int argc, char *argv[]);
    void reset(int argc, char *argv[]);

    msg_t run();

  public:
    ShellBase(SerialDriver *sd, const command_entry *commands,
              void *wa, size_t wa_size,
              const char *name, tprio_t prio);
  };
};
