# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.2.0)
set(toolchainVersion 14_2_Rel1)
set(picotoolVersion 2.2.0)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
set(PICO_BOARD pico CACHE STRING "Board type")

set(PICO_NO_COPRO_DIS 1)

cmake_minimum_required(VERSION 3.20)

if (NOT DEFINED PICO_SDK_PATH)
    message("PICO_SDK_PATH not defined!")
    set(PICO_SDK_FETCH_FROM_GIT on)
else()
    message("PICO_SDK_PATH found at ${PICO_SDK_PATH}")
endif ()

if (NOT DEFINED picotool_DIR)
    message("picotool_DIR not defined!")
    set(PICOTOOL_FETCH_FROM_GIT_PATH on)
else()
    message("picotool_DIR found at ${picotool_DIR}")
endif ()

# initialize the SDK based on PICO_SDK_PATH
# note: this must happen before project()
include(pico_sdk_import.cmake)

project(MUDDC)

# initialize the Raspberry Pi Pico SDK
pico_sdk_init()

set(CMAKE_CXX_STANDARD 20)

add_subdirectory(lib/MUDDC)
add_subdirectory(realizations/ed72)
