cmake_minimum_required(VERSION 3.6)
set(PICO_BOARD pico_w)
set(PICO_SDK_FETCH_FROM_GIT ON)
# Pull in SDK (must be before project)
include(pico_sdk_import.cmake)
include(FetchContent)

FetchContent_Declare(
  FreeRTOS
  GIT_REPOSITORY https://github.com/FreeRTOS/FreeRTOS-Kernel
  GIT_TAG        V11.0.1
  SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/lib/FreeRTOS
)

FetchContent_Declare(
    u8g2
    GIT_REPOSITORY https://github.com/olikraus/u8g2
    GIT_TAG c4f9cd9 # Version 2.35.9
    SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/lib/u8g2
)

project(MIC5_Final_assignment C CXX ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

if (PICO_SDK_VERSION_STRING VERSION_LESS "1.3.0")
    message(FATAL_ERROR "Raspberry Pi Pico SDK version 1.3.0 (or later) required. Your version is ${PICO_SDK_VERSION_STRING}")
endif ()

# Initialize the SDK
pico_sdk_init()



add_compile_options(-Wall
        -Wno-format          # int != int32_t as far as the compiler is concerned because gcc has int32_t as long int
        -Wno-unused-function # we have some for the docs that aren't called
        -Wno-maybe-uninitialized
        -ffunction-sections 
        -fdata-sections
)

FetchContent_MakeAvailable(u8g2)

if (PICO_CYW43_SUPPORTED) # set by BOARD=pico-w
    if (NOT TARGET pico_cyw43_arch)
        message("Skipping build as support is not available")
    else ()
        add_executable(people_counter "src/main.c")
        # enable usb output, disable uart output
        pico_enable_stdio_usb(people_counter 1)
        pico_enable_stdio_uart(people_counter 0)

        target_include_directories(people_counter PUBLIC
                ${CMAKE_CURRENT_LIST_DIR}/src
                ${CMAKE_CURRENT_LIST_DIR}/src/display
        )

        # Select the heap port.  values between 1-4 will pick a heap.
        set(FREERTOS_HEAP "4" CACHE STRING "" FORCE)

        # Select the native compile PORT
        set(FREERTOS_PORT "GCC_ARM_CM0" CACHE STRING "" FORCE)
        add_library(freertos_config INTERFACE)
        FetchContent_MakeAvailable(FreeRTOS)
        target_include_directories(freertos_config INTERFACE "config/")
        target_link_libraries(people_counter 
        pico_cyw43_arch_lwip_poll
        pico_lwip_mbedtls
        pico_mbedtls
        pico_stdlib
        freertos_config
        hardware_pwm
        hardware_spi
        freertos_kernel
        u8g2)
        pico_add_extra_outputs(people_counter)
        add_link_options(--print-memory-usage)
    endif ()
endif()
