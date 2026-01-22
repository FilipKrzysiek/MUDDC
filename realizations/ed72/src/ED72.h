//
// Created by Filip on 22.01.2026.
//

#ifndef MUDDC_ED72_H
#define MUDDC_ED72_H
#include "MUDDC/BaseController.h"


class ED72: public BaseController {
public:
    ED72(const bde::I2cDevice& expanderI2cline, const bde::I2cDevice& communicationI2CLine);

    void setup() override;
    void postTransmissionTask() override;
    void postReceiveAndReadDevTask() override;

    enum class DatOutSw: bde::datBit_t {
        DirCntr_Wk9 = -4,
        DirCntr_Wk7 = -3,
        DirCntr_Wk6 = -2,
        Undefined = -1,
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

private:
    bde::ExpanderEp_8 ex0;
    bde::ExpanderEp_8 ex1;
    bde::ExpanderEp_8 ex2;
    bde::PiPicoExpanderEp rp1;

    uint16_t lvCurrent = 0;

    void configureMasterDevice();
    void configureExpanders();
};

constexpr bde::datBit_t operator*(ED72::DatOutSw enumValue) {
    return static_cast<std::underlying_type_t<ED72::DatOutSw>>(enumValue);
}


#endif //MUDDC_ED72_H