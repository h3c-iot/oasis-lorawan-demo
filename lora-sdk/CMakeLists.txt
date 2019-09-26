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
project(lora-sdk)
cmake_minimum_required(VERSION 3.6)


add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/loramac)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/system)

# 后续增加功能后需要做编译裁剪
#add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/3rd_board)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/apps)

# 后续增加功能后需要做编译裁剪
#add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/lorawan_adapt)


