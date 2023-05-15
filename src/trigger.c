#include "trigger.h"
#include "app_state.h"
#include "imu.h"

/* 1000 msec = 1 sec */
#define SLEEP_TIME_MS 1000

/* The devicetree node identifier for the "led0" alias. */
#define TRIGGER_NODE DT_ALIAS(led0)

static const struct gpio_dt_spec trigger =
    GPIO_DT_SPEC_GET(TRIGGER_NODE, gpios);

static struct k_work_delayable trigger_low_work;
static struct k_work_delayable trigger_high_work;

void set_trigger_low_handler(struct k_work *work) {
  gpio_pin_set_dt(&trigger, 0);
}
void set_trigger_high_handler(struct k_work *work) {
  gpio_pin_set_dt(&trigger, 1);
}
void set_trigger_high_now() { gpio_pin_set_dt(&trigger, 1); }

int init_trigger_dev() {

  if (!device_is_ready(trigger.port)) {

    printk("Error: %s device is not ready", trigger.port->name);
    return -1;
  } else {
    printk(" %s device is ready! \n", trigger.port->name);
    int ret = gpio_pin_configure_dt(&trigger, GPIO_OUTPUT_LOW);
    printk(" gpio configure ret: %d \n", ret);
    k_work_init_delayable(&trigger_low_work, set_trigger_low_handler);
    k_work_init_delayable(&trigger_high_work, set_trigger_high_handler);

    return ret;
  }
}

void schedule_trigger_low(uint32_t delay_us) {
  k_work_reschedule(&trigger_low_work, K_USEC(delay_us));
};

void schedule_trigger_high(uint32_t delay_us) {
  k_work_reschedule(&trigger_high_work, K_USEC(delay_us));
};

void trigger_entry(void) {
  // wait for app to be initialized and exposure to be
  printk("[trigger.c] starting trigger_entry \n");
  while (is_app_initialized() == false || get_exposure() == -1) {
    k_msleep(500);
  }
  uint64_t start = 0;
  uint64_t end = 0;
  struct imu_buff_t *buff1 = get_buffer1_ptr();

  if (buff1 == NULL) {
    while (1) {
      printk("[trigger.c] PANIC: buff1 is null!!!\n");
      k_sleep(K_SECONDS(5));
    }
  }
  while (1) {
    if (get_trigger_state() == TRIGGER_STATE_START) {
      start = k_uptime_ticks();
      set_trigger_high_now();
      schedule_trigger_low(TOGGLE_DELAY_US);
      // schedule imu_read
      uint32_t exposure_ms = get_exposure();
      uint32_t time_delay_us = exposure_ms * 1000 / 2 - IMU_READ_TIME_US / 2;
      buff1->trig_timestamp = k_ticks_to_us_floor64(start);
      buff1->trig_seq++;
      schedule_imu_read_triggered(time_delay_us);
      // TODO: should we already send imu data here?
      k_sleep(K_USEC(TRIGGER_INTERVAL_US - 108));
      // k_msleep(4);
      end = k_ticks_to_us_floor64(k_uptime_ticks());
      // printk("trigger interval: %d us\n", k_cyc_to_us_floor32(end - start));
    } else {
      // wait and check trigger_state after 10ms
      k_msleep(10);
    }
  }
}

// K_THREAD_DEFINE(trigger_thread_id, 2048, trigger_entry, NULL, NULL, NULL,
//                 TRIGGER_THREAD_PRIORITY, 0, 0);
