#include "app_state.h"

struct app_state_t app_state = {
    .imu_seq = 0,
    .trigger_seq = 0,
    .trigger_state = TRIGGER_STATE_STOP,
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

void set_trigger_state(uint8_t state) { app_state.trigger_state = state; };
void set_exposure(uint32_t exposure) { app_state.exposure = exposure; };

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