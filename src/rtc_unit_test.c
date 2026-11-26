/**
 * @file rtc_unit_test.c
 * @brief Pure C unit test and TC4 48 MHz execution-time instrumentation for SAMD21G18A RTC.
 *
 * Author: Jaleel Gregory
 * Student ID: 816039976
 * Course: ECNG 3006 - Lab 3
 */

#include "rtc_unit_test.h"
#include "rtc_driver.h"

#define TC_SYNC_TIMEOUT_CYCLES  (1000000UL)
#define TC_COUNT_REG_OFFSET     (0x10U)

static const uint32_t kBenchPatterns[RTC_WRITE_PATTERN_COUNT] = {
  0x00000000UL,
  0xFFFFFFFFUL,
  0xA5A5A5A5UL,
  0x12345678UL
};

static void spin_delay(uint32_t cycles) {
  for (volatile uint32_t i = 0U; i < cycles; i++) {
    __NOP();
  }
}

static bool wait_tc4_sync(void) {
  uint32_t timeout = TC_SYNC_TIMEOUT_CYCLES;
  while (TC4->COUNT32.STATUS.bit.SYNCBUSY != 0U) {
    if (--timeout == 0U) {
      return false;
    }
  }
  return true;
}

static bool init_tc4_timer_48mhz(void) {
  uint32_t timeout = TC_SYNC_TIMEOUT_CYCLES;

  PM->APBCMASK.reg |= PM_APBCMASK_TC4 | PM_APBCMASK_TC5;

  GCLK->CLKCTRL.reg = GCLK_CLKCTRL_CLKEN |
                      GCLK_CLKCTRL_GEN_GCLK0 |
                      GCLK_CLKCTRL_ID_TC4_TC5;
  while (GCLK->STATUS.bit.SYNCBUSY != 0U) {
    if (--timeout == 0U) {
      return false;
    }
  }

  TC4->COUNT32.CTRLA.reg = TC_CTRLA_SWRST;
  if (!wait_tc4_sync()) {
    return false;
  }
  timeout = TC_SYNC_TIMEOUT_CYCLES;
  while (TC4->COUNT32.CTRLA.bit.SWRST != 0U) {
    if (--timeout == 0U) {
      return false;
    }
  }

  TC4->COUNT32.CTRLA.reg = TC_CTRLA_MODE_COUNT32 | TC_CTRLA_PRESCALER_DIV1;
  if (!wait_tc4_sync()) {
    return false;
  }

  TC4->COUNT32.COUNT.reg = 0UL;
  if (!wait_tc4_sync()) {
    return false;
  }

  TC4->COUNT32.CTRLA.bit.ENABLE = 1U;
  if (!wait_tc4_sync()) {
    return false;
  }

  TC4->COUNT32.READREQ.reg = TC_READREQ_RCONT | TC_READREQ_ADDR(TC_COUNT_REG_OFFSET);
  return wait_tc4_sync();
}

static void stats_init(rtc_timing_stats_t *stats) {
  stats->sample_count = 0UL;
  stats->min_ticks = 0xFFFFFFFFUL;
  stats->max_ticks = 0UL;
  stats->total_ticks = 0ULL;
  stats->avg_ticks = 0UL;
}

static void stats_record(rtc_timing_stats_t *stats, uint32_t ticks) {
  stats->sample_count++;
  if (ticks < stats->min_ticks) {
    stats->min_ticks = ticks;
  }
  if (ticks > stats->max_ticks) {
    stats->max_ticks = ticks;
  }
  stats->total_ticks += (uint64_t)ticks;
}

static void stats_finalize(rtc_timing_stats_t *stats) {
  if (stats->sample_count > 0UL) {
    stats->avg_ticks = (uint32_t)(stats->total_ticks / (uint64_t)stats->sample_count);
  } else {
    stats->min_ticks = 0UL;
  }
}

static uint32_t measure_timer_overhead(void) {
  uint32_t t0 = TC4->COUNT32.COUNT.reg;
  uint32_t t1 = TC4->COUNT32.COUNT.reg;
  return (t1 - t0);
}

bool rtc_unit_test_step(uint32_t test_value, rtc_test_result_t *result) {
  bool ok = true;
  uint32_t val_stopped = 0U;
  uint32_t val_running = 0U;

  if (result == 0) {
    return false;
  }

  ok = stop_rtc();
  result->write_ok = ok && singlewrite_rtc(test_value);
  val_stopped = singleread_rtc();

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

bool rtc_run_timing_benchmark(rtc_benchmark_report_t *report) {
  uint32_t overhead = 0UL;

  if (report == 0) {
    return false;
  }

  report->valid = false;
  stats_init(&report->write_overall);
  for (uint32_t p = 0U; p < RTC_WRITE_PATTERN_COUNT; p++) {
    stats_init(&report->write_by_pattern[p]);
  }
  stats_init(&report->read_overall);
  stats_init(&report->read_stopped);
  stats_init(&report->read_running);

  if (!init_tc4_timer_48mhz()) {
    return false;
  }

  (void)TC4->COUNT32.COUNT.reg;
  overhead = measure_timer_overhead();
  report->timer_overhead_ticks = overhead;

  /* 1. Benchmark singlewrite_rtc() across 100 samples (25 per 32-bit pattern). */
  if (!stop_rtc()) {
    return false;
  }
  for (uint32_t i = 0U; i < RTC_BENCHMARK_SAMPLES_PER_ROUTINE; i++) {
    uint32_t pat_idx = i % RTC_WRITE_PATTERN_COUNT;
    uint32_t pattern = kBenchPatterns[pat_idx];
    uint32_t t_start = TC4->COUNT32.COUNT.reg;
    bool ok = singlewrite_rtc(pattern);
    uint32_t t_end = TC4->COUNT32.COUNT.reg;
    uint32_t elapsed = (t_end - t_start);
    elapsed = (elapsed > overhead) ? (elapsed - overhead) : 0UL;

    if (!ok) {
      return false;
    }
    stats_record(&report->write_overall, elapsed);
    stats_record(&report->write_by_pattern[pat_idx], elapsed);
    spin_delay((i % 7U) * 150U + 50U);
  }

  /* 2. Benchmark singleread_rtc(): 50 stopped (fixed return) + 50 running (changing return). */
  if (!singlewrite_rtc(0x12345678UL)) {
    return false;
  }
  for (uint32_t i = 0U; i < (RTC_BENCHMARK_SAMPLES_PER_ROUTINE / 2U); i++) {
    uint32_t t_start = TC4->COUNT32.COUNT.reg;
    volatile uint32_t val = singleread_rtc();
    uint32_t t_end = TC4->COUNT32.COUNT.reg;
    uint32_t elapsed = (t_end - t_start);
    elapsed = (elapsed > overhead) ? (elapsed - overhead) : 0UL;
    (void)val;

    stats_record(&report->read_overall, elapsed);
    stats_record(&report->read_stopped, elapsed);
    spin_delay((i % 7U) * 150U + 50U);
  }

  if (!start_rtc()) {
    return false;
  }
  for (uint32_t i = 0U; i < (RTC_BENCHMARK_SAMPLES_PER_ROUTINE / 2U); i++) {
    uint32_t t_start = TC4->COUNT32.COUNT.reg;
    volatile uint32_t val = singleread_rtc();
    uint32_t t_end = TC4->COUNT32.COUNT.reg;
    uint32_t elapsed = (t_end - t_start);
    elapsed = (elapsed > overhead) ? (elapsed - overhead) : 0UL;
    (void)val;

    stats_record(&report->read_overall, elapsed);
    stats_record(&report->read_running, elapsed);
    spin_delay(2000U + (i % 7U) * 150U);
  }

  stats_finalize(&report->write_overall);
  for (uint32_t p = 0U; p < RTC_WRITE_PATTERN_COUNT; p++) {
    stats_finalize(&report->write_by_pattern[p]);
  }
  stats_finalize(&report->read_overall);
  stats_finalize(&report->read_stopped);
  stats_finalize(&report->read_running);

  report->valid = true;
  return true;
}
