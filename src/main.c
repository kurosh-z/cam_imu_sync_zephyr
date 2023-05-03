#include "imu.h"
#include "trigger.h"
#include "uart_com.h"
#include <SEGGER_RTT.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

#include "app_state.h"

// define imu_read_timer

void main(void) {

  SEGGER_RTT_Init();
  k_msleep(1000);
  printk("[main.c/main] starting imu driver ....\n");

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
  set_app_initialized(true);

  for (;;) {
    k_sleep(K_SECONDS(120));
  }
  // k_sleep(Z_TIMEOUT_US((uint32_t)(2471)));
}