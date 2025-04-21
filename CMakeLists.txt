# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Initialise pico_sdk from installed location
# (note this can come from environment, CMake cache etc)

set(PICO_BOARD pico_w CACHE STRING "Board type")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(ProjetoRevisao C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Add executable. Default name is the project name, version 0.1

add_executable(ProjetoRevisao main.c libs/ssd1306.c libs/led_matrix.c)

pico_set_program_name(ProjetoRevisao "ProjetoRevisao")
pico_set_program_version(ProjetoRevisao "0.1")

# Adicionando o arquivo PIO
pico_generate_pio_header(ProjetoRevisao ${CMAKE_CURRENT_LIST_DIR}/ws2812.pio OUTPUT_DIR ${CMAKE_CURRENT_LIST_DIR}/generated)

# Modify the below lines to enable/disable output over UART/USB
pico_enable_stdio_uart(ProjetoRevisao 1)
pico_enable_stdio_usb(ProjetoRevisao 1)

# Add the standard library to the build
target_link_libraries(ProjetoRevisao
        pico_stdlib
        hardware_pwm
        hardware_adc
        hardware_i2c
        pico_rand
        hardware_pio
        hardware_clocks
        )

# Add the standard include files to the build
target_include_directories(ProjetoRevisao PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
)

# Add any user requested libraries
target_link_libraries(ProjetoRevisao 
        ${CMAKE_CURRENT_LIST_DIR}/libs/structs.h
        )

pico_add_extra_outputs(ProjetoRevisao)

