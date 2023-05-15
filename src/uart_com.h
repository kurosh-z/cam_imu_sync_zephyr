#ifndef UART_COM_H_
#define UART_COM_H_

#include <SEGGER_RTT.h>
#include <stdio.h>
#include <string.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/byteorder.h>
#include <zephyr/sys/printk.h>

#define RECEIVE_TIMEOUT_MS 500
#define RECEIVE_BUFF_SIZE 128
#define TX_BUFF_SIZE 256
#define TX_BUFF_TIMEOUT_US 100 * 1000
#define BAUDRATE 921600
#define UART_RESPONSE_DELAY_MS 20
#define END_OF_MSG_LEN 8
#define END_OF_MSG                                                             \
  { 0x45, 0x4E, 0x44, 0x4F, 0x46, 0x4D, 0x53, 0x47 }

enum UARTResponseState { UART_RESPONSE_WAITING, UART_RESPONSE_SENT };

#define RESPONSE_QUEUE_SIZE 3
// struct
struct app_response_t {
  uint8_t resp[128];
  size_t len;
  uint8_t state;
};

struct app_response_queue_t {
  struct app_response_t queue[RESPONSE_QUEUE_SIZE];
  uint8_t head;
};

int init_uart_com();
int send_data_uart(uint8_t buff_num);

#endif
