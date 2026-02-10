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

    void initialize() override;

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
        ConverterOffAndUnlocked = 17,
        ConverterOn = 18,
        UnlockOverloadRelay = 19,
        LeftDoorIndividualOpen = 20,
        LeftDoorCentralOpen = 21,
        LeftDoorCentralClose = 22,
        DoorBell = 23,
        RightDoorCentralOpen = 24,
        RightDoorCentralClose = 25,
        RightDoorIndividualOpen = 26,
        ClearShpWatchman = 27,
        MainCircuitBreaker = 28,
        RadioCommunicationAttachment = 29, 
        SignalLampCheckI = 30,
        SignalLampCheckIi = 31,
        SignalLampCheckIii =32,
        Camshaft = 33,
        FrontPantographLowered = 34,
        FrontPantographRaised = 35,
        RearPantographRaised = 36,
        RightHeadlightDimmed = 37,
        HeadlightsDimmed = 38
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
