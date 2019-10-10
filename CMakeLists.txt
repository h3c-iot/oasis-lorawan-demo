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
project(oais-lorawan)
cmake_minimum_required(VERSION 3.6)

#---------------------------------------------------------------------------------------
# 定制射频板，TODO:添加standard板，作为默认
#---------------------------------------------------------------------------------------
set(MBED_RADIO_SHIELD_LIST SX1272MB2DAS SX1276MB1LAS SX1276MB1MAS SX1261MBXBAS SX1262MBXCAS SX1262MBXDAS )
set(MBED_RADIO_SHIELD SX1276MB1MAS CACHE STRING "Default MBED radio shield is SX1276MB1MAS")
set_property(CACHE MBED_RADIO_SHIELD PROPERTY STRINGS ${MBED_RADIO_SHIELD_LIST})

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/lora-sdk)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/example)
