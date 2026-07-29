//
// Created by Filip on 19.07.2026.
//

#ifndef MUDDC_X27STEPPERDRIVER_H
#define MUDDC_X27STEPPERDRIVER_H

#include <array>

#include "hardware/pwm.h"
#include "pico/stdlib.h"


class X27StepperDriver {
public:
    struct Step {
        float pin1 = 0;
        float pin2 = 0;
        float pin3 = 0;
        float pin4 = 0;
    };

    inline static uint16_t frequency = 30'000;

    struct PwmPin {
        uint slice = 0;
        uint channel = 0;
    };

    X27StepperDriver(uint8_t pin1, uint8_t pin2, uint8_t pin3, uint8_t pin4, uint16_t maxStep = 315, uint16_t wrap = 255);

    void prepareOnlyBack(uint8_t time_s = 4);

    void prepareOnlyBackSlow0(uint8_t coarseZeroing_s = 4, uint8_t fineZeroing_s = 2);

    void goToStepIn(uint16_t step, uint16_t ms);

    inline repeating_timer& getTimer() {
        return timer;
    }

    inline uint16_t getMaxStep() {
        return maxStep;
    }

private:
    std::array<Step, 24> microSteps;
    uint8_t middleMicroStep;
    int16_t actualStep = 0;
    int16_t targetStep = 0;
    int8_t actualMicroStep = 0;
    uint16_t maxStep = 157 * 2;
    uint8_t pin1;
    uint8_t pin2;
    uint8_t pin3;
    uint8_t pin4;
    uint16_t wrap = 255;

    pwm_config pwmConfig;

    PwmPin pwm1;
    PwmPin pwm2;
    PwmPin pwm3;
    PwmPin pwm4;

    repeating_timer timer;

    friend bool x27DriverCallback(repeating_timer_t * timer);

    void initPwm();

    void initSinglePwm(PwmPin& pwmData,uint8_t pin);

    bool makeStep();
};

bool x27DriverCallback(repeating_timer_t * timer);


#endif //MUDDC_X27STEPPERDRIVER_H
