
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
  // put valX
  encode_int32(val[0].val1, buf + 8 + 24 * i);
  encode_int32(val[0].val2, buf + 8 + 24 * i + 4);
  // put valY
  encode_int32(val[1].val1, buf + 16 + 24 * i);
  encode_int32(val[1].val2, buf + 16 + 24 * i + 4);
  // put valZ
  encode_int32(val[2].val1, buf + 24 + 24 * i);
  encode_int32(val[2].val2, buf + 24 + 24 * i + 4);
}

int process_mpu9250(bool triggered) {
  if (mpu9250_dev == NULL) {
    printk("mpu9250_dev is NULL");
    return -100;
  }
  uint8_t imu_data[IMU_DATA_MAX_LEN];
  // struct sensor_value temperature;
  struct sensor_value accel[3];
  struct sensor_value gyro[3];
  struct sensor_value magn[3];
  uint32_t start = k_cycle_get_32();
  int rc = sensor_sample_fetch(mpu9250_dev);
  uint32_t end = k_cycle_get_32();
  uint32_t process_delay_us = k_cyc_to_us_floor32(end - start);

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
    uint64_t imu_seq = increase_imu_seq();

    sys_put_be64(imu_seq, imu_data); // 8 bytes
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
    // TODO: add timestamp- add triggered flag to imu_data
    if (triggered) {
      set_buffer1(imu_data, 80);
      set_buffer1_state(IMU_BUFF_STATE_WAITING);
    }

    // imu_data[80] = triggered;                      // 1 byte
    // encode_int32(process_delay_us, imu_data + 81); // 4 bytes --> 85 bytes
    // printk("process delay [us]: %d\n", process_delay_us);
  }
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

// void imu_entry(void) {
//   SEGGER_RTT_Init();
//   k_sleep(K_SECONDS(3));
//   printk("starting imu \n");
//   // const struct device *mpu9250 = DEVICE_DT_GET(DT_NODELABEL(i2c0));
//   const struct device *mpu9250 = DEVICE_DT_GET_ONE(invensense_mpu9250);

//   if (!device_is_ready(mpu9250)) {
//     while (1) {
//       printk("Device %s is not ready\n", mpu9250->name);
//       k_sleep(K_SECONDS(4));
//     }
//     return;
//   }

// #ifdef CONFIG_MPU9250_TRIGGER
//   trigger = (struct sensor_trigger){
//       .type = SENSOR_TRIG_DATA_READY,
//       .chan = SENSOR_CHAN_ALL,
//   };
//   if (sensor_trigger_set(mpu9250, &trigger, handle_mpu9250_drdy) < 0) {
//     printk("Cannot configure trigger\n");
//     return;
//   }
//   printk("Configured for triggered sampling.\n");
// #endif

//   while (!IS_ENABLED(CONFIG_MPU9250_TRIGGER)) {
//     int rc = process_mpu9250(mpu9250);

//     if (rc != 0) {
//       break;
//     }
//     k_sleep(K_SECONDS(3));
//   }

//   /* triggered runs with its own thread after exit */
// }

// K_THREAD_DEFINE(imu_thread_id, IMU_STACK_SIZE, imu_entry, NULL, NULL, NULL,
//                 IMU_PRIORITY, 0, 0);