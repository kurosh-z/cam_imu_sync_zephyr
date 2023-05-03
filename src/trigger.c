#include "trigger.h"

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

int init_trigger_dev() {
  printk("starting trigger ....\n");

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

void set_trigger_low(uint32_t delay_us) {
  k_work_reschedule(&trigger_low_work, K_USEC(delay_us));
};

void set_trigger_high(uint32_t delay_us) {
  k_work_reschedule(&trigger_high_work, K_USEC(delay_us));
};

// void trigger_entry(void) {

//   SEGGER_RTT_Init();
//   k_msleep(1000);
//   init_trigger_dev();
// printk("starting trigger ....\n");
// int ret;

// if (!device_is_ready(trigger.port)) {
//   while (1) {
//     printk("Error: %s device is not ready", trigger.port->name);
//     k_msleep(5000);
//   }

// } else {
//   printk(" %s device is ready! \n", trigger.port->name);
// }

// ret = gpio_pin_configure_dt(&trigger, GPIO_OUTPUT_LOW);
// if (ret < 0) {
//   return;
// }

//   while (1) {

//     gpio_pin_set_dt(&trigger, 1);
//     printk("toggling hight \n");
//     // k_msleep(SLEEP_TIME_MS);
//     // gpio_pin_set_dt(&trigger, 0);
//     set_trigger_low(1500);
//     printk("toggling low \n");
//     k_msleep(3000);
//     printk("\n----------------------\n");
//   }
// }

// K_THREAD_DEFINE(trigger_thread_id, 2048, trigger_entry, NULL, NULL, NULL,
// 1, 0,
//                 0);