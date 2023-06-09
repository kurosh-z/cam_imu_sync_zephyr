#ifndef APP_STATE_H_
#define APP_STATE_H_

#include <SEGGER_RTT.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/printk.h>

#define TOGGLE_DELAY_US 300
#define TRIG_SEQ_DEVISOR 25
#define IMU_READ_TIME_US                                                       \
  2380 // measured experimentally and took an average of 100 readings
#define IMU_DATA_MAX_LEN 128
#define TRIGGER_INTERVAL_US 40 * 1000

#define UART_THREAD_PRIORITY 4
#define IMU_THREAD_PRIORITY 3
#define TRIGGER_THREAD_PRIORITY 2

// define commands
// define trigger commands:
#define CMD_SET_EXPOSURE "CMD_SET_EXPOSURE" // uint16 value
#define CMD_GET_EXPOSURE "CMD_GET_EXPOSURE" // uint16 value
#define CMD_SET_TRIGGER "CMD_SET_TRIGGER"   // uint8 value
#define CMD_GET_TRIGGER "CMD_GET_TRIGGER"   // uint8 value
#define CMD_GET_ERROR "CMD_GET_ERROR"       // uint8 value

// define response headers
#define RESP_SET_EXPOSURE "RESP_SET_EXPOSURE" // uint16 value
#define RESP_GET_EXPOSURE "RESP_GET_EXPOSURE" // uint16 value
#define RESP_SET_TRIGGER "RESP_SET_TRIGGER"   // uint8 value
#define RESP_GET_TRIGGER "RESP_GET_TRIGGER"   // uint8 value
#define RESP_GET_ERROR "RESP_GET_ERROR"       // uint8 value

enum ErrorCodes {
  NO_ERROR,
  UART_ERROR,
  IMU_ERROR,
  TRIGGER_ERROR,
  UNKNOWN_ERROR
};

enum TriggerState {
  TRIGGER_STATE_STOP = 0,
  TRIGGER_STATE_START = 1,
};

enum IMU_Buff_State { IMU_BUFF_STATE_WAITING, IMU_BUFF_STATE_SENT };
struct imu_buff_t {
  uint8_t data[IMU_DATA_MAX_LEN];
  uint8_t state;
  size_t len;
  uint32_t timestamp;
  uint64_t trig_seq;
  uint64_t trig_timestamp;
};
typedef struct app_state_t {
  bool initialized;
  uint8_t error;
  uint8_t trigger_state;
  uint64_t next_scheduled_imu_trig_read;
  uint32_t exposure; // in milliseconds [ms]
  uint64_t imu_seq;
  uint64_t trigger_seq;
  struct imu_buff_t imu_buff1; // has higher priority to send in comp. to
                               // imu_buff2 to use for sending when trigger sent
  struct imu_buff_t imu_buff2; // lower priority to be used for imu_data

} app_state_t;

void get_current_state(app_state_t *state);
void set_current_state(app_state_t *state);

uint64_t get_next_scheduled_imu_trig_read();
void set_next_scheduled_imu_trig_read(uint64_t next);

void set_buffer1(uint8_t *imu_data, size_t len);
void get_buffer1(uint8_t *imu_data);
size_t get_buffer1_len();
uint32_t get_buffer1_timestamp();
uint64_t get_buffer1_trig_seq();
uint32_t get_buffer1_trig_timestamp();
struct imu_buff_t *get_buffer1_ptr();

void set_buffer1_state(uint8_t state);
uint8_t get_buffer1_state();

void set_buffer2(uint8_t *imu_data, size_t len);
void get_buffer2(uint8_t *imu_data);
size_t get_buffer2_len();
struct imu_buff_t *get_buffer2_ptr();

void set_buffer2_state(uint8_t state);
uint8_t get_buffer2_state();

uint8_t set_trigger_state(uint8_t state);
uint8_t get_trigger_state();

uint32_t set_exposure(uint32_t exposure);
uint32_t get_exposure();

uint64_t increase_imu_seq();
uint64_t increase_trigger_seq();

uint64_t get_imu_seq();
uint64_t get_trigger_seq();

bool is_app_initialized();
void set_app_initialized(bool state);

#endif /* APP_STATE_H_ */