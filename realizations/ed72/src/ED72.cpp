//
// Created by Filip on 22.01.2026.
//

#include "ED72.h"
#include "MUDDC/BaseDevicesAndEndpoint.h"

ED72::ED72(const bde::I2cDevice& expanderI2cline, const bde::I2cDevice& communicationI2CLine) : BaseController(
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
    if (datagramIn.indicatorState(DatagramIn::Battery)) {
        lvCurrent = bde::PwmEp::fromPercentage(50);
    } else {
        lvCurrent = 0;
    }
}

void ED72::configureMasterDevice() {
    addMasterDeviceGpioInput(4, *DatOutSw::UnlockOverRelay);
    addMasterDeviceGpioInput(5, *DatOutSw::LeftDoorIndividualOpen);
    addMasterDeviceGpioInput(6, *DatOutSw::LeftDoorCentralOpen);
    addMasterDeviceGpioInput(7, *DatOutSw::LeftDoorCentralClose);
    addMasterDeviceGpioInput(8, *DatOutSw::DoorBellOpen);
    addMasterDeviceGpioInput(9, *DatOutSw::RightDoorCentralClose);
    addMasterDeviceGpioInput(10, *DatOutSw::RightDoorCentralOpen);
    addMasterDeviceGpioInput(11, *DatOutSw::RightDoorIndividualOpen);
    addMasterDeviceGpioInput(2, *DatOutSw::TransformerOffAndUnlocked);
    addMasterDeviceGpioInput(3, *DatOutSw::TransformerOn);
    addMasterDeviceGpioInput(16, *DatOutSw::ClearShpWatchman);
    addMasterDeviceGpioInput(17, *DatOutSw::RadioTelephone);
    addMasterDeviceGpioInput(22, *DatOutSw::DirCntr_Wk6);

    addMasterDevicePwm(18, datagramIn.hvCurrent1(), {});
    addMasterDevicePwm(19, datagramIn.hvVoltage(), {});
    addMasterDevicePwm(20, VarPtrProxy<uint16_t>(reinterpret_cast<uint8_t*>(&lvCurrent)), {});
    addMasterDevicePwm(21, datagramIn.lvVoltage(), {});
}

void ED72::configureExpanders() {
    // rp1 = bde::PiPicoExpanderEp(0x48);
    ex2 = bde::ExpanderEp_8(0x44, 0b1111'1111, 0b0000'0000,
                            {-1, -1, -1, -1, -1, -1, bde::u(DatOutSw::DirCntr_Wk7), bde::u(DatOutSw::DirCntr_Wk7)},
                            bde::CommunicationDir::Read);
}
