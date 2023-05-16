
#include "imu.h"
#include "app_state.h"

// LOG_MODULE_DECLARE(imu, LOG_LEVEL_WRN);
static struct k_work_delayable imu_read_work;
const struct device *mpu9250_dev = DEVICE_DT_GET_ONE(invensense_mpu9250);

static inline void encode_int32(int32_t n, uint8_t *bytes) {
  bytes[0] = (n >> 24) & 0xFF;
  bytes[1] = (n >> 16) & 0xFF;
  bytes[2] = (n >> 8) & 0xFF;
  bytes[3] = (n >> 0) & 0xFF;
}

static inline void put_sensor_val_to_buf(const struct sensor_value *val,
                                         uint8_t *buf, int i) {
  // put valX  //  "IMU" + imu_seq + imu_timestamp -> 19 bytes
  encode_int32(val[0].val1, buf + 3 + 16 + 24 * i);
  encode_int32(val[0].val2, buf + 3 + 16 + 24 * i + 4); // +8 bytes
  // put valY
  encode_int32(val[1].val1, buf + 3 + 24 + 24 * i);
  encode_int32(val[1].val2, buf + 3 + 24 + 24 * i + 4); // +8 bytes
  // put valZ
  encode_int32(val[2].val1, buf + 3 + 32 + 24 * i);
  encode_int32(val[2].val2, buf + 3 + 32 + 24 * i + 4); // +8 bytes
}

int process_mpu9250(bool triggered) {
  if (mpu9250_dev == NULL) {
    printk("mpu9250_dev is NULL");
    return -100;
  }

  uint8_t imu_data[IMU_DATA_MAX_LEN];
  memset(imu_data, 0, IMU_DATA_MAX_LEN);
  // struct sensor_value temperature;
  struct sensor_value accel[3];
  struct sensor_value gyro[3];
  struct sensor_value magn[3];

  uint64_t imu_timestamp = k_ticks_to_us_floor64(k_uptime_ticks());
  int rc = sensor_sample_fetch(mpu9250_dev);
  // uint32_t end = k_cycle_get_32();
  // uint32_t process_delay_us = k_cyc_to_us_floor32(end - start);

  if (rc == 0) {
    rc = sensor_channel_get(mpu9250_dev, SENSOR_CHAN_ACCEL_XYZ, accel);
  }
  if (rc == 0) {
    rc = sensor_channel_get(mpu9250_dev, SENSOR_CHAN_GYRO_XYZ, gyro);
  }
  if (rc == 0) {
    rc = sensor_channel_get(mpu9250_dev, SENSOR_CHAN_MAGN_XYZ, magn);
  }

  if (rc == 0) {
    memcpy(imu_data, "IMU", 3); // 3 bytes
    uint64_t imu_seq = increase_imu_seq();
    sys_put_be64(imu_seq, imu_data + 3);        // + 8 bytes
    sys_put_be64(imu_timestamp, imu_data + 11); // +8 bytes

    for (int i = 0; i < 3; i++) {
      switch (i) {
      case 0:
        put_sensor_val_to_buf(accel, imu_data, i);
        break;
      case 1:
        put_sensor_val_to_buf(gyro, imu_data, i);
        break;
      case 2:
        put_sensor_val_to_buf(magn, imu_data, i);
        break;

      default:
        break;
      }
    }
    // 19 + 24*3 = 91 bytes
    if (triggered) {
      imu_data[91] = triggered;                                   // 1 byte
      sys_put_be64(get_buffer1_trig_seq(), imu_data + 92);        // 8 bytes
      sys_put_be64(get_buffer1_trig_timestamp(), imu_data + 100); // 8 bytes
      set_buffer1(imu_data, 108);
      set_buffer1_state(IMU_BUFF_STATE_WAITING);
    } else {
      imu_data[91] = triggered;
      set_buffer2(imu_data, 92);
      set_buffer2_state(IMU_BUFF_STATE_WAITING);
    }

    // imu_data[80] = triggered;                      // 1 byte
    // encode_int32(process_delay_us, imu_data + 81); // 4 bytes --> 85 bytes
    // printk("process delay [us]: %d\n", process_delay_us);
  }
  return rc;
}

void imu_read_handler_triggered(struct k_work *work) { process_mpu9250(true); }

void schedule_imu_read_triggered(uint32_t delay_us) {
  k_work_reschedule(&imu_read_work, K_USEC(delay_us));
}

int init_imu() {
  printk("initializing imu device \n");
  // const struct device *mpu9250 = DEVICE_DT_GET(DT_NODELABEL(i2c0));

  if (!device_is_ready(mpu9250_dev)) {
    printk("Device mpu9250 is not ready!\n");
    return -1;
  }
  k_work_init_delayable(&imu_read_work, imu_read_handler_triggered);
  return 0;
}

void imu_entry(void) {
  SEGGER_RTT_Init();
  k_msleep(1000);
  printk("imu thread is started\n");
  while (!is_app_initialized()) {
    k_msleep(800);
  }
  while (1) {
    process_mpu9250(false);
    k_sleep(
        Z_TIMEOUT_US((uint32_t)(2481))); // TODO: see if this time still valid
  }
}

K_THREAD_DEFINE(imu_thread_id, 1024 * 2, imu_entry, NULL, NULL, NULL,
                IMU_THREAD_PRIORITY, 0, 0);
