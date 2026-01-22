

add_executable(MUDDC_ed72 src/main.cpp
        src/ED72.cpp
        src/ED72.h
)

target_link_libraries(MUDDC_ed72 pico_stdlib)
target_link_libraries(MUDDC_ed72 MUDDC_lib)

target_include_directories(MUDDC_ed72 PUBLIC ./../../lib/MUDDC/include)
target_include_directories(MUDDC_ed72 PRIVATE src)

pico_enable_stdio_usb(MUDDC_ed72 0)
pico_enable_stdio_uart(MUDDC_ed72 0)

pico_add_extra_outputs(MUDDC_ed72)