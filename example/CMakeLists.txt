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
project(example)
cmake_minimum_required(VERSION 3.6)

#---------------------------------------------------------------------------------------
# 定制开发板
#---------------------------------------------------------------------------------------
# 目前只支持NucleoL073开发板
set(BOARD_LIST NucleoL073)
set(BOARD NucleoL073 CACHE STRING "Default target platform is NucleoL073")
set_property(CACHE BOARD PROPERTY STRINGS ${BOARD_LIST})

if(BOARD STREQUAL NucleoL073)
	add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/stm32/NucleoL073)
endif()


