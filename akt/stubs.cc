#include <stdio.h>

#include "ch.h"
#include "hal.h"

void *operator new[](unsigned size) {
  port_halt();
  // not reached
  return (void *) 0xffffffff;
}

extern "C" {
  void __cxa_pure_virtual() {
    chDbgAssert("unimplemented pure virtual", __FILE__, __LINE__);
  }
  void *__dso_handle = NULL;

  void _exit(int status) {
    (void) status;
    chSysHalt();
    for(;;);
  }

  pid_t _getpid(void) {
    return 1;
  }

  void _kill(pid_t id) {
    (void) id;
  }

  struct stacked_registers {
    unsigned int r0;
    unsigned int r1;
    unsigned int r2;
    unsigned int r3;
    unsigned int r12;
    unsigned int lr;
    unsigned int pc;
    unsigned int psr;
  };

  struct fault_state {
    unsigned long BFAR;
    unsigned long CFSR;
    unsigned long HFSR;
    unsigned long DFSR;
    unsigned long AFSR;
  };

  void HardFaultVector_c(stacked_registers *regs) {
    fault_state f;

    (void) regs;
    f.BFAR = *((volatile unsigned long *)(0xE000ED38));
    f.CFSR = *((volatile unsigned long *)(0xE000ED28));
    f.HFSR = *((volatile unsigned long *)(0xE000ED2C));
    f.DFSR = *((volatile unsigned long *)(0xE000ED30));
    f.AFSR = *((volatile unsigned long *)(0xE000ED3C));
    while (1);
    (void) f;
  }

#if !defined(STM32F0XX)
  void HardFaultVector(void) __attribute__((naked));
  void HardFaultVector(void) {
    __asm ("tst lr, #4");
    __asm ("ite eq");
    __asm ("mrseq r0, msp");
    __asm ("mrsne r0, psp");
    __asm ("b HardFaultVector_c");
  }
#endif
};
