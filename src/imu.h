#ifndef IMU_H_
#define IMU_H_

#include <SEGGER_RTT.h>

#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <sys/_timeval.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/byteorder.h>
#include <zephyr/sys/printk.h>
#include <zephyr/timing/timing.h>

int init_imu();
typedef uint8_t (*received_data_cb_t)(uint64_t seq, uint32_t delay_us);
int process_mpu9250(bool triggered);
void schedule_imu_read_triggered(uint32_t delay_us);

#endif // IMU_H_