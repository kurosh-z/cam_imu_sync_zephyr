
#include "imu.h"

// LOG_MODULE_DECLARE(imu, LOG_LEVEL_WRN);

const struct device *mpu9250_dev = DEVICE_DT_GET_ONE(invensense_mpu9250);

static const char *now_str(void) {
  static char buf[16]; /* ...HH:MM:SS.MMM */
  uint32_t now = k_uptime_get_32();

  unsigned int ms = now % MSEC_PER_SEC;
  unsigned int s;
  unsigned int min;
  unsigned int h;

  now /= MSEC_PER_SEC;
  s = now % 60U;
  now /= 60U;
  min = now % 60U;
  now /= 60U;
  h = now;

  snprintf(buf, sizeof(buf), "%u:%02u:%02u.%03u", h, min, s, ms);
  return buf;
}

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

uint64_t data_counter = 0;
uint16_t cnt = 0;
double average = 0;
int process_mpu9250(uint8_t *imu_data, received_data_cb_t cb) {

  if (mpu9250_dev == NULL) {
    printk("mpu9250_dev is NULL");
    return -100;
  }

  // struct sensor_value temperature;
  struct sensor_value accel[3];
  struct sensor_value gyro[3];
  struct sensor_value magn[3];
  uint32_t start = k_cycle_get_32();
  int rc = sensor_sample_fetch(mpu9250_dev);
  uint32_t end = k_cycle_get_32();
  uint32_t process_delay_us = k_cyc_to_us_floor32(end - start);
  average += process_delay_us;
  uint8_t triggered = 0;
  if (cb != NULL) {
    triggered = cb(data_counter, process_delay_us);
  }

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
    data_counter++;

    sys_put_be64(data_counter, imu_data); // 8 bytes
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

    imu_data[80] = triggered;                      // 1 byte
    encode_int32(process_delay_us, imu_data + 81); // 4 bytes --> 85 bytes
    printk("process delay [us]: %d\n", process_delay_us);
    // clang-format off
    //  snprintf(imu_data, 256, IMU_FORMAT_OUT, data_counter,
    //           accel[0].val1, accel[0].val2, 
    //           accel[1].val1, accel[1].val2, 
    //           accel[2].val1, accel[2].val2,
    //           gyro[0].val1, gyro[0].val2, 
    //           gyro[1].val1, gyro[1].val2, 
    //           gyro[2].val1, gyro[2].val2,
    //           magn[0].val1, magn[0].val2,
    //           magn[1].val1, magn[1].val2,
    //           magn[2].val1, magn[2].val2);
              
    //clang-format on   

    // printk("---------------------------------\n");
    // uint8_t mag_test[4] = {0, 0, 0, 0};
    // sys_put_be64(data_counter, imu_data);
    // for (int i = 0; i < 3; i++) {
    //   sys_put_sys_be32(accel[0].val1, imu_data + 8 + 4 * i);
    //   sys_put_sys_be32(accel[0].val2, imu_data + 8 + 4 * (i + 1));
    // }
    // sys_put_be32(accel[1].val2, mag_test);
    // memcpy(imu_data, mag_test, 4);

    // printk("mag_test: %d-> %x, %x, %x, %x \n\n", accel[1].val2, mag_test[0],
    //        mag_test[1], mag_test[2], mag_test[3]);

      
        // printk(
        // "  cnt  %llu\n"
        // "  accel %f %f %f m/s/s\n"
        // "  gyro  %f %f %f rad/s\n"
        // "  magn  %f %f %f rad\n",
        // data_counter,
        // sensor_value_to_double(&accel[0]), sensor_value_to_double(&accel[1]),
        // sensor_value_to_double(&accel[2]), sensor_value_to_double(&gyro[0]),
        // sensor_value_to_double(&gyro[1]), sensor_value_to_double(&gyro[2]),sensor_value_to_double(&magn[0]),
        // sensor_value_to_double(&magn[1]), sensor_value_to_double(&magn[2]));

      // printk("\"================================\n\n");

    // printk("imu_data:%d, %s \n\n", strlen(imu_data),imu_data);
    // printk(
    //     // "[%s]:%g Cel\n"
    //     "  acX    1: %d 2: %d \n"
    //     "  acY    1: %d 2: %d \n"
    //     "  acZ    1: %d 2: %d \n"
    //     "  gyroX  1: %d 2: %d \n"
    //     "  gyroY  1: %d 2: %d \n"
    //     "  gyroZ  1: %d 2: %d \n"
    //     "  magnX  1: %d 2: %d \n"
    //     "  magnY  1: %d 2: %d \n"
    //     "  magnZ  1: %d 2: %d \n\n",
    //     accel[0].val1, accel[0].val2,
    //     accel[1].val1, accel[1].val2,
    //     accel[2].val1, accel[2].val2,
    //     gyro[0].val1, gyro[0].val2,
    //     gyro[1].val1, gyro[1].val2,
    //     gyro[2].val1, gyro[2].val2,
    //     magn[0].val1, magn[0].val2,
    //     magn[1].val1, magn[1].val2,
    //     magn[2].val1, magn[2].val2);
  
       
      

  } else {
    printk("sample fetch/get failed: %d\n", rc);
  }

  return rc;
}

#ifdef CONFIG_MPU9250_TRIGGER
static struct sensor_trigger trigger;

static void handle_mpu9250_drdy(const struct device *dev,
                                const struct sensor_trigger *trig) {
  int rc = process_mpu9250(dev);

  if (rc != 0) {
    printk("cancelling trigger due to failure: %d\n", rc);
    (void)sensor_trigger_set(dev, trig, NULL);
    return;
  }
}
#endif /* CONFIG_MPU9250_TRIGGER */

int init_imu() {
  printk("initializing imu device \n");
  // const struct device *mpu9250 = DEVICE_DT_GET(DT_NODELABEL(i2c0));

  if (!device_is_ready(mpu9250_dev)) {
    printk("Device mpu9250 is not ready!\n");
    return -1;
  }
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