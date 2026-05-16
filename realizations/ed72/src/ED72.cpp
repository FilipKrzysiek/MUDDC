//
// Created by Filip on 22.01.2026.
//

#include "ED72.h"
#include "MUDDC/BaseDevicesAndEndpoint.h"
#include <bsp/board_api.h>
#include "hardware/adc.h"

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
    processBrakeValve();
    processLamps();
}

void ED72::initialize() {
    BaseController::initialize();

    adc_init();

    adc_gpio_init(28);
    adc_select_input(2);
}

void ED72::configureMasterDevice() {
    addMasterDeviceGpioInput(2, *DatOutSw::ClearShpWatchman, true);
    addMasterDeviceGpioInput(3, *DatOutSw::ConverterOn, true);
    addMasterDeviceGpioInput(4, *DatOutSw::ConverterOffAndUnlocked, true);
    addMasterDeviceGpioInput(5, *DatOutSw::RightDoorIndividualOpen, true);
    addMasterDeviceGpioInput(6, *DatOutSw::RightDoorCentralOpen, true);
    addMasterDeviceGpioInput(7, *DatOutSw::RightDoorCentralClose, true);
    addMasterDeviceGpioInput(8, *DatOutSw::DoorBell, true);
    addMasterDeviceGpioInput(9, *DatOutSw::LeftDoorCentralClose, true);
    addMasterDeviceGpioInput(10, *DatOutSw::LeftDoorCentralOpen, true);
    addMasterDeviceGpioInput(11, *DatOutSw::LeftDoorIndividualOpen, true);
    addMasterDeviceGpioInput(16, *DatOutSw::MainCircuitBreaker, true);
    addMasterDeviceGpioInput(17, *DatOutSw::UnlockOverloadRelay, true);
    addMasterDeviceGpioInput(22, *DatOutSw::DirCntr_Wk6, true);

    addMasterDevicePwm(18, datagramIn.hvCurrent1(), {});
    addMasterDevicePwm(19, datagramIn.hvVoltage(), {});
    addMasterDevicePwm(20, VarPtrProxy<uint16_t>(reinterpret_cast<uint8_t *>(&lvCurrent)), {});
    addMasterDevicePwm(21, datagramIn.lvVoltage(), {});
}

void ED72::configureExpanders() {
    rp1 = bde::PiPicoExpanderEp(0x18, 0b1111'1111'1111'1111'1111'1111, 0b1111'1111'1111'1111'1111'1111, {
                                    *DatOutSw::LeftLamp,
                                    *DatOutSw::RightLamp,
                                    *DatOutSw::TopLamp,
                                    *DatOutSw::LeftRedLamp,
                                    *DatOutSw::RightRedLamp,
                                    *DatOutSw::BoardLighting,
                                    *DatOutSw::CabinLightingI,
                                    *DatOutSw::CabinLightingIi,
                                    *DatOutSw::EnabledInteriorLighting,
                                    *DatOutSw::DisabledInteriorLighting,
                                    *DatOutSw::AllPantographsLoweredDown,
                                    *DatOutSw::Compressor,
                                    *DatOutSw::UnlockCompressor,
                                    *DatOutSw::Buzzer,
                                    *DatOutSw::HeadlightsDimmed,
                                    *DatOutSw::RightHeadlightDimmed,
                                    *DatOutSw::RearPantographRaised,
                                    *DatOutSw::FrontPantographRaised,
                                    *DatOutSw::FrontPantographLowered,
                                    *DatOutSw::Camshaft,
                                    *DatOutSw::SignalLampCheckI,
                                    *DatOutSw::SignalLampCheckIi,
                                    *DatOutSw::SignalLampCheckIii,
                                    *DatOutSw::RadioCommunicationAttachment,
                                },
                                bde::CommunicationDir::Read);

    ex0 = bde::ExpanderEp_8(0x20, 0, 0, {
                                DatagramIn::LineBreaker,
                                DatagramIn::MotorOverload,
                                bde::u(VirtualDatagramIn::DoorBuzzerLamp),
                                bde::u(VirtualDatagramIn::DoorLock),
                                bde::u(VirtualDatagramIn::ConverterOff),
                                DatagramIn::DoorLeftAllowed,
                                DatagramIn::DoorLeftOpened,
                                DatagramIn::DoorRightAllowed,
                            }, bde::CommunicationDir::Write);

    ex1 = bde::ExpanderEp_8(0x21, 0, 0, {
                                DatagramIn::DoorRightOpened,
                                bde::u(VirtualDatagramIn::BrakeVoltageOn),
                                DatagramIn::MotorResistors,
                                bde::u(VirtualDatagramIn::Radiotelephone),
                                DatagramIn::Shp,
                                DatagramIn::RadioStop,
                                DatagramIn::Alerter,
                            }, bde::CommunicationDir::Write);

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
    uint8_t valToSet = 0;

    if (pozB3 && checkSum == 6) {
        valToSet = 6;
    } else if (pozB2 && checkSum == 5) {
        valToSet = 5;
    } else if (pozB1 && checkSum == 4) {
        valToSet = 4;
    } else if (pozR && checkSum == 3) {
        valToSet = 3;
    } else if (pozS && checkSum == 2) {
        valToSet = 2;
    } else if (pozP && checkSum == 1) {
        valToSet = 1;
    }

    // uint8_t newValue = valToSet;
    // //TODO make it better
    // if (valToSet > prevMasterCtrl[0]) {
    //     if (valToSet - prevMasterCtrl[0] > 1) {
    //         newValue = prevMasterCtrl[0];
    //         prevOutside = true;
    //     }
    // } else {
    //     if (prevMasterCtrl[0] - valToSet > 1) {
    //         newValue = prevMasterCtrl[0];
    //         prevOutside = true;
    //     }
    // }
    // uint8_t newValue = std::max(valToSet, std::max(prevMasterCtrl[0], prevMasterCtrl[1]));

    // if (std::abs(valToSet - prevMasterCtrl[0]) < std::abs(valToSet - prevMasterCtrl[1]))

    prevMasterCtrl[1] = prevMasterCtrl[0];
    prevMasterCtrl[0] = valToSet;

    datagramOut.setMasterController(valToSet);
}

void ED72::directionController() {
    bool wk6 = std::ranges::find_if(masterDeviceInputs, [](const bde::GpioEp &input) {
        return input.bit == *DatOutSw::DirCntr_Wk6;
    })->value;

    bool wk7 = ex2.getValueOnDataBit(*DatOutSw::DirCntr_Wk7);

    bool wk9 = ex2.getValueOnDataBit(*DatOutSw::DirCntr_Wk9);

    std::string tmp = "DirCtrl: " + std::to_string(wk6) + ", " + std::to_string(wk7) + ", " + std::to_string(wk9) +
                      "\n\r";
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
    if (datagramIn.batteryState()) {
        lvCurrent = bde::PwmEp::fromPercentage(3);
    } else {
        lvCurrent = 0;
    }
}

void ED72::processBrakeValve() {
    uint32_t sum = 0;
    const uint8_t measurments = 10;
    for (uint8_t i = 0; i < measurments; i++) {
        sum += adc_read();
        sleep_us(1);
    }

    datagramOut.setTrainBrake(sum / measurments);
}

void ED72::processLamps() {
    if (datagramIn.batteryState() && rp1.getValueOnDataBit(*DatOutSw::RadioCommunicationAttachment)) {
        datagramIn.setIndicatorState(*VirtualDatagramIn::Radiotelephone, true);
    } else {
        datagramIn.setIndicatorState(*VirtualDatagramIn::Radiotelephone, false);
    }

    if (datagramIn.batteryState() && getValueOnPin(*DatOutSw::DoorBell)) {
        datagramIn.setIndicatorState(*VirtualDatagramIn::DoorBuzzerLamp, true);
    } else {
        datagramIn.setIndicatorState(*VirtualDatagramIn::DoorBuzzerLamp, false);
    }

    if (datagramIn.lvVoltage()() < bde::PwmEp::fromPercentage((110 * 100) / 150) && datagramIn.batteryState()) {
        datagramIn.setIndicatorState(*VirtualDatagramIn::ConverterOff, true);
    } else {
        datagramIn.setIndicatorState(*VirtualDatagramIn::ConverterOff, false);
    }

    if (datagramIn.tacho() > 10 && datagramIn.batteryState()) {
        datagramIn.setIndicatorState(*VirtualDatagramIn::DoorLock, true);
    } else {
        datagramIn.setIndicatorState(*VirtualDatagramIn::DoorLock, false);
    }

    if (datagramIn.batteryState() && (datagramIn.indicatorState(DatagramIn::DirBackward) || datagramIn.indicatorState(
                                          DatagramIn::DirForward))) {
        datagramIn.setIndicatorState(*VirtualDatagramIn::BrakeVoltageOn, true);
    } else {
        datagramIn.setIndicatorState(*VirtualDatagramIn::BrakeVoltageOn, false);
    }
}

//TODO think about keep information about expanders data in one place
