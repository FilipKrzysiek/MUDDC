//
// Created by Filip on 14.09.2025.
//

#ifndef MUDDC_MASTER_DATAGRAMIN_H
#define MUDDC_MASTER_DATAGRAMIN_H
#include <array>
#include <cstdint>

#include "VarPtrProxy.h"

/**
 * Datagram received from Maszyna (PC)
 * @author Krzysztof Swłdek
 */
class DatagramIn {
public:
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
        AlerterSound = 39,
        None = -1
    };


    uint8_t * data();

    [[nodiscard]] const uint8_t * data() const;

    [[nodiscard]] constexpr uint32_t size() const;

    [[nodiscard]] bool preambleIsValid() const;

    const uint16_t &tacho();

    /**
     * Get value from datagram of passed element
     * @param indicatorIndex Enum which indicator value you want to get
     * @return Indicator state
     */
    [[nodiscard]] bool indicatorState(Indicators indicatorIndex) const;

    [[nodiscard]] VarPtrProxy<uint16_t> breakPress() const;

    [[nodiscard]] VarPtrProxy<uint16_t> pipePress() const;

    [[nodiscard]] VarPtrProxy<uint16_t> tankPress() const;

    [[nodiscard]] VarPtrProxy<uint16_t> hvVoltage() const;

    [[nodiscard]] VarPtrProxy<uint16_t> hvCurrent1() const;

    [[nodiscard]] VarPtrProxy<uint16_t> hvCurrent2() const;

    [[nodiscard]] VarPtrProxy<uint16_t> hvCurrent3() const;

    [[nodiscard]] uint8_t year() const;

    [[nodiscard]] uint8_t month() const;

    [[nodiscard]] uint8_t day() const;

    [[nodiscard]] uint8_t hour() const;

    [[nodiscard]] uint8_t minute() const;

    [[nodiscard]] uint8_t second() const;

    [[nodiscard]] uint8_t millisecond() const;

    [[nodiscard]] VarPtrProxy<uint32_t> odometer() const;

    [[nodiscard]] VarPtrProxy<uint16_t> lvVoltage() const;

    [[nodiscard]] const uint8_t &radioChanel() const;

    [[nodiscard]] VarPtrProxy<uint16_t> pantographPress() const;

private:
    std::array<uint8_t, 52> rawData = {};
};

constexpr uint32_t DatagramIn::size() const {
    return rawData.size();
}

inline uint8_t * DatagramIn::data() {
    return rawData.data();
}

inline const uint8_t * DatagramIn::data() const {
    return rawData.data();
}

#endif //MUDDC_MASTER_DATAGRAMIN_H
