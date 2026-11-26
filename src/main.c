/**
 * @file main.c
 * @brief PlatformIO Arduino MKR Zero pure-C runner for RTC unit test and timing benchmark.
 *
 * Author: Jaleel Gregory
 * Student ID: 816039976
 * Course: ECNG 3006 - Lab 3
 */

#include <sam.h>
#include "rtc_driver.h"
#include "rtc_unit_test.h"

static const uint32_t kTestPatterns[] = {
  0x00000000UL,
  0x12345678UL,
  0xA5A5A5A5UL,
  0x0000FFFFUL
};

static uint32_t iteration = 0UL;
static volatile uint32_t last_written = 0UL;
static volatile uint32_t last_read_stopped = 0UL;
static volatile uint32_t last_read_running = 0UL;
static volatile bool last_passed = false;
static rtc_benchmark_report_t q3_report;

static void delay_cycles(uint32_t count) {
  for (volatile uint32_t i = 0UL; i < count; i++) {
    __NOP();
  }
}

void setup(void) {
  PM->APBBMASK.reg |= PM_APBBMASK_PORT;
  PORT->Group[1].DIRSET.reg = PORT_PB08;
  PORT->Group[1].OUTCLR.reg = PORT_PB08;

  if (!init_rtc()) {
    while (1) {
      PORT->Group[1].OUTTGL.reg = PORT_PB08;
      delay_cycles(400000UL);
    }
  }

  if (!rtc_run_timing_benchmark(&q3_report)) {
    PORT->Group[1].OUTSET.reg = PORT_PB08;
  }
}

void loop(void) {
  const uint32_t pattern_count = sizeof(kTestPatterns) / sizeof(kTestPatterns[0]);
  uint32_t pattern = kTestPatterns[iteration % pattern_count] + iteration;
  rtc_test_result_t res;

  bool passed = rtc_unit_test_step(pattern, &res);
  last_written = res.written_value;
  last_read_stopped = res.read_while_stopped;
  last_read_running = res.read_after_running;
  last_passed = passed;

  if (passed && q3_report.valid) {
    PORT->Group[1].OUTTGL.reg = PORT_PB08;
  } else {
    PORT->Group[1].OUTSET.reg = PORT_PB08;
  }

  iteration++;
  delay_cycles(2000000UL);
}
