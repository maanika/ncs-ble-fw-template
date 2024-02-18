# Copyright (c) 2021 Nordic Semiconductor ASA
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.13.1)

set(mcuboot_CONFIG_BOOT_SIGNATURE_TYPE_ECDSA_P256 y)
set(mcuboot_CONFIG_BOOT_SIGNATURE_KEY_FILE
    \"${CMAKE_CURRENT_SOURCE_DIR}/../priv-nrf52833dk_nrf52833.pem\")

set(PM_STATIC_YML_FILE
    ${CMAKE_CURRENT_SOURCE_DIR}/configuration/${BOARD}/pm_static.yml)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

project(app LANGUAGES C)

target_sources(app PRIVATE
    src/main.c
)
