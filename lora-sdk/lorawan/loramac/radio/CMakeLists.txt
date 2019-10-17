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
project(radio)
cmake_minimum_required(VERSION 3.6)

#---------------------------------------------------------------------------------------
# 定制射频模块
#---------------------------------------------------------------------------------------
# 选择radio射频芯片, 目前只支持sx1276(sx1278)
set(RADIO_LIST sx1272 sx1276 sx126x)
set(RADIO sx1276 CACHE STRING "Default radio is sx1276")

# 匹配Radio
if(MBED_RADIO_SHIELD STREQUAL SX1272MB2DAS)
    set(RADIO sx1272 CACHE INTERNAL "Radio sx1272 selected")
elseif(MBED_RADIO_SHIELD STREQUAL SX1276MB1LAS OR MBED_RADIO_SHIELD STREQUAL SX1276MB1MAS OR MBED_RADIO_SHIELD STREQUAL SX1278H3C OR MBED_RADIO_SHIELD STREQUAL SX1278ACSIPS78F OR MBED_RADIO_SHIELD STREQUAL WSL305S)
    set(RADIO sx1276 CACHE INTERNAL "Radio sx1276 selected")
elseif(MBED_RADIO_SHIELD STREQUAL SX1261MBXBAS OR MBED_RADIO_SHIELD STREQUAL SX1262MBXCAS OR MBED_RADIO_SHIELD STREQUAL SX1262MBXDAS)
    set(RADIO sx126x CACHE INTERNAL "Radio sx126x selected")
else()
    message(STATUS "Please specify the MBED_RADIO_SHIELD!\nPossible values are: SX1272MB2DAS, SX1276MB1LAS, SX1276MB1MAS, SX1261MBXBAS, SX1262MBXCAS and SX1262MBXDAS.")
endif()

set_property(CACHE RADIO PROPERTY STRINGS ${RADIO_LIST})
set_property(CACHE RADIO PROPERTY ADVANCED)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}/${RADIO}
	${CMAKE_CURRENT_SOURCE_DIR}/../../../system
	${CMAKE_CURRENT_SOURCE_DIR}/../../../../example/include
)

#---------------------------------------------------------------------------------------
# Target
#---------------------------------------------------------------------------------------

file(GLOB ${PROJECT_NAME}_SOURCES "${RADIO}/*.c")

add_library(${PROJECT_NAME} OBJECT EXCLUDE_FROM_ALL ${${PROJECT_NAME}_SOURCES})

#add_dependencies(${PROJECT_NAME} example)

