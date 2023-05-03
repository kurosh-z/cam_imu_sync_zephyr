/**
 * @file uart_com.c
 * @author kurosh zamani (kurosh.zamany@gmail.com)
 * @brief
 * @version 0.1
 * @date 2023-04-20
 *
 * @copyright Copyright (c) 2023
 *
 */

#include "uart_com.h"

const struct device *uart = DEVICE_DT_GET(DT_NODELABEL(uart0));
static uint8_t tx_buf[TX_BUFF_SIZE] = {0};
static uint8_t rx_buf[RECEIVE_BUFF_SIZE] = {0};
static uint8_t end_of_msg[END_OF_MSG_LEN] = END_OF_MSG;

static void uart_callback(const struct device *dev, struct uart_event *evt,
                          void *user_data) {
  switch (evt->type) {
  case UART_RX_RDY:
    if (user_data != NULL) {
      printk("[uart_com.c] uart_callback: WARN: user_data call is not "
             "implemented  yet!"
             "\n");
    }
    // TODO: implement receive data
    uart_rx_disable(dev);
    break;
  case UART_RX_DISABLED:
    uart_rx_enable(dev, rx_buf, sizeof rx_buf, RECEIVE_TIMEOUT_MS);
  default:
    break;
  }
}

static void parse_command(uint8_t *data, size_t len) {}

int init_uart_com() {
  int ret;
  printk("[uart_com.c] init_uart_com ... \n");
  if (!device_is_ready(uart)) {
    printk("[uart_com.c] uart device is not ready \n");
    return -1;
  }
  k_msleep(2000);
  struct uart_config uart_cfg = {
      .baudrate = BAUDRATE,
      .parity = UART_CFG_PARITY_NONE,
      .stop_bits = UART_CFG_STOP_BITS_1,
      .data_bits = UART_CFG_DATA_BITS_8,
      .flow_ctrl = UART_CFG_FLOW_CTRL_NONE,
  };
  ret = uart_configure(uart, &uart_cfg);
  if (ret) {
    printk("[uart_com.c] uart_configure failed: %d\n", ret);
  } else {
    printk("[uart_com.c] uart_configure success \n");
  }

  ret = uart_callback_set(uart, uart_callback, NULL);
  if (ret != 0) {
    printk("[uart_com.c] uart_callback_set failed \n");
    return ret;
  }
  k_msleep(300);
  printk("[uart_com.c] uart_callback_set success \n");
  return 0;
}

struct data_out {
  uint64_t cnt;
  uint8_t dummy1;
  uint8_t arr[10];
};
const char *OUT_FORMAT = "%llu,%d,%s\n";

struct data_out data_out = {.cnt = 0, .dummy1 = 0, .arr = "testing"};
void fill_tx_with_data() {
  data_out.cnt++;
  data_out.dummy1 = (uint8_t)(data_out.cnt / 2);
  // printf("[uart_com.c] fill_tx_with_data ... \n");
  memset(tx_buf, 0, sizeof(tx_buf));
  // printk("\n-----------");
  // printk("cnt: %llu \n", data_out.cnt);
  snprintf(tx_buf, sizeof(tx_buf), OUT_FORMAT, data_out.cnt, data_out.dummy1,
           data_out.arr);
  printk("tx_buf: %s \n", tx_buf);
  // for (int i = 0; i < strlen(tx_buf); i++) {
  //   // tx_buf[i] = i;
  //   printk("0x%x -  ", tx_buf[i]);
  // }
  // printk("\n");

  int err = uart_tx(uart, tx_buf, strlen(tx_buf), TX_BUFF_TIMEOUT_MS);
  // test unpacking
  // printf("\n\n unpacking ... \n\n");
  // uint64_t cnt = 0;
  // for (int i = 7; i >= 0; i--) {
  //   uint8_t sh = 8 * i;
  //   printk(" tx_buf[%d] << %d: 0x%x \n", i, sh, tx_buf[i] << 8 * i);
  //   cnt = cnt | tx_buf[i] << 8 * (i);
  // }

  // while (1) {
  // printk("data_cnt: %lld, cnt: %lld \n", data_cnt, cnt);
  // printk("diff: %lld", cnt - data_cnt);

  // }
}

uint8_t send_data_uart(uint8_t *data, size_t len) {
  int ret;
  int _len = len + END_OF_MSG_LEN;
  if (_len > TX_BUFF_SIZE) {
    printk("ERROR: len is bigger than TX_BUFF_SIZE \n");
    return -100;
  }

  memset(tx_buf, 0, sizeof(tx_buf));
  memcpy(tx_buf, data, len);
  memcpy(tx_buf + len, end_of_msg, END_OF_MSG_LEN);
  // printk("tx_buf: \n");
  // for (int i = 0; i < _len; i++) {
  //   printk("0x%x - ", tx_buf[i]);
  //   k_msleep(2);
  // }
  // printk("\n");
  ret = uart_tx(uart, tx_buf, _len, TX_BUFF_TIMEOUT_MS);
  return ret;
}

// void uart_entry(void) {
//   SEGGER_RTT_Init();
//   printk("[uart_com.c] uart_entry ... \n");
//   int ret = init_uart_com();
//   if (ret) {
//     while (1) {
//       printk("[uart_com.c/uart_entry] init_uart_com failed \n");
//     }
//   }
//   k_msleep(1000);
//   while (1) {

//     fill_tx_with_data();
//     // ret = send_data_uart();
//     // printk("[uart_com.c/uart_entry] send_data_uart %d \n", ret);
//     k_msleep(5000);
//   }
// }

// K_THREAD_DEFINE(uart_thread_id, UART_STACK_SIZE, uart_entry, NULL, NULL,
// NULL,
//                 UART_PRIORITY, 0, 0);