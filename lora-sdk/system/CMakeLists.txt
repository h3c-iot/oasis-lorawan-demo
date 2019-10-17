##################################################################################
##
## Copyright (C) 2019, New H3C Technologies Co., Ltd.
##
##      License: Revised BSD License, see LICENSE.TXT file included in the project
## Data Created: 2019/10/9
##      Authors: wangzhen
##  Description: CMake project file
##
##################################################################################
project(system)
cmake_minimum_required(VERSION 3.6)


#list(APPEND ${PROJECT_NAME}_SOURCES
#    "${CMAKE_CURRENT_SOURCE_DIR}/adc.c"
#    "${CMAKE_CURRENT_SOURCE_DIR}/delay.c"
#    "${CMAKE_CURRENT_SOURCE_DIR}/eeprom.c"
#    "${CMAKE_CURRENT_SOURCE_DIR}/fifo.c"
#    "${CMAKE_CURRENT_SOURCE_DIR}/gpio.c"
#    "${CMAKE_CURRENT_SOURCE_DIR}/gps.c"
#    "${CMAKE_CURRENT_SOURCE_DIR}/i2c.c"
#    "${CMAKE_CURRENT_SOURCE_DIR}/nvmm.c"
#    "${CMAKE_CURRENT_SOURCE_DIR}/systime.c"
#    "${CMAKE_CURRENT_SOURCE_DIR}/timer.c"
#    "${CMAKE_CURRENT_SOURCE_DIR}/uart.c"
#    "${CMAKE_CURRENT_SOURCE_DIR}/utilities.c"
#)

file(GLOB ${PROJECT_NAME}_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/*.c"
)

add_library(${PROJECT_NAME} OBJECT EXCLUDE_FROM_ALL ${${PROJECT_NAME}_SOURCES})

# 引用头文件
target_include_directories( ${PROJECT_NAME} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}/../../example/include
)

