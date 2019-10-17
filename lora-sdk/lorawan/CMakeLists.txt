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
project(ladapter)
cmake_minimum_required(VERSION 3.6)

#if(MBED_RADIO_SHIELD STREQUAL WSL305S)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/3rd_board)
#else()
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/loramac)
#endif()

file(GLOB ${PROJECT_NAME}_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/*.c"
)

add_library(${PROJECT_NAME} OBJECT EXCLUDE_FROM_ALL ${${PROJECT_NAME}_SOURCES})

# 引用头文件
target_include_directories( ${PROJECT_NAME} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}/loramac
	${CMAKE_CURRENT_SOURCE_DIR}/loramac/mac
	${CMAKE_CURRENT_SOURCE_DIR}/loramac/radio
	${CMAKE_CURRENT_SOURCE_DIR}/3rd_board/wsl30x
	${CMAKE_CURRENT_SOURCE_DIR}/../apps/common
	${CMAKE_CURRENT_SOURCE_DIR}/../include
	${CMAKE_CURRENT_SOURCE_DIR}/../system

)

if(MBED_RADIO_SHIELD STREQUAL WSL305S)
    target_compile_definitions(${PROJECT_NAME} PUBLIC -DLORA_MODULE_WSL30X)
endif()
