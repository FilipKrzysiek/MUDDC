#include "pico/stdlib.h"
#include <bsp/board_api.h>
#include <tusb.h>
#include "ED72.h"

int main() {
    board_init();
    tusb_init();

    ED72 veh({i2c0, 12, 13, 100 * 1000}, {i2c1, 14, 15, 400 * 1000});
    veh.initialize();

    // ReSharper disable once CppDFAEndlessLoop
    while (true) {
        tud_task();
        veh.blinkTask();
        veh.cdcTask();

        sleep_ms(1);
    }
    return 0;
}

//TODO if disconnected keep values or set to 0?