##################################################################################
##
## Copyright (C) 2019, New H3C Technologies Co., Ltd. All rights reserved.
##
##      License: Revised BSD License, see LICENSE.TXT file included in the project
## Data Created: 2019/10/9
##      Authors: wangzhen
##  Description: CMake project file
##
##################################################################################
project(NucleoL073)
cmake_minimum_required(VERSION 3.6)
enable_language(ASM)

set(LINKER_SCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/board/cmsis/arm-gcc/stm32l073xx_flash.ld)
include(stm32l0)

list(APPEND ${PROJECT_NAME}_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/board/adc-board.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/board/board.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/board/delay-board.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/board/eeprom-board.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/board/gpio-board.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/board/lpm-board.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/board/rtc-board.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/board/spi-board.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/board/uart-board.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/board/cmsis/arm-gcc/startup_stm32l073xx.s"
    "${CMAKE_CURRENT_SOURCE_DIR}/board/cmsis/system_stm32l0xx.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../STM32Cube_FW_L0_V1.11.2/Drivers/STM32L0xx_HAL_Driver/Src/stm32l0xx_hal.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../STM32Cube_FW_L0_V1.11.2/Drivers/STM32L0xx_HAL_Driver/Src/stm32l0xx_hal_adc.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../STM32Cube_FW_L0_V1.11.2/Drivers/STM32L0xx_HAL_Driver/Src/stm32l0xx_hal_adc_ex.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../STM32Cube_FW_L0_V1.11.2/Drivers/STM32L0xx_HAL_Driver/Src/stm32l0xx_hal_cortex.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../STM32Cube_FW_L0_V1.11.2/Drivers/STM32L0xx_HAL_Driver/Src/stm32l0xx_hal_dma.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../STM32Cube_FW_L0_V1.11.2/Drivers/STM32L0xx_HAL_Driver/Src/stm32l0xx_hal_flash.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../STM32Cube_FW_L0_V1.11.2/Drivers/STM32L0xx_HAL_Driver/Src/stm32l0xx_hal_flash_ex.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../STM32Cube_FW_L0_V1.11.2/Drivers/STM32L0xx_HAL_Driver/Src/stm32l0xx_hal_gpio.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../STM32Cube_FW_L0_V1.11.2/Drivers/STM32L0xx_HAL_Driver/Src/stm32l0xx_hal_i2c.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../STM32Cube_FW_L0_V1.11.2/Drivers/STM32L0xx_HAL_Driver/Src/stm32l0xx_hal_i2c_ex.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../STM32Cube_FW_L0_V1.11.2/Drivers/STM32L0xx_HAL_Driver/Src/stm32l0xx_hal_pwr.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../STM32Cube_FW_L0_V1.11.2/Drivers/STM32L0xx_HAL_Driver/Src/stm32l0xx_hal_pwr_ex.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../STM32Cube_FW_L0_V1.11.2/Drivers/STM32L0xx_HAL_Driver/Src/stm32l0xx_hal_rcc.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../STM32Cube_FW_L0_V1.11.2/Drivers/STM32L0xx_HAL_Driver/Src/stm32l0xx_hal_rcc_ex.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../STM32Cube_FW_L0_V1.11.2/Drivers/STM32L0xx_HAL_Driver/Src/stm32l0xx_hal_rtc.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../STM32Cube_FW_L0_V1.11.2/Drivers/STM32L0xx_HAL_Driver/Src/stm32l0xx_hal_rtc_ex.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../STM32Cube_FW_L0_V1.11.2/Drivers/STM32L0xx_HAL_Driver/Src/stm32l0xx_hal_spi.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../STM32Cube_FW_L0_V1.11.2/Drivers/STM32L0xx_HAL_Driver/Src/stm32l0xx_hal_uart.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../STM32Cube_FW_L0_V1.11.2/Drivers/STM32L0xx_HAL_Driver/Src/stm32l0xx_hal_uart_ex.c"
)

if(MBED_RADIO_SHIELD STREQUAL SX1272MB2DAS)
    list(APPEND ${PROJECT_NAME}_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/board/sx1272mb2das-board.c")
elseif(MBED_RADIO_SHIELD STREQUAL SX1276MB1LAS)
    list(APPEND ${PROJECT_NAME}_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/board/sx1276mb1las-board.c")
elseif(MBED_RADIO_SHIELD STREQUAL SX1276MB1MAS)
    list(APPEND ${PROJECT_NAME}_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/board/sx1276mb1mas-board.c")
elseif(MBED_RADIO_SHIELD STREQUAL SX1261MBXBAS)
    list(APPEND ${PROJECT_NAME}_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/board/sx1261mbxbas-board.c")
elseif(MBED_RADIO_SHIELD STREQUAL SX1262MBXCAS)
    list(APPEND ${PROJECT_NAME}_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/board/sx1262mbxcas-board.c")
elseif(MBED_RADIO_SHIELD STREQUAL SX1262MBXDAS)
    list(APPEND ${PROJECT_NAME}_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/board/sx1262mbxdas-board.c")
else()
    message(STATUS " Please specify the MBED_RADIO_SHIELD !\nPossible values are: SX1272MB2DAS, SX1276MB1LAS, SX1276MB1MAS, SX1261MBXBAS, SX1262MBXCAS and SX1262MBXDAS.")
endif()

add_library(${PROJECT_NAME} OBJECT EXCLUDE_FROM_ALL ${${PROJECT_NAME}_SOURCES})

target_compile_definitions(${PROJECT_NAME} PUBLIC -DUSE_HAL_DRIVER -DSTM32L073xx -D${MBED_RADIO_SHIELD})


target_include_directories(${PROJECT_NAME} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include
	${CMAKE_CURRENT_SOURCE_DIR}/board
    ${CMAKE_CURRENT_SOURCE_DIR}/board/cmsis
	${CMAKE_CURRENT_SOURCE_DIR}/../STM32Cube_FW_L0_V1.11.2/Drivers/CMSIS/Include
    ${CMAKE_CURRENT_SOURCE_DIR}/../STM32Cube_FW_L0_V1.11.2/Drivers/STM32L0xx_HAL_Driver/Inc
	${CMAKE_CURRENT_SOURCE_DIR}/../../../lora-sdk/system
	${CMAKE_CURRENT_SOURCE_DIR}/../../../lora-sdk/loramac/radio
)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/apps)

set_property(TARGET ${PROJECT_NAME} PROPERTY C_STANDARD 11)