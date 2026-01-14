//
// Created by Filip on 07.09.2025.
//

#ifndef RAWDATA_H
#define RAWDATA_H

#include <cstdint>
#include <vector>

class RawData {
public:
    static constexpr uint32_t preambleValue = 0xEFEF'EFEF;
    struct DataIn {
    // private:
        uint8_t preamble[4] = {0, 0, 0, 0};
        uint8_t tacho[2] = {0, 0};
    public:
        uint8_t indicatorGroup0 = 0;
        uint8_t indicatorGroup1 = 0;
        uint8_t indicatorGroup2 = 0;
        uint8_t indicatorGroup3 = 0;
        uint8_t indicatorGroup4 = 0;
    private:
        uint8_t breakPress[2] = {0, 0};
        uint8_t pipePress[2] = {0, 0};
        uint8_t tankPress[2] = {0, 0};
        uint8_t hvVoltage[2] = {0, 0};
        uint8_t hvCurrent1[2] = {0, 0};
        uint8_t hvCurrent2[2] = {0, 0};
        uint8_t hvCurrent3[2] = {0, 0};
        uint8_t yearMonth[2] = {0, 0};
        uint8_t dayHourMinute[2] = {0, 0};
        uint8_t secondMillisecond[2] = {0, 0};
        uint8_t odometer[4] = {0, 0, 0, 0};
        uint8_t lvVoltage[2] = {0, 0};
    public:
        uint8_t radioChanel = 0;
    private:
        uint8_t pantographPress[2] = {0, 0};
        uint8_t empty[12];
    public:

    };

    struct DataOut {
    private:
        uint16_t preamble[2] = {0xEFEF, 0xEFEF};
    public:
        uint8_t switchGroup0 = 0;
        uint8_t switchGroup1 = 0;
        uint8_t switchGroup2 = 0;
        uint8_t switchGroup3 = 0;
        uint8_t switchGroup4 = 0;
        uint8_t switchGroup5 = 0;
        uint8_t masterController = 0;
        uint8_t secondController = 0;
        uint16_t trainBrake = 0;
        uint16_t independentBrake = 0;
    private:
        uint16_t empty[2];
    };

    enum Indicators {
        EpBreak = 0,
        VentilatorOverload = 1,
        MotorOverload = 2,
        EmergencyBreak = 3,
        LockPipe = 4,
        DirForward = 5,
        DirBackward = 6,
        CoupledHvVoltage = 8,
        DoorLeftAllowed = 9,
        DoorLeftOpened = 10,
        DoorRightAllowed = 11,
        DoorRightOpened = 12,
        DoorStepAllowed = 13,
        Battery = 14,
        TrainHeating = 16,
        MotorResistors = 17,
        WheelSlip = 18,
        Alerter = 22,
        Shp = 23,
        MotorConnectors = 24,
        ConverterOverload = 26,
        GroundRelay = 27,
        MotorOverload2 = 28,
        LineBreaker = 29,
        CompressorOverload = 30,
        TrainStateCab = 34,
        RecorderBraking = 35,
        RecorderPower = 36,
        RadioStop = 37,
        SpringBrakeActive = 38,
        AlerterSound = 39
    };


    bool getIndicatorState(Indicators indicator);

    void setSwitchState(uint8_t bit, bool state);

    /**
     * Get pointer to data in structure
     * @return pair with pointer to data and data size
     */
    std::pair<uint8_t *, int>getInDataAsPointer();

    /**
     * Get pointer to data in structure
     * @return pair with pointer to data and data size
     */
    std::pair<uint8_t *, int>getOutDataAsPointer();

    bool dataInIsOk() const;

private:
    DataIn dataInStruct;
    DataOut dataOutStruct;

};



#endif //RAWDATA_H
