#include "imu.h"
#include "trigger.h"
#include "uart_com.h"
#include <SEGGER_RTT.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

#define DELAY_MIN_US 1000
#define DELAY_MAX_US 5000
#define TOGGLE_DELAY_US 600
#define TRIG_SEQ_DEVISOR 25

uint8_t send_trigger_cb(uint64_t seq, uint32_t delayed_us) {
  icState_t icState;
  get_current_icState(&icState);
  if ((icState.trigger_state != TRIGGER_STATE_START) ||
      (icState.exposure == -1)) {
    return 0;
  }

  // we want to trigger in the middle of the exposure time
  uint32_t delay_us = MIN(0, icState.exposure * 1000 / 2 - delayed_us);

  if (seq % TRIG_SEQ_DEVISOR == 0) {
    printk("[main.c/send_trigger_cb] seq: %lld\n", seq);
    set_trigger_high(delay_us);
    set_trigger_low(delay_us + TOGGLE_DELAY_US);
    return 1;
  }
  return 0;
}

void main(void) {

  SEGGER_RTT_Init();
  k_msleep(1000);
  printk("[main.c/main] starting imu driver ....\n");

  uint8_t imu_data[IMU_DATA_LEN] = {0};
  memset(imu_data, 0, sizeof(imu_data));

  int err = 0;
  err = init_imu();
  if (err) {
    while (1) {
      printk("[main.c/main] ERROR: init_imu failed with error %d\n", err);
      k_sleep(K_SECONDS(5));
    }
  }

  err = init_uart_com();
  if (err) {
    while (1) {
      printk("[main.c/main] ERROR: init_uart_com failed with error %d\n", err);
      k_sleep(K_SECONDS(5));
    }
  }
  err = init_trigger_dev();
  if (err) {
    while (1) {
      printk("[main.c/main] ERROR: init_trigger_dev failed with error %d\n",
             err);
      k_sleep(K_SECONDS(5));
    }
  }

  double average = 0;
  uint64_t cnt = 0;
  double delay = DELAY_MIN_US;
  while (1) {
    uint32_t start = k_uptime_get_32();
    err = process_mpu9250(imu_data, &send_trigger_cb);

    if (err) {
      printk("[main.c/main] ERROR: process_mpu9250 failed with error %d\n",
             err);
    } else {

      // printk("[main.c/main] process time %d ms\n ", end - start);

      send_data_uart(imu_data, IMU_DATA_LEN);
    }
    k_msleep(2000);
    // k_sleep(Z_TIMEOUT_US((uint32_t)(2471)));

    uint32_t end = k_uptime_get_32();

    average += (end - start);
    cnt++;
    if (cnt >= 400) {
      // printk("[main.c/main] average process time %f ms\n ", average / cnt);
      double diff = (2000 - average) * 1000;
      // printk("average %f,cnt: %lld , diff: %f \n", average, cnt, diff /
      // 1000); delay = delay + diff; // delay should be changed so that we
      //                                   // get 200 measurements in 1000ms
      // delay = delay < DELAY_MIN_MS ? DELAY_MIN_MS : delay;
      // delay = delay > DELAY_MAX_MS ? DELAY_MAX_MS : delay;

      // printk("new delay: %f, %d\n ", delay, (uint32_t)(delay));
      cnt = 0;
      average = 0;
    }
  }
}