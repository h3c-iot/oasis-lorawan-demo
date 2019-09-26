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
project(mac)
cmake_minimum_required(VERSION 3.6)

#---------------------------------------------------------------------------------------
# 定制是否支持ClassB, 暂不支持
#---------------------------------------------------------------------------------------
option(CLASSB_ENABLED "Class B support of LoRaMac" OFF)

#---------------------------------------------------------------------------------------
# 定制支持的国家码, 暂时只支持CN470
#---------------------------------------------------------------------------------------
option(REGION_EU868 "Region EU868" OFF)
option(REGION_US915 "Region US915" OFF)
option(REGION_CN779 "Region CN779" OFF)
option(REGION_EU433 "Region EU433" OFF)
option(REGION_AU915 "Region AU915" OFF)
option(REGION_AS923 "Region AS923" OFF)
option(REGION_CN470 "Region CN470" ON)
option(REGION_KR920 "Region KR920" OFF)
option(REGION_IN865 "Region IN865" OFF)
option(REGION_RU864 "Region RU864" OFF)
set(REGION_LIST REGION_EU868 REGION_US915 REGION_CN779 REGION_EU433 REGION_AU915 REGION_AS923 REGION_CN470 REGION_KR920 REGION_IN865 REGION_RU864)

#---------------------------------------------------------------------------------------
# 定制当前工作的国家码, 暂时只支持CN470
#---------------------------------------------------------------------------------------
set(ACTIVE_REGION_LIST   LORAMAC_REGION_EU868 LORAMAC_REGION_US915 LORAMAC_REGION_CN779
    LORAMAC_REGION_EU433 LORAMAC_REGION_AU915 LORAMAC_REGION_AS923 LORAMAC_REGION_CN470
    LORAMAC_REGION_KR920 LORAMAC_REGION_IN865 LORAMAC_REGION_RU864
)
set(ACTIVE_REGION LORAMAC_REGION_CN470 CACHE STRING "Default active region is CN470")
set_property(CACHE ACTIVE_REGION PROPERTY STRINGS ${ACTIVE_REGION_LIST})


file(GLOB ${PROJECT_NAME}_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/*.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/region/Region.c"
	"${CMAKE_CURRENT_SOURCE_DIR}/region/RegionCommon.c"
)

# 定制国家码文件，目前不支持多国切换
if(REGION_EU868)
	list(APPEND ${PROJECT_NAME}_SOURCES
		"${CMAKE_CURRENT_SOURCE_DIR}/region/RegionEU868.c"
	)
endif()
if(REGION_US915)
	list(APPEND ${PROJECT_NAME}_SOURCES
		"${CMAKE_CURRENT_SOURCE_DIR}/region/RegionUS915.c"
	)
endif()
if(REGION_CN779)
	list(APPEND ${PROJECT_NAME}_SOURCES
		"${CMAKE_CURRENT_SOURCE_DIR}/region/RegionCN779.c"
	)
endif()
if(REGION_EU433)
	list(APPEND ${PROJECT_NAME}_SOURCES
		"${CMAKE_CURRENT_SOURCE_DIR}/region/RegionEU433.c"
	)
endif()
if(REGION_AU915)
	list(APPEND ${PROJECT_NAME}_SOURCES
		"${CMAKE_CURRENT_SOURCE_DIR}/region/RegionAU915.c"
	)
endif()
if(REGION_AS923)
	list(APPEND ${PROJECT_NAME}_SOURCES
		"${CMAKE_CURRENT_SOURCE_DIR}/region/RegionAS923.c"
	)
endif()
if(REGION_CN470)
	list(APPEND ${PROJECT_NAME}_SOURCES
		"${CMAKE_CURRENT_SOURCE_DIR}/region/RegionCN470.c"
	)
endif()
if(REGION_KR920)
	list(APPEND ${PROJECT_NAME}_SOURCES
		"${CMAKE_CURRENT_SOURCE_DIR}/region/RegionKR920.c"
	)
endif()
if(REGION_IN865)
	list(APPEND ${PROJECT_NAME}_SOURCES
		"${CMAKE_CURRENT_SOURCE_DIR}/region/RegionIN865.c"
	)
endif()
if(REGION_RU864)
	list(APPEND ${PROJECT_NAME}_SOURCES
		"${CMAKE_CURRENT_SOURCE_DIR}/region/RegionRU864.c"
	)
endif()

add_library(${PROJECT_NAME} OBJECT EXCLUDE_FROM_ALL ${${PROJECT_NAME}_SOURCES})

# 定义编译宏
foreach( REGION ${REGION_LIST} )
    if(${REGION})
        target_compile_definitions(${PROJECT_NAME} PUBLIC -D"${REGION}")
    endif()
endforeach()

# 引用头文件
target_include_directories( ${PROJECT_NAME} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/region
	${CMAKE_CURRENT_SOURCE_DIR}/../../system
	${CMAKE_CURRENT_SOURCE_DIR}/../radio
)

# Add define if class B is supported
target_compile_definitions(${PROJECT_NAME} PRIVATE $<$<BOOL:${CLASSB_ENABLED}>:LORAMAC_CLASSB_ENABLED>)

target_compile_definitions(${PROJECT_NAME} PUBLIC ACTIVE_REGION=${ACTIVE_REGION})

set_property(TARGET ${PROJECT_NAME} PROPERTY C_STANDARD 11)
