#ifndef APP_STATE_H_
#define APP_STATE_H_

#include <SEGGER_RTT.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/printk.h>

#define TOGGLE_DELAY_US 600
#define TRIG_SEQ_DEVISOR 25
#define IMU_READ_TIME_US 2380
#define IMU_DATA_MAX_LEN 128
#define TRIGGER_INTERVAL_US 40 * 1000

enum TriggerState {
  TRIGGER_STATE_STOP,
  TRIGGER_STATE_START,
  TRIGGER_STATE_PAUSE
};
enum IMU_Buff_State { IMU_BUFF_STATE_WAITING, IMU_BUFF_STATE_SENT };
struct imu_buff_t {
  uint8_t data[IMU_DATA_MAX_LEN];
  uint8_t state;
  size_t len;
};
typedef struct app_state_t {
  bool initialized;
  uint8_t trigger_state;
  uint32_t exposure;
  uint64_t imu_seq;
  uint64_t trigger_seq;
  struct imu_buff_t imu_buff1; // has higher priority to send in comp. to
                               // imu_buff2 to use for sending when trigger sent
  struct imu_buff_t imu_buff2; // lower priority to be used for imu_data

} app_state_t;
void get_current_state(app_state_t *state);
void set_current_state(app_state_t *state);
void set_buffer1(uint8_t *imu_data, size_t len);
void set_buffer1_state(uint8_t state);
void set_buffer2(uint8_t *imu_data, size_t len);
void set_buffer2_state(uint8_t state);
void set_trigger_state(uint8_t state);
void set_exposure(uint32_t exposure);
uint32_t get_exposure();
uint8_t get_trigger_state();

uint64_t increase_imu_seq();
uint64_t increase_trigger_seq();
uint64_t get_imu_seq();
uint64_t get_trigger_seq();
bool is_app_initialized();
void set_app_initialized(bool state);

#endif /* APP_STATE_H_ */