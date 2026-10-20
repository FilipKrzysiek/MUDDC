

add_executable(MUDDC_ed72 src/main.cpp
        src/ED72.cpp
        src/ED72.h
)

set(MUDDC_ed72_VERSION 0.0.10)

set_target_properties(MUDDC_ed72 PROPERTIES VERSION ${MUDDC_ed72_VERSION})

target_link_libraries(MUDDC_ed72 pico_stdlib)
target_link_libraries(MUDDC_ed72 hardware_adc)
target_link_libraries(MUDDC_ed72 MUDDC_lib)

target_include_directories(MUDDC_ed72 PUBLIC ./../../lib/MUDDC/include)
target_include_directories(MUDDC_ed72 PRIVATE src)

set(MUDDC_ed72_fileName "MUDDC_ed72-${MUDDC_ed72_VERSION}")

set_target_properties(MUDDC_ed72 PROPERTIES OUTPUT_NAME "${MUDDC_ed72_fileName}")

pico_enable_stdio_usb(MUDDC_ed72 0)
pico_enable_stdio_uart(MUDDC_ed72 0)

pico_add_extra_outputs(MUDDC_ed72)

# ----- Add submodules

add_subdirectory(modules/manometers)

# ----- Build Pi Pico expander build option

include(ExternalProject)

set(MUDDC_ed72_rp1_version "0.0.1")

ExternalProject_Add(MUDDC_ed72_rp1
        SOURCE_DIR ${CMAKE_SOURCE_DIR}/lib/PiPicoExpander
        BINARY_DIR "${CMAKE_BINARY_DIR}/rp1"
        CMAKE_ARGS -DSLAVE_ADDR=0x18
                    -DI2C_FREQ=400000
                    "-DCMAKE_RUNTIME_OUTPUT_DIRECTORY:PATH=${MUDDC_ed72_OUTPUT_DIR}"
        BUILD_COMMAND ${CMAKE_COMMAND} --build .
        INSTALL_COMMAND ""
        COMMENT "Building RP1 expander"
)

add_dependencies(MUDDC_ed72 MUDDC_ed72_rp1)

# -- Copy files to bin

set(MUDDC_ed72_OUTPUT_DIR "${CMAKE_BINARY_DIR}/bin/ed72")

add_custom_command(TARGET MUDDC_ed72 POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
        "${CMAKE_CURRENT_BINARY_DIR}/${MUDDC_ed72_fileName}.uf2"
        "${MUDDC_ed72_OUTPUT_DIR}/${MUDDC_ed72_fileName}.uf2"
        COMMENT "Copying Main uf2 file to destination dir"
)

add_custom_command(TARGET MUDDC_ed72 POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
        "${CMAKE_BINARY_DIR}/rp1/PicoExpander.uf2"
        "${MUDDC_ed72_OUTPUT_DIR}/PicoExpander-rp1-${MUDDC_ed72_rp1_version}.uf2"
        COMMENT "Copying RPI expander uf2 file to destination dir"
)


