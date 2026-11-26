/**
 * @file rtc_unit_test.h
 * @brief Unit test interface for the SAMD21G18A RTC driver routines.
 *
 * Author: Jaleel Gregory
 * Student ID: 816039976
 * Course: ECNG 3006 - Lab 3
 */

#ifndef RTC_UNIT_TEST_H
#define RTC_UNIT_TEST_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  uint32_t written_value;
  uint32_t read_while_stopped;
  uint32_t read_after_running;
  bool write_ok;
  bool start_stop_ok;
  bool passed;
} rtc_test_result_t;

/**
 * @brief Executes one read/write/run unit-test cycle against the RTC peripheral.
 *
 * Stops the RTC, writes test_value, reads it back to confirm exact register
 * integrity, starts the RTC for a brief delay, and reads again to verify counting.
 *
 * @param test_value 32-bit test pattern to write into RTC COUNT.
 * @param result Pointer to output struct populated with test telemetry.
 * @return true if the test cycle passed, false otherwise.
 */
bool rtc_unit_test_step(uint32_t test_value, rtc_test_result_t *result);

#ifdef __cplusplus
}
#endif

#endif /* RTC_UNIT_TEST_H */
