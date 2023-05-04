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
#include "app_state.h"

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

int send_data_uart(uint8_t buff_num) {
  // printk("[uart_com.c] send_data_uart ... \n");
  int len = buff_num == 1 ? get_buffer1_len() : get_buffer2_len();
  int ret;
  int _len = len + END_OF_MSG_LEN;
  if (_len > TX_BUFF_SIZE) {
    printk("ERROR: len is bigger than TX_BUFF_SIZE \n");
    return -100;
  }

  memset(tx_buf, 0, sizeof(tx_buf));
  if (buff_num == 1) {
    get_buffer1(tx_buf);
  } else if (buff_num == 2) {
    get_buffer2(tx_buf);
  } else {
    printk("ERROR: expected buff_num 1 or 2  got %d\n", buff_num);
    return -200;
  }
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

void uart_entry(void) {
  SEGGER_RTT_Init();
  k_msleep(500);
  printk("[uart_com.c] uart_entry ... \n");
  while (!is_app_initialized()) {
    k_msleep(500);
  }

  while (1) {
    if (get_buffer1_state() == IMU_BUFF_STATE_WAITING) {
      send_data_uart(1);
      set_buffer1_state(IMU_BUFF_STATE_SENT);
    }
    if (get_buffer2_state() == IMU_BUFF_STATE_WAITING) {
      send_data_uart(2);
      set_buffer2_state(IMU_BUFF_STATE_SENT);
    }
    k_sleep(K_USEC(500));
  }
}

K_THREAD_DEFINE(uart_thread_id, 2 * 1024, uart_entry, NULL, NULL, NULL,
                UART_THREAD_PRIORITY, 0, 0);