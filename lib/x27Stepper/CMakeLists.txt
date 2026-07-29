cmake_minimum_required(VERSION 3.16)

set(CMAKE_CXX_STANDARD 17)

project(x27stepper)

add_library(x27stepper
        src/X27StepperDriver.cpp
        include/X27StepperDriver.h)

target_include_directories(x27stepper PUBLIC include)

target_link_libraries(x27stepper PUBLIC pico_stdlib hardware_pwm)

#pico_add_extra_outputs(x27stepper)`