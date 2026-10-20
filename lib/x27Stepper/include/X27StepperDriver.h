//
// Created by Filip on 19.07.2026.
//

#ifndef MUDDC_X27STEPPERDRIVER_H
#define MUDDC_X27STEPPERDRIVER_H

#include <array>

#include "hardware/pwm.h"
#include "pico/mutex.h"
#include "pico/stdlib.h"

struct PwmPin {
    uint slice = 0;
    uint channel = 0;
};

class X27StepperDriver {
public:
    struct Step {
        float pin1 = 0;
        float pin2 = 0;
        float pin3 = 0;
        float pin4 = 0;
    };

    inline static uint16_t frequency = 30'000;

    X27StepperDriver(uint8_t pin1, uint8_t pin2, uint8_t pin3, uint8_t pin4, mutex_t *mutex, uint16_t maxStep = 315,
                     uint16_t wrap = 255);

    inline void prepareOnlyBackFull(uint8_t time_s = 4) {
        prepareOnlyBack(time_s, maxStep);
    }

    void prepareOnlyBack(uint8_t time_s = 1, uint16_t fromStep = 20);

    void goToStepIn(uint16_t step, uint16_t ms);

    [[nodiscard]] inline uint16_t getMaxStep() const {
        return maxStep;
    }

    [[nodiscard]] inline uint16_t maxMicroStep() const {
        return maxStep * microSteps.size() / 2;
    }

    [[nodiscard]] constexpr inline uint16_t singleStepSize() const {
        return microSteps.size() / 2;
    }

    void makeStep();

private:
    std::array<Step, 24> microSteps;
    uint8_t middleMicroStep;
    uint16_t actualStep = 0;
    uint16_t targetStep = 0;
    uint16_t maxStep = 157 * 2;
    uint8_t stepsStep = 1;
    uint16_t singleStepTime = 0;
    uint8_t pin1;
    uint8_t pin2;
    uint8_t pin3;
    uint8_t pin4;
    uint16_t wrap = 255;

    uint64_t lastUpdateTime = 0;

    mutex_t *mutex = nullptr;

    pwm_config pwmConfig;

    PwmPin pwm1;
    PwmPin pwm2;
    PwmPin pwm3;
    PwmPin pwm4;

    friend bool x27DriverCallback(repeating_timer_t *timer);

    void initPwm();

    void initSinglePwm(PwmPin &pwmData, uint8_t pin);
};


#endif //MUDDC_X27STEPPERDRIVER_H
