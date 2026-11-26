/**
 * @file rtc_unit_test.h
 * @brief Unit test and timing instrumentation interface for the SAMD21G18A RTC driver.
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

#define RTC_BENCHMARK_SAMPLES_PER_ROUTINE  (100U)
#define RTC_WRITE_PATTERN_COUNT            (4U)

typedef struct {
  uint32_t written_value;
  uint32_t read_while_stopped;
  uint32_t read_after_running;
  bool write_ok;
  bool start_stop_ok;
  bool passed;
} rtc_test_result_t;

typedef struct {
  uint32_t sample_count;
  uint32_t min_ticks;
  uint32_t max_ticks;
  uint64_t total_ticks;
  uint32_t avg_ticks;
} rtc_timing_stats_t;

typedef struct {
  uint32_t timer_overhead_ticks;
  rtc_timing_stats_t write_overall;
  rtc_timing_stats_t write_by_pattern[RTC_WRITE_PATTERN_COUNT];
  rtc_timing_stats_t read_overall;
  rtc_timing_stats_t read_stopped;
  rtc_timing_stats_t read_running;
  bool valid;
} rtc_benchmark_report_t;

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

/**
 * @brief Initializes TC4/TC5 in 32-bit mode at 48 MHz and benchmarks
 *        singlewrite_rtc() and singleread_rtc() over 100 samples each.
 *
 * @param report Output benchmark report with min/max/avg CPU ticks (at 48 MHz).
 * @return true if benchmarking completed successfully, false otherwise.
 */
bool rtc_run_timing_benchmark(rtc_benchmark_report_t *report);

#ifdef __cplusplus
}
#endif

#endif /* RTC_UNIT_TEST_H */
