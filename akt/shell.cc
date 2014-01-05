#include "akt/shell.h"
#include "ch.h"
#include "hal.h"
#include "chprintf.h"

#include <cstring>

using namespace akt;

Ring<ShellCommand>    ShellCommand::commands __attribute__ ((init_priority(200)));
BaseSequentialStream *ShellCommand::tty = 0;
char                  ShellCommand::line[LINE_SIZE];
unsigned int          ShellCommand::line_length;
char                 *ShellCommand::argv[MAX_ARGS];
unsigned int          ShellCommand::argc;
const char           *ShellCommand::prompt = "> ";

ShellCommand::ShellCommand(const char *n) :
  Ring(),
  name(n)
{
  join(commands);
}

ShellCommand *ShellCommand::find_command(const char *name) {
  if (!strcmp(name, "?")) name = "help";

  for (Iterator i=commands.begin(); i != commands.end(); ++i) {
    const char *cmd_name = i->name;

    if (!strcmp(name, cmd_name)) return i;
  }

  return 0;
}

void ShellCommand::read_line() {
  chprintf(tty, prompt);
  line_length = 0;

  while (line_length < LINE_SIZE) {
    char c = (char) chSequentialStreamGet(tty);

    switch (c) {
    case 0x04 : // ctrl-D
      chprintf(tty, "^D");
      line[line_length] = 0;
      return;

    case 0x7f : // delete
    case 0x08 : // ctrl-H
      if (line_length > 0) {
        chSequentialStreamPut(tty, 0x08);
        chSequentialStreamPut(tty, ' ');
        chSequentialStreamPut(tty, 0x08);
        line_length--;
      }
      break;

    case 0x15 : // ctrl-U
      chprintf(tty, "^U\r\n%s", prompt);
      line[line_length=0] = 0;
      break;

    case '\r' :
      chprintf(tty, "\r\n");
      line[line_length] = 0;
      return;

    default :
      if (c < ' ' || c >= 0x80) {
        //chprintf(tty, "<0x%02x>", c);
        continue;
      }

      if (line_length >= LINE_SIZE-1) continue;
      chSequentialStreamPut(tty, c);
      line[line_length++] = c;
    }
  }
}

void ShellCommand::parse_line() {
  unsigned int i=0;
  argc = 0;

  while (i < line_length && argc < MAX_ARGS) {
    // start argument word
    argv[argc++] = &line[i];
    while (i < line_length && line[i] != ' ') i++;
    line[i++] = 0;

    // skip whitespace
    while (i < line_length && line[i] == ' ') i++;
  }
}

msg_t ShellCommand::run_loop(void *) {
  chThdSelf()->p_name = "shell";

  while (tty != 0) {
    read_line();
    parse_line();

    if (argc > 0) {
      ShellCommand *cmd = find_command(argv[0]);

      if (cmd) {
        cmd->exec(argc, argv);
      } else {
        chprintf(tty, "%s ?\r\n", argv[0]);
      }
    }
  }

  return 0;
}

HelpCommand::HelpCommand() :
  ShellCommand("help")
{
}

void HelpCommand::exec(int argc, char *argv[]) {
  if (argc == 0) {
    chprintf(tty, "help     -- list available commands\r\n");
  } else {
    for (Iterator i=commands.begin(); i != commands.end(); ++i) i->exec(0, 0);
  }
}

InfoCommand::InfoCommand() :
  ShellCommand("info")
{
}

void InfoCommand::exec(int argc, char *argv[]) {
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

#if CH_USE_REGISTRY
extern "C" {
  extern uint8_t __main_thread_stack_end__;
};

ThreadsCommand::ThreadsCommand() :
  ShellCommand("threads")
{
}

void ThreadsCommand::exec(int argc, char *argv[]) {
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

extern "C" char __init_array_start;
extern "C" char _etext;
extern "C" char __main_stack_base__;
extern "C" char _data;
extern "C" char _edata;
extern "C" char _bss_start;
extern "C" char _bss_end;

MemoryCommand::MemoryCommand() :
  ShellCommand("memory")
{
}

void MemoryCommand::exec(int argc, char *argv[]) {
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

ResetCommand::ResetCommand() :
  ShellCommand("reset")
{
}

void ResetCommand::exec(int argc, char *argv[]) {
  if (argc == 0) {
    chprintf(tty, "reset    -- force hard reset\r\n");
  } else {
    // Ensure all outstanding memory accesses included
    // buffered write are completed before reset
    asm volatile ("dsb");

    // Keep priority group unchanged
    SCB->AIRCR  = ((0x5FA << SCB_AIRCR_VECTKEY_Pos)      |
                   (SCB->AIRCR & SCB_AIRCR_PRIGROUP_Msk) |
                   SCB_AIRCR_SYSRESETREQ_Msk);
    asm volatile ("dsb");

    // wait until reset
    while(1);
  }
}
