
#ifndef TRIGGER_H_
#define TRIGGER_H_

#include <SEGGER_RTT.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

int init_trigger_dev();
void set_trigger_low(uint32_t delay_us);
void set_trigger_high(uint32_t delay_us);
#endif