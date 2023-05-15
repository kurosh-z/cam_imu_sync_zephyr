#include "app_state.h"

struct app_state_t app_state = {
    .imu_seq = 0,
    .trigger_seq = 0,
    .trigger_state = TRIGGER_STATE_STOP,
    .error = NO_ERROR,
    .initialized = false,
    .exposure = 12,
    .imu_buff1 =
        {
            .data = {0},
            .state = IMU_BUFF_STATE_SENT,
            .len = 0,
            .timestamp = 0,
            .trig_seq = 0,
            .trig_timestamp = 0,
        },
    .imu_buff2 =
        {
            .data = {0},
            .state = IMU_BUFF_STATE_SENT,
            .len = 0,
            .timestamp = 0,
            .trig_seq = 0,
            .trig_timestamp = 0,
        },
};

void get_current_state(app_state_t *state) {
  memcpy(state, &app_state, sizeof(app_state_t));
};

void set_current_state(app_state_t *state) {
  memcpy(&app_state, state, sizeof(app_state_t));
};

void set_buffer1(uint8_t *imu_data, size_t len) {
  memcpy(app_state.imu_buff1.data, imu_data, len);
  app_state.imu_buff1.len = len;
};
void get_buffer1(uint8_t *imu_data) {
  memcpy(imu_data, app_state.imu_buff1.data, app_state.imu_buff1.len);
}
size_t get_buffer1_len() { return app_state.imu_buff1.len; }
struct imu_buff_t *get_buffer1_ptr() { return &app_state.imu_buff1; }

void set_buffer1_state(uint8_t state) { app_state.imu_buff1.state = state; };
uint8_t get_buffer1_state() { return app_state.imu_buff1.state; };
struct imu_buff_t *get_buffer2_ptr() { return &app_state.imu_buff2; }

uint32_t get_buffer1_timestamp() { return app_state.imu_buff1.timestamp; }
uint64_t get_buffer1_trig_seq() { return app_state.imu_buff1.trig_seq; }
uint32_t get_buffer1_trig_timestamp() {
  return app_state.imu_buff1.trig_timestamp;
}

void set_buffer2(uint8_t *imu_data, size_t len) {
  memcpy(app_state.imu_buff2.data, imu_data, len);
  app_state.imu_buff2.len = len;
};
void get_buffer2(uint8_t *imu_data) {
  memcpy(imu_data, app_state.imu_buff2.data, app_state.imu_buff2.len);
}
size_t get_buffer2_len() { return app_state.imu_buff2.len; }

void set_buffer2_state(uint8_t state) { app_state.imu_buff2.state = state; };
uint8_t get_buffer2_state() { return app_state.imu_buff2.state; };

/***
 * @brief Set trigger state
 * @param state TRIGGER_STATE_STOP or TRIGGER_STATE_START
 * @return state if state is valid, otherwise return current state
 */
uint8_t set_trigger_state(uint8_t state) {
  if (state == TRIGGER_STATE_STOP || state == TRIGGER_STATE_START) {
    app_state.trigger_state = state;
    return state;
  } else {
    return app_state.trigger_state;
  }
};

/***
 * @brief Set exposure time in milliseconds
 * @param exposure_ms Exposure time in milliseconds
 * @return exposure_ms if exposure_ms is in range, otherwise return min or max
 */
uint32_t set_exposure(uint32_t exposure_ms) {
  uint32_t exp_min = IMU_READ_TIME_US / 1000;
  uint32_t exp_max = (TRIGGER_INTERVAL_US - 500) / 1000;
  if (exposure_ms < exp_min) {
    app_state.exposure = exp_min;
    return exp_min;
  } else if (exposure_ms > exp_max) {
    app_state.exposure = exp_max;
    return exp_max;
  } else {
    app_state.exposure = exposure_ms;
    return exposure_ms;
  }
}

uint32_t get_exposure() { return app_state.exposure; };
uint8_t get_trigger_state() { return app_state.trigger_state; };

uint64_t increase_imu_seq() {
  app_state.imu_seq++;
  return app_state.imu_seq;
};
uint64_t increase_trigger_seq() {
  app_state.trigger_seq++;
  return app_state.trigger_seq;
};
uint64_t get_imu_seq() { return app_state.imu_seq; };
uint64_t get_trigger_seq() { return app_state.trigger_seq; };

bool is_app_initialized() { return app_state.initialized; };
void set_app_initialized(bool state) { app_state.initialized = state; };