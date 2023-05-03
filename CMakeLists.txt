# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)
set(BOARD nrf52840dk_nrf52840)
# set(DTC_OVERLAY_FILE overlay_SSI.dts)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(blinky)

target_sources(app PRIVATE src/main.c)
target_sources(app PRIVATE src/uart_com.c)
target_sources(app PRIVATE src/imu.c)
target_sources(app PRIVATE src/trigger.c)
