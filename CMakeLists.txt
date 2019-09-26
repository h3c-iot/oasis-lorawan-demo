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


add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/lora-sdk)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/example)
