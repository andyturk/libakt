// -*- Mode:C++ -*-

#pragma once

#include "akt/ring.h"

#include "ch.h"
#include "hal.h"

namespace akt {
  /**
   * @class ShellCommand
   * @brief Parses lines read from a serial console and executes commands
   *
   * This class implements a simple command line shell. Input lines are parsed
   * into words separated by whitespace. The first word on each line names the
   * command to be executed.
   *
   * Each command is represented by a single instance of a ShellCommand
   * subclass, usually declared statically. ShellCommand's constructor
   * automatically adds each instance to a global list of known commands.
   *
   * Applications using a shell should first initialize the tty variable
   * to point at a valid BaseSequentialStream. Then the run_loop() method
   * should be called in a dedicated thread.
   */
  class ShellCommand : public Ring<ShellCommand> {
  public:
    /**
     * @brief Constructs an instance and adds it to the list of all commands
     * @param[in] string which will invoke this command from the tty
     */
    ShellCommand(const char *name);

    /**
     * @brief Executes a specific command with pre-parsed args
     * @param[in] argc the number of arguments (>= 0)
     * @param[in] argv[] an array of null terminated argument strings
     * 
     * When argc == 0, the command should print out help information
     * to the tty
     */
    virtual void exec(int argc, char *argv[]) = 0;

    /**
     * @brief Repeatedly reads a line from the console and executes the command
     * @details Under normal circumstances, this function never returns
     */
    static msg_t run_loop(void *);

    static BaseSequentialStream *tty;   /// serial console used by shell commands
    static const char *prompt;          /// string printed out before a line is read from the tty

  protected:
    /**
     * @brief Tries to locate a Command instance with the specified name
     * @param[in] name null terminated name
     * @returns a pointer to the command with a matching name
     * @returns 0 otherwise
     */
    static ShellCommand *find_command(const char *name);

    enum {
      LINE_SIZE = 80,                   /// size of internal line buffer
      MAX_ARGS = 5                      /// max number of words parsed
    };

    const char *name;                   /// string used to invoke this command from the tty
    static void read_line();            /// reads a single line of text from the tty with minimal editing
    static void parse_line();           /// parses a line of text into words separated by whitespace

    static Ring<ShellCommand> commands; /// linked list of all known commands

    static char line[LINE_SIZE];        /// line buffer for tty input
    static unsigned int line_length;    /// number of chars in the line buffer
    static char *argv[MAX_ARGS];        /// parsed words
    static unsigned int argc;           /// number of parsed words
  };

  /**
   * @brief Prints out a list of known commands
   * 
   * Example output:
   * > help                                        
   * reset    -- force hard reset                  
   * memory   -- display RAM/ROM info              
   * threads  -- list ChibiOS threads              
   * info     -- program/toolchain information     
   * help     -- list available commands           
   */
  class HelpCommand : public ShellCommand {
  public:
    HelpCommand();
    void exec(int argc, char *argv[]) override;
  };

  /**
   * @brief Prints out build info
   *
   * Example output:
   * > info                                                                          
   * Kernel:       2.7.0unstable                                                     
   * Compiler:     GCC 4.7.3 20130312 (release) [ARM/embedded-4_7-branch revision 19]
   * Architecture: ARMv7-ME                                                          
   * Core Variant: Cortex-M4                                                         
   * Port Info:    Advanced kernel mode                                              
   * Platform:     STM32F407/F417 High Performance with DSP and FPU                  
   * Board:        Hannibal v2 2013-11-22                                            
   * Build time:   Jan  5 2014 - 11:05:02                                            
   */
  class InfoCommand : public ShellCommand {
  public:
    InfoCommand();
    void exec(int argc, char *argv[]) override;
  };

#if CH_USE_REGISTRY
  /**
   * @brief Lists ChibiOS/RT threads
   *
   * Example output:
   * 
   * > threads                                               
   *     addr stku prio refs     state     time name         
   * 200012B8 0000   64    1  SLEEPING 00000342 main         
   * 200011F8 0000    1    1     READY 00006109 idle         
   * 20001718 0000    2    1     READY 00000000 usb_lld_pump 
   * 20002200 0000   64    1   CURRENT 00000001 shell        
   * 20002048 0000   64    1  SLEEPING 00000000 buttons      
   */
  class ThreadsCommand : public ShellCommand {
  public:
    ThreadsCommand();
    void exec(int argc, char *argv[]) override;
  };
#endif

  /**
   * @brief Prints out memory allocation/usage
   *
   * Example output:
   * 
   * > memory                                         
   * .text            : 080001AC - 0802A758 (170K)    
   * .data            : 20000800 - 200011F4 (3K)      
   * .bss             : 200011F8 - 200032A0 (9K)      
   * core free memory : 101728 bytes                  
   * heap fragments   : 0                             
   * heap free total  : 0 bytes                       
   */
  class MemoryCommand : public ShellCommand {
  public:
    MemoryCommand();
    void exec(int argc, char *argv[]) override;
  };

  /**
   * @brief Forces a software reset of the system
   *
   */
  class ResetCommand : public ShellCommand {
  public:
    ResetCommand();
    void exec(int argc, char *argv[]) override;
  };

  /**
    @brief Creates all the built-in shell commands with one declaration
  */
  struct BuiltInShellCommands {
    HelpCommand help_command;
    InfoCommand info_command;
#if CH_USE_REGISTRY
    ThreadsCommand threads_command;
#endif
    MemoryCommand memory_command;
    ResetCommand reset_command;
  };
};
