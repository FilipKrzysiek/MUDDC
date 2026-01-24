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
        MasterCtrl_B3 = -10,
        MasterCtrl_B2 = -9,
        MasterCtrl_B1 = -8,
        MasterCtrl_R = -7,
        MasterCtrl_S = -6,
        MasterCtrl_P = -5,
        DirCntr_Wk9 = -4,
        DirCntr_Wk7 = -3,
        DirCntr_Wk6 = -2,
        Undefined = -1,
        LeftLamp = 0,
        RightLamp = 1,
        TopLamp = 2,
        LeftRedLamp = 3,
        RightRedLamp = 4,
        BoardLighting = 5,
        CabinLightingI = 6,
        CabinLightingIi = 7,
        EnabledInteriorLighting = 8,
        DisabledInteriorLighting = 9,
        AllPantographsLoweredDown = 10,
        Compressor = 11,
        UnlockCompressor = 12,
        Buzzer = 13,
        ReverserBackward = 14,
        ReverserForward = 15,
        ReverserForwardI = 16,

        // UnlockOverRelay = 0,
        // LeftDoorIndividualOpen = 1,
        // LeftDoorCentralOpen = 2,
        // LeftDoorCentralClose = 3,
        // DoorBellOpen = 4,
        // RightDoorCentralOpen = 5,
        // RightDoorCentralClose = 6,
        // RightDoorIndividualOpen = 7,
        // TransformerOffAndUnlocked = 8,
        // TransformerOn = 9,
        // ClearShpWatchman = 10,
        // RadioTelephone = 11,
    };

private:
    bde::ExpanderEp_8 ex0;
    bde::ExpanderEp_8 ex1;
    bde::ExpanderEp_8 ex2;
    bde::PiPicoExpanderEp rp1;

    uint16_t lvCurrent = 0;

    void configureMasterDevice();
    void configureExpanders();

    void masterController();
    void directionController();

    void simulateBatteryCurrent();
};

constexpr bde::datBit_t operator*(ED72::DatOutSw enumValue) {
    return static_cast<std::underlying_type_t<ED72::DatOutSw>>(enumValue);
}


#endif //MUDDC_ED72_H