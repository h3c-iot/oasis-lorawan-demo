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
project(lora-sdk)
cmake_minimum_required(VERSION 3.6)


add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/lorawan)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/net)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/system)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/apps)



add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/middleware)
