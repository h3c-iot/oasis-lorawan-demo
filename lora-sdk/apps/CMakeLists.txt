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
project(lorawan-apps)
cmake_minimum_required(VERSION 3.6)

file(GLOB ${PROJECT_NAME}_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/common/*.c"
	"${CMAKE_CURRENT_SOURCE_DIR}/common/LmHandler/*.c"
	"${CMAKE_CURRENT_SOURCE_DIR}/common/LmHandler/packages/*.c"
)

add_library(${PROJECT_NAME} OBJECT EXCLUDE_FROM_ALL ${${PROJECT_NAME}_SOURCES})

# 引用头文件
target_include_directories( ${PROJECT_NAME} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/common
	${CMAKE_CURRENT_SOURCE_DIR}/common/LmHandler
	${CMAKE_CURRENT_SOURCE_DIR}/common/LmHandler/packages
	${CMAKE_CURRENT_SOURCE_DIR}/../system
	${CMAKE_CURRENT_SOURCE_DIR}/../include
	${CMAKE_CURRENT_SOURCE_DIR}/../loramac/mac
	${CMAKE_CURRENT_SOURCE_DIR}/../loramac/mac/region
	${CMAKE_CURRENT_SOURCE_DIR}/../loramac/radio
)

target_compile_definitions(${PROJECT_NAME}  PUBLIC
	$<BUILD_INTERFACE:$<TARGET_PROPERTY:mac,INTERFACE_COMPILE_DEFINITIONS>>
)

