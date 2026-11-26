/**
 * @file rtc_driver.h
 * @brief Register-level RTC (Absolute Timing Reference) driver for Arduino MKR Zero (SAMD21G18A).
 *
 * Author: Jaleel Gregory
 * Student ID: 816039976
 * Course: ECNG 3006 - Lab 3
 */

#ifndef RTC_DRIVER_H
#define RTC_DRIVER_H

#include <sam.h>
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initializes XOSC32K, GCLK2, PM APBA clock, and RTC Mode 0 (32-bit counter).
 *
 * Explicitly configures clock routing and peripheral settings without assuming
 * hardware defaults are active. Leaves the RTC stopped after resetting COUNT to 0.
 *
 * @return true if initialization succeeded, false on synchronization timeout.
 */
bool init_rtc(void);

/**
 * @brief Starts (enables) the RTC Mode 0 counter.
 *
 * @return true if the RTC started and synchronized, false on timeout.
 */
bool start_rtc(void);

/**
 * @brief Stops (disables) the RTC Mode 0 counter.
 *
 * @return true if the RTC stopped and synchronized, false on timeout.
 */
bool stop_rtc(void);

/**
 * @brief Reads the current 32-bit RTC Mode 0 counter value.
 *
 * Issues a read synchronization request via READREQ before reading COUNT.
 *
 * @return Current 32-bit RTC counter value.
 */
uint32_t singleread_rtc(void);

/**
 * @brief Writes a 32-bit value to the RTC Mode 0 COUNT register.
 *
 * @param value 32-bit counter value to load into RTC->MODE0.COUNT.
 * @return true if the write synchronized successfully, false on timeout.
 */
bool singlewrite_rtc(uint32_t value);

#ifdef __cplusplus
}
#endif

#endif /* RTC_DRIVER_H */
