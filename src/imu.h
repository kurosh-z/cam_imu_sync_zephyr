#ifndef IMU_H_
#define IMU_H_

#include <SEGGER_RTT.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/byteorder.h>
#include <zephyr/sys/printk.h>
#define IMU_DATA_LEN 85
// #define IMU_PRIORITY 1
// #define IMU_STACK_SIZE 1024 * 4
// #define IMU_FORMAT_OUT "%llu,%f,%f,%f,%f,%f,%f,%f,%f,%f\n"

int init_imu();
typedef uint8_t (*received_data_cb_t)(uint64_t seq, uint32_t delay_us);
int process_mpu9250(uint8_t *imu_data, received_data_cb_t cb);

#endif // IMU_H_