// -*- Mode:C++ -*-

#pragma once

#include "ch.h"
#include "hal.h"

namespace akt {
  class VTimer : VirtualTimer {
    static void vtimer_cb(void *arg) {
      ((VTimer *) arg)->event();
    }

    virtual void event() {}

  public:
    VTimer() {}

    void set (systime_t when) {chVTSet (this, when, &vtimer_cb, this);}
    void setI(systime_t when) {chVTSetI(this, when, &vtimer_cb, this);}
    void reset()              {chVTReset(this);}
    void resetI()             {chVTResetI(this);}
    bool is_setI()            {return chVTIsArmedI(this);}
  };

#if HAL_USE_GPT
  class Timer : protected GPTConfig {
  protected:
    GPTDriver &driver;

    static void timer_cb(GPTDriver *d) {
      ((Timer *) d->config)->event();
    }

  public:
  Timer(GPTDriver &driver, gptfreq_t hz) : driver(driver) {
      frequency = hz;
      callback = &timer_cb;
    }

    virtual void init() {
      gptStart(&driver, this);
    }

    void start(gptcnt_t interval, bool continuous=true) {
      if (continuous) gptStartContinuous(&driver, interval);
      else            gptStartOneShot(&driver, interval);
    }

    void startI(gptcnt_t interval, bool continuous=true) {
      if (continuous) gptStartContinuousI(&driver, interval);
      else            gptStartOneShotI(&driver, interval);
    }

    void stop () {gptStopTimer(&driver);}
    void stopI() {gptStopTimerI(&driver);}

    virtual void event() {}
  };
#endif
};
