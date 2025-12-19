cmake_minimum_required(VERSION 3.10)
project(TMS C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_EXTENSIONS ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/build/bin)

# ===============================
# 소스 파일 수집
# ===============================
file(GLOB CORE_SRC ${CMAKE_SOURCE_DIR}/src/core/*.c)
file(GLOB HW_SRC   ${CMAKE_SOURCE_DIR}/src/hw/*.c)
file(GLOB NET_SRC  ${CMAKE_SOURCE_DIR}/src/net/*.c)
file(GLOB UTIL_SRC ${CMAKE_SOURCE_DIR}/src/util/*.c)

# ===============================
# bcm2835: 내부 정적 라이브러리
# ===============================
add_library(bcm2835 STATIC
    ${CMAKE_SOURCE_DIR}/lib/bcm2835/bcm2835.c
)
target_include_directories(bcm2835 PUBLIC
    ${CMAKE_SOURCE_DIR}/lib/bcm2835
)

# ===============================
# ADS1263: 내부 정적 라이브러리
#   - bcm2835 백엔드 사용 시 sysfs/softspi 불필요
# ===============================
add_library(ads1263 STATIC
    ${CMAKE_SOURCE_DIR}/lib/ads1263/Config/DEV_Config.c
    ${CMAKE_SOURCE_DIR}/lib/ads1263/Config/dev_hardware_SPI.c
    ${CMAKE_SOURCE_DIR}/lib/ads1263/Driver/ADS1263.c
)

target_include_directories(ads1263 PUBLIC
    ${CMAKE_SOURCE_DIR}/lib/ads1263/Config
    ${CMAKE_SOURCE_DIR}/lib/ads1263/Driver
)

# ADS1263가 bcm2835 경로를 타도록 define
target_compile_definitions(ads1263 PRIVATE RPI USE_BCM2835_LIB)

# ADS1263는 bcm2835에 의존
target_link_libraries(ads1263 PRIVATE bcm2835)

# ===============================
# 실행 파일
# ===============================
add_executable(tms
    ${CORE_SRC}
    ${HW_SRC}
    ${NET_SRC}
    ${UTIL_SRC}
)

target_include_directories(tms PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/src/core
    ${CMAKE_SOURCE_DIR}/src/hw
    ${CMAKE_SOURCE_DIR}/src/net
    ${CMAKE_SOURCE_DIR}/src/util
)

# 필요한 라이브러리 링크
target_link_libraries(tms PRIVATE
    websockets
    ads1263
    m
)
