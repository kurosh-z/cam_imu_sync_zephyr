#ifndef UART_COM_H_
#define UART_COM_H_

#include <SEGGER_RTT.h>
#include <stdio.h>
#include <string.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

#define UART_PRIORITY 2
#define UART_STACK_SIZE 1024 * 6
#define RECEIVE_TIMEOUT_MS 500
#define RECEIVE_BUFF_SIZE 256
#define TX_BUFF_SIZE 256
#define TX_BUFF_TIMEOUT_MS 100
#define BAUDRATE 921600
#define END_OF_MSG_LEN 8
#define END_OF_MSG                                                             \
  { 0x45, 0x4E, 0x44, 0x4F, 0x46, 0x4D, 0x53, 0x47 }

// define trigger commands:
#define CMD_SET_EXPOSURE "CMD_SET_EXPOSURE"
#define CMD_TRIGGER "CMD_TRIGGER"

enum TriggerState {
  TRIGGER_STATE_STOP,
  TRIGGER_STATE_START,
  TRIGGER_STATE_PAUSE
};

typedef struct icState_t {
  uint8_t trigger_state;
  uint32_t exposure;
} icState_t;

int init_uart_com();
uint8_t send_data_uart(uint8_t *data, size_t len);
void get_current_icState(icState_t *state);
#endif
