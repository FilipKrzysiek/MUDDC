
add_library(MUDDC_lib STATIC include/MUDDC/DatagramIn.h src/DatagramIn.cpp
        include/MUDDC/DatagramOut.h src/DatagramOut.cpp
        src/usb_descriptor.cpp
        include/MUDDC/VarPtrProxy.h
        src/BaseController.cpp
        include/MUDDC/BaseController.h
        include/MUDDC/BaseDevicesAndEndpoint.h
        src/BaseDevicesAndEndpoint.cpp
        include/MUDDC/bitFunctions.h
)

target_include_directories(MUDDC_lib PRIVATE include)
target_include_directories(MUDDC_lib PUBLIC src)

target_link_libraries(MUDDC_lib pico_stdlib pico_multicore pico_unique_id  hardware_i2c hardware_pwm tinyusb_device tinyusb_board)


pico_enable_stdio_usb(MUDDC_lib 0)
pico_enable_stdio_uart(MUDDC_lib 0)
