# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(cellular_modem)

target_sources(app PRIVATE 
    src/main.c
    src/espressif-esp32-operator.c
    src/espressif.c
)