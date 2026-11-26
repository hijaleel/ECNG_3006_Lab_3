/**
 * @file rtc_unit_test.c
 * @brief Pure C unit test implementation for SAMD21G18A RTC read/write routines.
 *
 * Author: Jaleel Gregory
 * Student ID: 816039976
 * Course: ECNG 3006 - Lab 3
 */

#include "rtc_unit_test.h"
#include "rtc_driver.h"

static void spin_delay(uint32_t cycles) {
  for (volatile uint32_t i = 0U; i < cycles; i++) {
    __NOP();
  }
}

bool rtc_unit_test_step(uint32_t test_value, rtc_test_result_t *result) {
  bool ok = true;
  uint32_t val_stopped = 0U;
  uint32_t val_running = 0U;

  if (result == 0) {
    return false;
  }

  /* Stop RTC so COUNT remains deterministic during write/read verification. */
  ok = stop_rtc();
  result->write_ok = ok && singlewrite_rtc(test_value);
  val_stopped = singleread_rtc();

  /* Start RTC, allow ~10+ 32.768 kHz ticks to elapse, then read again. */
  result->start_stop_ok = ok && start_rtc();
  spin_delay(20000U);
  val_running = singleread_rtc();

  result->written_value = test_value;
  result->read_while_stopped = val_stopped;
  result->read_after_running = val_running;
  result->passed = result->write_ok &&
                   result->start_stop_ok &&
                   (val_stopped == test_value) &&
                   (val_running > test_value);

  return result->passed;
}
