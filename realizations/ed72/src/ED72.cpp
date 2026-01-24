//
// Created by Filip on 22.01.2026.
//

#include "ED72.h"
#include "MUDDC/BaseDevicesAndEndpoint.h"

ED72::ED72(const bde::I2cDevice &expanderI2cline, const bde::I2cDevice &communicationI2CLine) : BaseController(
    expanderI2cline, communicationI2CLine) {
    configureMasterDevice();
    configureExpanders();

    ED72::setup();
}

void ED72::setup() {
    expanders.emplace_back(&ex0);
    expanders.emplace_back(&ex1);
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

void ED72::configureMasterDevice() {
    // addMasterDeviceGpioInput(4, *DatOutSw::UnlockOverRelay);
    // addMasterDeviceGpioInput(5, *DatOutSw::LeftDoorIndividualOpen);
    // addMasterDeviceGpioInput(6, *DatOutSw::LeftDoorCentralOpen);
    // addMasterDeviceGpioInput(7, *DatOutSw::LeftDoorCentralClose);
    // addMasterDeviceGpioInput(8, *DatOutSw::DoorBellOpen);
    // addMasterDeviceGpioInput(9, *DatOutSw::RightDoorCentralClose);
    // addMasterDeviceGpioInput(10, *DatOutSw::RightDoorCentralOpen);
    // addMasterDeviceGpioInput(11, *DatOutSw::RightDoorIndividualOpen);
    // addMasterDeviceGpioInput(2, *DatOutSw::TransformerOffAndUnlocked);
    // addMasterDeviceGpioInput(3, *DatOutSw::TransformerOn);
    // addMasterDeviceGpioInput(16, *DatOutSw::ClearShpWatchman);
    // addMasterDeviceGpioInput(17, *DatOutSw::RadioTelephone);
    addMasterDeviceGpioInput(22, *DatOutSw::DirCntr_Wk6);

    addMasterDevicePwm(18, datagramIn.hvCurrent1(), {});
    addMasterDevicePwm(19, datagramIn.hvVoltage(), {});
    addMasterDevicePwm(20, VarPtrProxy<uint16_t>(reinterpret_cast<uint8_t *>(&lvCurrent)), {});
    addMasterDevicePwm(21, datagramIn.lvVoltage(), {});
}

void ED72::configureExpanders() {
    rp1 = bde::PiPicoExpanderEp(0x20, 0b1111'1111'1111'1111'1111'1111, 0, {
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

    ex2 = bde::ExpanderEp_8(0x44, 0b1111'1111, 0b0000'0000,
                            {
                                *DatOutSw::MasterCtrl_P, *DatOutSw::MasterCtrl_S, *DatOutSw::MasterCtrl_R,
                                *DatOutSw::MasterCtrl_B1, *DatOutSw::MasterCtrl_B2, *DatOutSw::MasterCtrl_B3,
                                *DatOutSw::DirCntr_Wk7, *DatOutSw::DirCntr_Wk7
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

    if (checkSum == 0) {
        datagramOut.setMasterController(0);
    } else if (checkSum == 1) {
        if (pozP) {
            datagramOut.setMasterController(42);
        } else if (pozS) {
            datagramOut.setMasterController(85);
        } else if (pozR) {
            datagramOut.setMasterController(127);
        } else if (pozB1) {
            datagramOut.setMasterController(170);
        } else if (pozB2) {
            datagramOut.setMasterController(212);
        } else {
            datagramOut.setMasterController(255);
        }
    } else {
        datagramOut.setMasterController(0);
    }
}

void ED72::directionController() {
    auto wk6 = std::ranges::find_if(masterDeviceInputs, [](const bde::GpioEp &input) {
        return input.bit == *DatOutSw::DirCntr_Wk6;
    })->value;

    auto wk7 = std::ranges::find_if(masterDeviceInputs, [](const bde::GpioEp &input) {
        return input.bit == *DatOutSw::DirCntr_Wk7;
    })->value;

    auto wk9 = std::ranges::find_if(masterDeviceInputs, [](const bde::GpioEp &input) {
        return input.bit == *DatOutSw::DirCntr_Wk9;
    })->value;

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
        lvCurrent = bde::PwmEp::fromPercentage(50);
    } else {
        lvCurrent = 0;
    }
}
