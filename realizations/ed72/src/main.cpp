#include "pico/stdlib.h"
#include <bsp/board_api.h>
#include <tusb.h>

#include "../../../lib/MUDDC/include/MUDDC/MainController.h"
// #include "pico/multicore.h"

enum Switches {
    UnlockOverRelay = 0,
    LeftDoorIndividualOpen = 1,
    LeftDoorCentralOpen = 2,
    LeftDoorCentralClose = 3,
    DoorBellOpen = 4,
    RightDoorCentralOpen = 5,
    RightDoorCentralClose = 6,
    RightDoorIndividualOpen = 7,
    TransformerOffAndUnlocked = 8,
    TransformerOn = 9,
    ClearShpWatchman = 10,
    RadioTelephone = 11,

};

int main() {
    board_init();
    tusb_init();

    MainController mc({i2c0, 12, 13, 100 * 1000}, {i2c1, 14, 15, 400 * 1000});

    mc.addMainDeviceGpioInput(4, UnlockOverRelay);
    mc.addMainDeviceGpioInput(5, LeftDoorIndividualOpen);
    mc.addMainDeviceGpioInput(6, LeftDoorCentralOpen);
    mc.addMainDeviceGpioInput(7, LeftDoorCentralClose);
    mc.addMainDeviceGpioInput(8, DoorBellOpen);
    mc.addMainDeviceGpioInput(9, RightDoorCentralClose);
    mc.addMainDeviceGpioInput(10, RightDoorCentralOpen);
    mc.addMainDeviceGpioInput(11, RightDoorIndividualOpen);
    mc.addMainDeviceGpioInput(2, TransformerOffAndUnlocked);
    mc.addMainDeviceGpioInput(3, TransformerOn);
    mc.addMainDeviceGpioInput(16, ClearShpWatchman);
    mc.addMainDeviceGpioInput(17, RadioTelephone);

    MainController::ExpanderEndpoint mainControllerExpander{0x48,
        {true, true, true, true, true, true, true, true}
    };
    mainControllerExpander.direction = MainController::ExpanderDir::Read;
    mc.addI2cExpander(mainControllerExpander);

    // MainController::PicoExpander rp1Expander {0x20,
    //     {false, false, false},
    //     {false},
    //     {},
    //     MainController::ExpanderDir::Write
    // };

    {
        MainController::PwmEndpoint engineCurrent(18, mc.accessDatagramIn().hvCurrent1(), {});
        mc.addMainDevicePwm(engineCurrent);

        MainController::PwmEndpoint hvVoltage(19, mc.accessDatagramIn().hvVoltage(), {});
        mc.addMainDevicePwm(hvVoltage);

        MainController::PwmEndpoint lvVoltage(21, mc.accessDatagramIn().lvVoltage(), {});
        mc.addMainDevicePwm(lvVoltage);

        MainController::PwmEndpoint lvCurrent(20, mc.accessDatagramIn().lvVoltage(), {});
        mc.addMainDevicePwm(hvVoltage);
        //TODO make if battery on correct current, if off 0
    }

    mc.initialize();

    auto postTaskFun = [&mc]() {

    };

    // ReSharper disable once CppDFAEndlessLoop
    while (true) {
        tud_task();
        mc.blinkTask();
        mc.cdcTask();

        sleep_ms(1);
    }
    return 0;
}

//TODO if disconnected keep values or set to 0?