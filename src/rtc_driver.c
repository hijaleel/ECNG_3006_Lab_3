/**
 * @file rtc_driver.c
 * @brief Register-level RTC driver implementation using CMSIS <sam.h> for SAMD21G18A.
 *
 * Author: Jaleel Gregory
 * Student ID: 816039976
 * Course: ECNG 3006 - Lab 3
 */

#include "rtc_driver.h"

#define RTC_SYNC_TIMEOUT_CYCLES   (1000000UL)
#define RTC_COUNT_REG_OFFSET      (0x10U)
#define RTC_GCLK_GENERATOR_ID     (2U)

static bool wait_xosc32k_ready(void) {
  uint32_t timeout = RTC_SYNC_TIMEOUT_CYCLES;
  while (SYSCTRL->PCLKSR.bit.XOSC32KRDY == 0U) {
    if (--timeout == 0U) {
      return false;
    }
  }
  return true;
}

static bool wait_gclk_sync(void) {
  uint32_t timeout = RTC_SYNC_TIMEOUT_CYCLES;
  while (GCLK->STATUS.bit.SYNCBUSY != 0U) {
    if (--timeout == 0U) {
      return false;
    }
  }
  return true;
}

static bool wait_rtc_sync(void) {
  uint32_t timeout = RTC_SYNC_TIMEOUT_CYCLES;
  while (RTC->MODE0.STATUS.bit.SYNCBUSY != 0U) {
    if (--timeout == 0U) {
      return false;
    }
  }
  return true;
}

bool init_rtc(void) {
  uint32_t timeout = RTC_SYNC_TIMEOUT_CYCLES;

  /* 1. Enable APB bus clock for the RTC peripheral. */
  PM->APBAMASK.reg |= PM_APBAMASK_RTC;

  /* 2. Configure external 32.768 kHz crystal (XOSC32K on PA00/PA01).
   *    If already enabled by startup code (e.g., for DFLL48M), preserve ENABLE
   *    and ensure EN32K output is active so GCLK0 is never disrupted.
   */
  if ((SYSCTRL->XOSC32K.reg & SYSCTRL_XOSC32K_ENABLE) == 0U) {
    SYSCTRL->XOSC32K.reg = SYSCTRL_XOSC32K_STARTUP(0x6U) |
                           SYSCTRL_XOSC32K_XTALEN |
                           SYSCTRL_XOSC32K_EN32K;
    SYSCTRL->XOSC32K.bit.ENABLE = 1U;
  } else {
    SYSCTRL->XOSC32K.bit.EN32K = 1U;
  }

  if (!wait_xosc32k_ready()) {
    return false;
  }

  /* 3. Configure GCLK2 (divide by 1) from XOSC32K and route to RTC. */
  GCLK->GENDIV.reg = GCLK_GENDIV_ID(RTC_GCLK_GENERATOR_ID) |
                     GCLK_GENDIV_DIV(1U);
  if (!wait_gclk_sync()) {
    return false;
  }

  GCLK->GENCTRL.reg = GCLK_GENCTRL_ID(RTC_GCLK_GENERATOR_ID) |
                      GCLK_GENCTRL_SRC_XOSC32K |
                      GCLK_GENCTRL_GENEN;
  if (!wait_gclk_sync()) {
    return false;
  }

  GCLK->CLKCTRL.reg = GCLK_CLKCTRL_CLKEN |
                      GCLK_CLKCTRL_GEN_GCLK2 |
                      GCLK_CLKCTRL_ID_RTC;
  if (!wait_gclk_sync()) {
    return false;
  }

  /* 4. Disable RTC before performing a software reset. */
  if (!stop_rtc()) {
    return false;
  }

  RTC->MODE0.CTRL.reg = RTC_MODE0_CTRL_SWRST;
  while (RTC->MODE0.CTRL.bit.SWRST != 0U) {
    if (--timeout == 0U) {
      return false;
    }
  }
  if (!wait_rtc_sync()) {
    return false;
  }

  /* 5. Explicitly set 32-bit Counter Mode (MODE0) with DIV1 prescaler. */
  RTC->MODE0.CTRL.reg = RTC_MODE0_CTRL_MODE_COUNT32 |
                        RTC_MODE0_CTRL_PRESCALER_DIV1;
  if (!wait_rtc_sync()) {
    return false;
  }

  /* 6. Clear all pending interrupt flags and initialize COUNT to 0. */
  RTC->MODE0.INTFLAG.reg = 0xFFU;
  return singlewrite_rtc(0U);
}

bool start_rtc(void) {
  if (!wait_rtc_sync()) {
    return false;
  }
  RTC->MODE0.CTRL.bit.ENABLE = 1U;
  return wait_rtc_sync();
}

bool stop_rtc(void) {
  if (!wait_rtc_sync()) {
    return false;
  }
  RTC->MODE0.CTRL.bit.ENABLE = 0U;
  return wait_rtc_sync();
}

uint32_t singleread_rtc(void) {
  if (!wait_rtc_sync()) {
    return 0U;
  }
  RTC->MODE0.READREQ.reg = RTC_READREQ_RREQ |
                           RTC_READREQ_ADDR(RTC_COUNT_REG_OFFSET);
  if (!wait_rtc_sync()) {
    return 0U;
  }
  return RTC->MODE0.COUNT.reg;
}

bool singlewrite_rtc(uint32_t value) {
  if (!wait_rtc_sync()) {
    return false;
  }
  RTC->MODE0.COUNT.reg = value;
  return wait_rtc_sync();
}
