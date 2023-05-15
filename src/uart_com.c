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
static const uint8_t end_of_msg[END_OF_MSG_LEN] = END_OF_MSG;

static struct app_response_queue_t response_queue = {
    .head = 0,
    .queue = {
        {.resp = {0}, .len = 0, .state = UART_RESPONSE_SENT},
        {.resp = {0}, .len = 0, .state = UART_RESPONSE_SENT},
        {.resp = {0}, .len = 0, .state = UART_RESPONSE_SENT},
    }};

static struct k_work_delayable send_response_work;

void send_response_handler(struct k_work *work) {
  bool sent = false;
  for (uint8_t i = 0; i < RESPONSE_QUEUE_SIZE; i++) {
    if (response_queue.queue[i].state == UART_RESPONSE_WAITING) {
      // if no response is sent yet, send it
      if (!sent) {
        // add END_OF_MSG to the end of the response
        size_t len = response_queue.queue[i].len + END_OF_MSG_LEN;
        memcpy(&response_queue.queue[i].resp[response_queue.queue[i].len],
               end_of_msg, END_OF_MSG_LEN);
        uart_tx(uart, response_queue.queue[i].resp, len, TX_BUFF_TIMEOUT_US);
        printk("[uart_com.c] send_response_handler:len= %d,  %s\n",
               response_queue.queue[i].len, response_queue.queue[i].resp);
        k_msleep(10);

        // for (int j = 0; j < len; j++)
        //   printk("0x%x ", response_queue.queue[i].resp[j]);
        // printk("\n");
        // k_msleep(10);
        response_queue.queue[i].state = UART_RESPONSE_SENT;
        memset(response_queue.queue[i].resp, 0,
               sizeof(response_queue.queue[i].resp));
        response_queue.queue[i].len = 0;
        sent = true;
      }

      //  else {
      //   // if already sent once and there is another response in the queue
      //   // schedule it to be sent later
      //   printk("[uart_com.c] send_response_handler: rescheduling the next "
      //          "response item\n");
      //   k_work_reschedule(&send_response_work,
      //   K_MSEC(UART_RESPONSE_DELAY_MS));
      // }
    }
  }
}

static void parse_command(uint8_t *data, size_t len) {

  bool response_scheduled = false;
  printk("\n-------------------------------\n");
  printk("[uart_com.c] parse_command: %s\n", data);
  // for (int i = 0; i < len; i++)
  //   printk("%x ", data[i]);
  // printk("\n");

  if (strncmp(data, CMD_SET_EXPOSURE, 17) == 0) {
    uint32_t exposure_ms = sys_get_be32(&data[17]);
    printk("[uart_com.c] parse_command:CMD_SET_EXPOSURE = %d\n", exposure_ms);
    uint32_t new_exposure = set_exposure(exposure_ms);
    for (uint8_t i = 0; i < RESPONSE_QUEUE_SIZE; i++) {
      if (response_queue.queue[i].state == UART_RESPONSE_SENT) {
        memcpy(response_queue.queue[i].resp, RESP_SET_EXPOSURE, 18); // 18 bytes
        sys_put_be32(new_exposure,
                     &response_queue.queue[i].resp[18]); // + 4 bytes
        response_queue.queue[i].len = 22;
        response_queue.queue[i].state = UART_RESPONSE_WAITING;
        response_scheduled = true;
        break;
      }
    }
  } else if (strncmp(data, CMD_GET_EXPOSURE, 16) == 0) {
    uint32_t exposure_ms = get_exposure();
    printk("[uart_com.c] parse_command:CMD_GET_EXPOSURE = %d\n", exposure_ms);
    for (uint8_t i = 0; i < RESPONSE_QUEUE_SIZE; i++) {
      if (response_queue.queue[i].state == UART_RESPONSE_SENT) {
        memcpy(response_queue.queue[i].resp, RESP_GET_EXPOSURE, 17); // 17 bytes
        sys_put_be16(exposure_ms,
                     &response_queue.queue[i].resp[17]); // + 4 bytes

        response_queue.queue[i].len = 21;
        response_queue.queue[i].state = UART_RESPONSE_WAITING;
        response_scheduled = true;
        break;
      }
    }
  } else if (strncmp(data, CMD_SET_TRIGGER, 16) == 0) {
    uint8_t trigger_mode = data[16];
    printk("[uart_com.c] parse_command:CMD_SET_TRIGGER = %d\n", trigger_mode);
    uint8_t new_trigger_mode = set_trigger_state(trigger_mode);
    for (uint8_t i = 0; i < RESPONSE_QUEUE_SIZE; i++) {
      if (response_queue.queue[i].state == UART_RESPONSE_SENT) {
        memcpy(response_queue.queue[i].resp, RESP_SET_TRIGGER, 17); // 17 bytes
        response_queue.queue[i].resp[17] = new_trigger_mode;        // + 1 byte
        response_queue.queue[i].len = 18;
        response_queue.queue[i].state = UART_RESPONSE_WAITING;
        response_scheduled = true;
        break;
      }
    }
  } else if (strncmp(data, CMD_GET_TRIGGER, 16) == 0) {
    uint8_t trigger_mode = get_trigger_state();
    printk("[uart_com.c] parse_command:CMD_GET_TRIGGER = %d\n", trigger_mode);
    for (uint8_t i = 0; i < RESPONSE_QUEUE_SIZE; i++) {
      if (response_queue.queue[i].state == UART_RESPONSE_SENT) {
        memcpy(response_queue.queue[i].resp, RESP_GET_TRIGGER, 17); // 17 bytes
        response_queue.queue[i].resp[17] = trigger_mode;            // + 1 byte
        response_queue.queue[i].len = 18;
        response_queue.queue[i].state = UART_RESPONSE_WAITING;
        response_scheduled = true;
        break;
      }
    }
  }

  else {
    printk(
        "[uart_com.c/parse_command] parse_command: WARN: unknown command \n");
    return;
  }

  if (!response_scheduled) {
    printk("[uart_com.c/parse_command] parse_command: WARN: response queue "
           "is full \n");
    return;
  }
  k_work_reschedule(&send_response_work, K_MSEC(UART_RESPONSE_DELAY_MS));
}

static void uart_callback(const struct device *dev, struct uart_event *evt,
                          void *user_data) {
  switch (evt->type) {
  case UART_RX_RDY:
    if (user_data != NULL) {
      printk("[uart_com.c] uart_callback: WARN: user_data call is not "
             "implemented  yet!"
             "\n");
    }
    parse_command(evt->data.rx.buf, evt->data.rx.len);
    uart_rx_disable(dev);
    break;
  case UART_RX_DISABLED:
    uart_rx_enable(dev, rx_buf, sizeof rx_buf, 1000);
  default:
    break;
  }
}

int init_uart_com() {
  int ret;
  printk("[uart_com.c] init_uart_com ... \n");
  if (!device_is_ready(uart)) {
    printk("[uart_com.c] uart device is not ready \n");
    return -1;
  }
  printk("[uart_com.c] uart device is ready \n");
  k_msleep(1000);
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
  printk("[uart_com.c] uart_callback_set set \n");
  uart_rx_enable(uart, rx_buf, sizeof rx_buf, 1000);
  // initializing send response work
  k_work_init_delayable(&send_response_work, send_response_handler);
  k_msleep(100);
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
  ret = uart_tx(uart, tx_buf, _len, TX_BUFF_TIMEOUT_US);
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