cmake_minimum_required(VERSION 3.10)
project(TMS C)

set(CMAKE_C_STANDARD 11)

# POSIX 기능 매크로 정의 (clock_gettime, localtime_r 등)
add_definitions(-D_POSIX_C_SOURCE=200809L)

# Backend/Platform options
option(PLATFORM_RPI "Target Raspberry Pi" ON)
option(USE_BCM2835 "Use bcm2835 library backend" OFF)
option(USE_DEV_SPI "Use /dev/spidev backend" ON)

if(USE_BCM2835 AND USE_DEV_SPI)
    message(FATAL_ERROR "Enable only one of USE_BCM2835 or USE_DEV_SPI")
endif()

# 빌드 출력 디렉토리 지정
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/build/bin)

# include 디렉토리 추가
include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/src/core
    ${CMAKE_SOURCE_DIR}/src/hw
    ${CMAKE_SOURCE_DIR}/src/net
    ${CMAKE_SOURCE_DIR}/src/util
    ${CMAKE_SOURCE_DIR}/lib/ads1263/Config
    ${CMAKE_SOURCE_DIR}/lib/ads1263/Driver
    ${CMAKE_SOURCE_DIR}/lib/bcm2835
)

# 소스 파일 수집
file(GLOB CORE_SRC ${CMAKE_SOURCE_DIR}/src/core/*.c)
file(GLOB HW_SRC   ${CMAKE_SOURCE_DIR}/src/hw/*.c)
file(GLOB NET_SRC  ${CMAKE_SOURCE_DIR}/src/net/*.c)
file(GLOB UTIL_SRC ${CMAKE_SOURCE_DIR}/src/util/*.c)

# ADS1263 소스: RPi 전용 sysfs_gpio 사용
set(ADS1263_SRC
    ${CMAKE_SOURCE_DIR}/lib/ads1263/Config/DEV_Config.c
    ${CMAKE_SOURCE_DIR}/lib/ads1263/Config/dev_hardware_SPI.c
    ${CMAKE_SOURCE_DIR}/lib/ads1263/Config/sysfs_software_spi.c
    ${CMAKE_SOURCE_DIR}/lib/ads1263/Config/RPI_sysfs_gpio.c
    ${CMAKE_SOURCE_DIR}/lib/ads1263/Driver/ADS1263.c
)

# 실행 파일
add_executable(tms
    ${CORE_SRC}
    ${HW_SRC}
    ${NET_SRC}
    ${UTIL_SRC}
    ${ADS1263_SRC}
)

# Define target macros based on selected backend/platform
if(PLATFORM_RPI)
    target_compile_definitions(tms PRIVATE RPI)
endif()

if(USE_BCM2835)
    target_compile_definitions(tms PRIVATE USE_BCM2835_LIB)
    target_sources(tms PRIVATE ${CMAKE_SOURCE_DIR}/lib/bcm2835/bcm2835.c)
elseif(USE_DEV_SPI)
    target_compile_definitions(tms PRIVATE USE_DEV_LIB)
endif()

# 필요한 라이브러리 링크
target_link_libraries(tms websockets)

# ===============================
# Raspberry Pi 최적화 옵션 추가
# ===============================
if(PLATFORM_RPI)
    target_compile_options(tms PRIVATE
        -O2                 # 안정성과 성능 균형 (O3보다 보수적)
        -march=armv8-a      # ARMv8-A ISA 사용
        -mtune=cortex-a72   # RPi4 CPU에 최적화
        -fomit-frame-pointer # 약간의 성능 향상, 큰 안정성 리스크 없음
    )
endif()