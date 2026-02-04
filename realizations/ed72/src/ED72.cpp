//
// Created by Filip on 22.01.2026.
//

#include "ED72.h"
#include "MUDDC/BaseDevicesAndEndpoint.h"
#include <bsp/board_api.h>

ED72::ED72(const bde::I2cDevice &expanderI2cline, const bde::I2cDevice &communicationI2CLine) : BaseController(
    expanderI2cline, communicationI2CLine) {
    configureMasterDevice();
    configureExpanders();

    ED72::setup();
}

void ED72::setup() {
    // expanders.emplace_back(&ex0);
    // expanders.emplace_back(&ex1);
    expanders.emplace_back(&ex2);

    piPicoExpanders.emplace_back(&rp1);
}

void ED72::postTransmissionTask() {
}

void ED72::postReceiveAndReadDevTask() {
    simulateBatteryCurrent();
    masterController();
    directionController();
}

void ED72::initialize() {
    BaseController::initialize();

    // Set base value
    uint8_t zeros = 0;
    // auto ret = i2c_write_timeout_us(expanderI2cLine.device, ex2.address, &zeros, 1, false, 20);
}

void ED72::configureMasterDevice() {
    addMasterDeviceGpioInput(2, *DatOutSw::ClearShpWatchman);
    addMasterDeviceGpioInput(3, *DatOutSw::ConverterOn);
    addMasterDeviceGpioInput(4, *DatOutSw::ConverterOffAndUnlocked);
    addMasterDeviceGpioInput(5, *DatOutSw::RightDoorIndividualOpen);
    addMasterDeviceGpioInput(6, *DatOutSw::RightDoorCentralOpen);
    addMasterDeviceGpioInput(7, *DatOutSw::RightDoorCentralClose);
    addMasterDeviceGpioInput(8, *DatOutSw::DoorBell);
    addMasterDeviceGpioInput(9, *DatOutSw::LeftDoorCentralClose);
    addMasterDeviceGpioInput(10, *DatOutSw::LeftDoorCentralOpen);
    addMasterDeviceGpioInput(11, *DatOutSw::LeftDoorIndividualOpen);
    addMasterDeviceGpioInput(16, *DatOutSw::MainCircuitBreaker);
    addMasterDeviceGpioInput(17, *DatOutSw::UnlockOverloadRelay);
    addMasterDeviceGpioInput(22, *DatOutSw::DirCntr_Wk6, true);

    addMasterDevicePwm(18, datagramIn.hvCurrent1(), {});
    addMasterDevicePwm(19, datagramIn.hvVoltage(), {});
    addMasterDevicePwm(20, VarPtrProxy<uint16_t>(reinterpret_cast<uint8_t *>(&lvCurrent)), {});
    addMasterDevicePwm(21, datagramIn.lvVoltage(), {});
}

void ED72::configureExpanders() {
    rp1 = bde::PiPicoExpanderEp(0x18, 0b1111'1111'1111'1111'1111'1111, 0, {
                                    *DatOutSw::LeftLamp,
                                    *DatOutSw::RightLamp,
                                    *DatOutSw::TopLamp,
                                    *DatOutSw::LeftRedLamp,
                                    *DatOutSw::RightRedLamp,
                                    *DatOutSw::BoardLighting,
                                    *DatOutSw::CabinLightingI,
                                    *DatOutSw::CabinLightingIi,
                                },
                                bde::CommunicationDir::Read);

    ex2 = bde::ExpanderEp_8(0x22, 0b1111'1111, 0xff,
                            {
                                *DatOutSw::MasterCtrl_P, *DatOutSw::MasterCtrl_S, *DatOutSw::MasterCtrl_R,
                                *DatOutSw::MasterCtrl_B1, *DatOutSw::MasterCtrl_B2, *DatOutSw::MasterCtrl_B3,
                                *DatOutSw::DirCntr_Wk7, *DatOutSw::DirCntr_Wk9
                            },
                            bde::CommunicationDir::Read);

}

void ED72::masterController() {
    bool pozP = ex2.getValueOnDataBit(*DatOutSw::MasterCtrl_P);
    bool pozS = ex2.getValueOnDataBit(*DatOutSw::MasterCtrl_S);
    bool pozR = ex2.getValueOnDataBit(*DatOutSw::MasterCtrl_R);
    bool pozB1 = ex2.getValueOnDataBit(*DatOutSw::MasterCtrl_B1);
    bool pozB2 = ex2.getValueOnDataBit(*DatOutSw::MasterCtrl_B2);
    bool pozB3 = ex2.getValueOnDataBit(*DatOutSw::MasterCtrl_B3);

    uint8_t checkSum = pozP + pozS + pozR + pozB1 + pozB2 + pozB3;

    if (pozB3 && checkSum == 6) {
        datagramOut.setMasterController(255);
    } else if (pozB2 && checkSum == 5) {
        datagramOut.setMasterController(212);
    } else if (pozB1 && checkSum == 4) {
        datagramOut.setMasterController(170);
    } else if (pozR && checkSum == 3) {
        datagramOut.setMasterController(127);
    } else if (pozS && checkSum == 2) {
        datagramOut.setMasterController(85);
    } else if (pozP && checkSum == 1) {
        datagramOut.setMasterController(42);
    } else {
        datagramOut.setMasterController(0);
    }
}

void ED72::directionController() {
    bool wk6 = std::ranges::find_if(masterDeviceInputs, [](const bde::GpioEp &input) {
        return input.bit == *DatOutSw::DirCntr_Wk6;
    })->value;

    bool wk7 = ex2.getValueOnDataBit(*DatOutSw::DirCntr_Wk7);

    bool wk9 = ex2.getValueOnDataBit(*DatOutSw::DirCntr_Wk9);

    std::string tmp = "DirCtrl: " + std::to_string(wk6) + ", " + std::to_string(wk7) + ", " + std::to_string(wk9) + "\n\r";
    tud_cdc_n_write(1, tmp.c_str(), tmp.size());

    if (!wk6 && wk7 && wk9) {
        datagramOut.setSwitchState(*DatOutSw::ReverserBackward, true);
        datagramOut.setSwitchState(*DatOutSw::ReverserForward, false);
        datagramOut.setSwitchState(*DatOutSw::ReverserForwardI, false);
    } else if (!wk6 && !wk7 && wk9) {
        // Pos off
        datagramOut.setSwitchState(*DatOutSw::ReverserBackward, false);
        datagramOut.setSwitchState(*DatOutSw::ReverserForward, false);
        datagramOut.setSwitchState(*DatOutSw::ReverserForwardI, false);
    } else if (wk6 && wk7 && wk9) {
        datagramOut.setSwitchState(*DatOutSw::ReverserBackward, false);
        datagramOut.setSwitchState(*DatOutSw::ReverserForward, true);
        datagramOut.setSwitchState(*DatOutSw::ReverserForwardI, false);
    } else if (wk6 && !wk7 && wk9) {
        datagramOut.setSwitchState(*DatOutSw::ReverserBackward, false);
        datagramOut.setSwitchState(*DatOutSw::ReverserForward, false);
        datagramOut.setSwitchState(*DatOutSw::ReverserForwardI, true);
    } else {
        // Pos 0
        datagramOut.setSwitchState(*DatOutSw::ReverserBackward, false);
        datagramOut.setSwitchState(*DatOutSw::ReverserForward, false);
        datagramOut.setSwitchState(*DatOutSw::ReverserForwardI, false);
    }
}

void ED72::simulateBatteryCurrent() {
    if (datagramIn.indicatorState(DatagramIn::Battery)) {
        lvCurrent = bde::PwmEp::fromPercentage(10);
    } else {
        lvCurrent = 0;
    }
}
