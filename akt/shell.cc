#include "akt/shell.h"
#include "ch.h"
#include "hal.h"
#include "chprintf.h"
#include <string.h>

namespace akt {
  ShellBase::ShellBase(SerialDriver *sd,
                       const command_entry *commands,
                       void *work_area,
                       size_t work_area_size,
                       const char *name,
                       tprio_t prio) :
    ThreadBase(work_area, work_area_size, name, prio),
    sd(sd),
    tty((BaseSequentialStream *) sd),
    prompt("> "),
    command_table(commands == 0 ? default_command_table : commands)
  {
  }

  void ShellBase::readline() {
    chprintf(tty, prompt);
    ll = 0;

    while (ll < LINE_SIZE) {
      char c = (char) chSequentialStreamGet(tty);

      switch (c) {
      case 0x04 : // ctrl-D
        chprintf(tty, "^D");
        line[ll] = 0;
        return;

      case 0x7f : // delete
      case 0x08 : // ctrl-H
        if (ll > 0) {
          chSequentialStreamPut(tty, 0x08);
          chSequentialStreamPut(tty, ' ');
          chSequentialStreamPut(tty, 0x08);
          ll--;
        }
        break;

      case 0x15 : // ctrl-U
        chprintf(tty, "^U\r\n%s", prompt);
        line[ll=0] = 0;
        break;

      case '\r' :
        chprintf(tty, "\r\n");
        line[ll] = 0;
        return;

      default :
        if (c < ' ' || c >= 0x80) {
          //chprintf(tty, "<0x%02x>", c);
          continue;
        }

        if (ll >= LINE_SIZE-1) continue;
        chSequentialStreamPut(tty, c);
        line[ll++] = c;
      }
    }
  }

  void ShellBase::puts(const char *msg) {
    chprintf(tty, "%s\r\n", msg);
  }

  void ShellBase::parseline() {
    unsigned int i=0;
    argc = 0;

    while (i < ll && argc < MAX_ARGS) {
      // start argument word
      argv[argc++] = &line[i];
      while (i < ll && line[i] != ' ') i++;
      line[i++] = 0;

      // skip whitespace
      while (i < ll && line[i] == ' ') i++;
    }
  }

  const ShellBase::command_entry *ShellBase::find(const char *name) {
    if (!strcmp(name, "?")) name = "help";

    for (unsigned i=0; command_table; ++i) {
      if (!command_table[i].name) return 0;
      if (!strcmp(name, command_table[i].name)) return command_table + i;
    }

    __builtin_unreachable();
  }

  const ShellBase::command_entry ShellBase::default_command_table[] = {
    {&help, "help"},
    {&info, "info"},
#if CH_USE_REGISTRY
    {&threads, "threads"},
#endif
    {&memory, "memory"},
    {&reset, "reset"},
    {0, 0}
  };

  void ShellBase::list_commands() {
    for (unsigned i=0; command_table; ++i) {
      const command_entry &ce = command_table[i];
      if (ce.name) {
        ce.fn(this, 0, 0);
        // chprintf(tty, " %s\r\n", ce.name);
      } else {
        break;
      }
    }
  }

  msg_t ShellBase::run() {
    while (1) {
      readline();
      parseline();

      if (argc > 0) {
        const command_entry *c = find(argv[0]);

        if (c) {
          (c->fn)(this, argc, argv);
        } else {
          chprintf(tty, "%s ?\r\n", argv[0]);
        }
      }
    }

    return RDY_OK;
  }

  void ShellBase::help(int argc, char *argv[]) {
    if (argc == 0) {
      chprintf(tty, "help     -- list available commands\r\n");
    } else {
      list_commands();
    }
  }

  void ShellBase::info(int argc, char *argv[]) {
    if (argc == 0) {
      chprintf(tty, "info     -- program/toolchain information\r\n");
    } else {
      chprintf(tty, "Kernel:       %s\r\n", CH_KERNEL_VERSION);
#ifdef CH_COMPILER_NAME
      chprintf(tty, "Compiler:     %s\r\n", CH_COMPILER_NAME);
#endif
      chprintf(tty, "Architecture: %s\r\n", CH_ARCHITECTURE_NAME);
#ifdef CH_CORE_VARIANT_NAME
      chprintf(tty, "Core Variant: %s\r\n", CH_CORE_VARIANT_NAME);
#endif
#ifdef CH_PORT_INFO
      chprintf(tty, "Port Info:    %s\r\n", CH_PORT_INFO);
#endif
#ifdef PLATFORM_NAME
      chprintf(tty, "Platform:     %s\r\n", PLATFORM_NAME);
#endif
#ifdef BOARD_NAME
      chprintf(tty, "Board:        %s\r\n", BOARD_NAME);
#endif
#ifdef __DATE__
#ifdef __TIME__
      chprintf(tty, "Build time:   %s%s%s\r\n", __DATE__, " - ", __TIME__);
#endif
#endif
    }
  }

  extern "C" char __init_array_start;
  extern "C" char _etext;
  extern "C" char __main_stack_base__;
  extern "C" char _data;
  extern "C" char _edata;
  extern "C" char _bss_start;
  extern "C" char _bss_end;


  void ShellBase::memory(int argc, char *argv[]) {
    if (argc == 0) {
      chprintf(tty, "memory   -- display RAM/ROM info\r\n");
    } else {
      size_t n, size;
      n = chHeapStatus(NULL, &size);
#define MEM(fmt,start,end) chprintf(tty, fmt, (start), (end), (1023 + ((end)-(start))) >> 10)
      MEM(".text            : %.8x - %.8x (%dK)\r\n", &__init_array_start, &_etext);
      MEM(".data            : %.8x - %.8x (%dK)\r\n", &_data, &_edata);
      MEM(".bss             : %.8x - %.8x (%dK)\r\n", &_bss_start, &_bss_end);
#undef MEM

      chprintf(tty, "core free memory : %u bytes\r\n", chCoreStatus());
      chprintf(tty, "heap fragments   : %u\r\n", n);
      chprintf(tty, "heap free total  : %u bytes\r\n", size);
    }
  }

#if CH_USE_REGISTRY
  extern "C" {
    extern uint8_t __main_thread_stack_end__;
  };

  void ShellBase::threads(int argc, char *argv[]) {
    static const char *states[] = {THD_STATE_NAMES};

    if (argc == 0) {
      chprintf(tty, "threads  -- list ChibiOS threads\r\n");
    } else {
      chprintf(tty, "%8s %4s %4s %4s %9s %8s %s\r\n",
               "addr", "stku", "prio", "refs", "state", "time", "name");

      for (::Thread *t=chRegFirstThread(); t; t = chRegNextThread(t)) {
#if defined(CH_DBG_HAVE_STKTOP) && defined(CH_DBG_ENABLE_STACK_CHECK)
        uint8_t *stack_top, *untouched;

        if (t->p_stktop == 0) { // must be the main thread
          stack_top = &__main_thread_stack_end__;
        } else {
          stack_top = ((uint8_t *) t->p_stktop) ;
        }

        untouched = (uint8_t *) t->p_stklimit;
        while (*untouched == CH_STACK_FILL_VALUE && untouched < stack_top) ++untouched;

        uint32_t stack_used = untouched - (uint8_t *) t->p_stklimit;
#else
        uint32_t stack_used = 0;
#endif

        chprintf(tty, "%.8lx %.4lx %4lu %4lu %9s %.8lu %s\r\n",
                 (uint32_t)t, (uint32_t) stack_used /*t->p_ctx.r13*/,
                 (uint32_t)t->p_prio, (uint32_t)(t->p_refs - 1),
                 states[t->p_state], (uint32_t)t->p_time,
                 chRegGetThreadName(t));
      }
    }
  }
#endif

  void ShellBase::reset(int argc, char *argv[]) {
    if (argc == 0) {
      chprintf(tty, "reset    -- force hard reset\r\n");
    } else {
      asm volatile ("dsb");                                      /* Ensure all outstanding memory accesses included
                                                                  buffered write are completed before reset */
      SCB->AIRCR  = ((0x5FA << SCB_AIRCR_VECTKEY_Pos)      |
                     (SCB->AIRCR & SCB_AIRCR_PRIGROUP_Msk) |
                     SCB_AIRCR_SYSRESETREQ_Msk);                   /* Keep priority group unchanged */
      asm volatile ("dsb");
      while(1);                                                    /* wait until reset */
    }
  }
};
