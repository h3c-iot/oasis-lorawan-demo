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
project(loramac)
cmake_minimum_required(VERSION 3.6)

file(GLOB ${PROJECT_NAME}_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/soft-se/*.c"
)

#if(MBED_RADIO_SHIELD STREQUAL SX1278ACSIPS78F OR MBED_RADIO_SHIELD STREQUAL SX1278H3C)
list(APPEND ${PROJECT_NAME}_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/loramac_vat.c")
#endif()

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/mac)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/radio)

add_library(${PROJECT_NAME} OBJECT EXCLUDE_FROM_ALL ${${PROJECT_NAME}_SOURCES})

# 引用头文件
target_include_directories( ${PROJECT_NAME} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}/mac
	${CMAKE_CURRENT_SOURCE_DIR}/radio
	${CMAKE_CURRENT_SOURCE_DIR}/../../system
)

target_compile_definitions(${PROJECT_NAME}  PUBLIC
		$<BUILD_INTERFACE:$<TARGET_PROPERTY:mac,INTERFACE_COMPILE_DEFINITIONS>>
)

if(MBED_RADIO_SHIELD STREQUAL WSL305S)
    target_compile_definitions(${PROJECT_NAME} PUBLIC -DLORA_MODULE_WSL30X)
endif()

add_dependencies(${PROJECT_NAME} mac)
