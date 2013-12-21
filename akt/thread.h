#pragma once

#include "akt/assert.h"

#include "ch.h"
#include "hal.h"

#include <string.h>

namespace akt {
  class ThreadBase {
    static msg_t thread_cb(void *arg) {
      return ((ThreadBase *) arg)->run();
    }

  protected:
    struct {
      void *working_area;
      size_t working_area_size;
      const char *name;
      tprio_t priority;
    } args;
    ::Thread *ch_thread;
    virtual msg_t run() {return 0;}

  public:
    ThreadBase(void *wa, size_t swa, const char *name = 0, tprio_t p = NORMALPRIO) {
      args.working_area = wa;
      args.working_area_size = swa;
      args.name = name;
      args.priority = p;
    }

    operator ::Thread *() const {return ch_thread;}
    tprio_t set_priority(tprio_t p) {return chThdSetPriority(p);}
    void terminate() {chThdTerminate(ch_thread);}
    void sleep(systime_t time) {chThdSleep(time);}
    void sleep_until(systime_t time) {chThdSleepUntil(time);}
    void yield() {chThdYield();}
    void exit(msg_t msg) {chThdExit(msg);}
    void exitS(msg_t msg) {chThdExitS(msg);}

#if CH_USE_WAITEXIT
    msg_t wait() {return chThdWait(ch_thread);}
#endif

    void start() {
      chSysLock();

      if (ch_thread == 0) {
        ch_thread = chThdCreateI(args.working_area,
                                 args.working_area_size,
                                 args.priority,
                                 &thread_cb,
                                 this);
        assert(args.working_area == (void *) ch_thread);

        if (args.name) ch_thread->p_name = args.name;
      }

      chSchWakeupS(ch_thread, RDY_OK);
      chSysUnlock();
    }

    static ThreadBase *self() {return ((ThreadBase *) 0);}
    static tprio_t get_priority() {return chThdGetPriority();}
    systime_t get_ticks() {return chThdGetTicks(ch_thread);}
    void *local_storage() const {return (void *) (ch_thread + 1);}
    bool terminated() const {return chThdTerminated(ch_thread);}
    static bool should_terminate() {return chThdShouldTerminate();}

    void resume() { // see chThdResumeI if this causes problems
      chSysLock();
      chThdResumeI(ch_thread);
      chSysUnlock();
    }
    static void sleepS(systime_t time) {chThdSleepS(time);}
    static void sleepSeconds(unsigned int time) {chThdSleep(S2ST(time));}
    static void sleepMillis(unsigned int time) {chThdSleep(MS2ST(time));}
    static void sleepMicros(unsigned int time) {chThdSleep(US2ST(time));}
  };

  template<unsigned Size=128>
  class ChibiThread : public ThreadBase {
    WORKING_AREA(working_area, Size);

  public:
    ChibiThread(const char *name, tprio_t priority = NORMALPRIO);
  };

  template<unsigned Size>
  ChibiThread<Size>::ChibiThread(const char *name, tprio_t priority) :
    ThreadBase(&working_area, sizeof(working_area), name, priority)
  {
  }
};
