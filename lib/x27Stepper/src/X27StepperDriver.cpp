//
// Created by Filip on 19.07.2026.
//

#include "X27StepperDriver.h"

#include <cmath>
#include <iostream>
#include "pico/mutex.h"
#include "hardware/clocks.h"

X27StepperDriver::X27StepperDriver(uint8_t pin1, uint8_t pin2, uint8_t pin3, uint8_t pin4, mutex_t *mutex,
                                   uint16_t maxStep, uint16_t wrap) : pin1(pin1), pin2(pin2), pin3(pin3), pin4(pin4),
                                                                      mutex(mutex),
                                                                      maxStep(maxStep), wrap(wrap),
                                                                      middleMicroStep(12) {
    microSteps = {
        {
            {.pin1 = 0.80, .pin2 = 0, .pin3 = 0.80, .pin4 = 0},
            {.pin1 = 0.85, .pin2 = 0, .pin3 = 0.55, .pin4 = 0},
            {.pin1 = 0.90, .pin2 = 0, .pin3 = 0.35, .pin4 = 0},
            {.pin1 = 0.85, .pin2 = 0, .pin3 = 0.15, .pin4 = 0},
            {.pin1 = 0.80, .pin2 = 0, .pin3 = 0, .pin4 = 0},
            {.pin1 = 0.55, .pin2 = 0, .pin3 = 0, .pin4 = 0.15},
            {.pin1 = 0.35, .pin2 = 0, .pin3 = 0, .pin4 = 0.35},
            {.pin1 = 0.15, .pin2 = 0, .pin3 = 0, .pin4 = 0.55},
            {.pin1 = 0.0, .pin2 = 0, .pin3 = 0, .pin4 = 0.80},
            {.pin1 = 0.0, .pin2 = 0.15, .pin3 = 0, .pin4 = 0.85},
            {.pin1 = 0.0, .pin2 = 0.35, .pin3 = 0, .pin4 = 0.90},
            {.pin1 = 0.0, .pin2 = 0.55, .pin3 = 0, .pin4 = 0.85},
            {.pin1 = 0.0, .pin2 = 0.80, .pin3 = 0, .pin4 = 0.80},
            {.pin1 = 0.0, .pin2 = 0.85, .pin3 = 0, .pin4 = 0.55},
            {.pin1 = 0.0, .pin2 = 0.90, .pin3 = 0, .pin4 = 0.35},
            {.pin1 = 0.0, .pin2 = 0.85, .pin3 = 0, .pin4 = 0.15},
            {.pin1 = 0.0, .pin2 = 0.80, .pin3 = 0, .pin4 = 0},
            {.pin1 = 0.0, .pin2 = 0.55, .pin3 = 0.15, .pin4 = 0},
            {.pin1 = 0.0, .pin2 = 0.35, .pin3 = 0.35, .pin4 = 0},
            {.pin1 = 0.0, .pin2 = 0.15, .pin3 = 0.55, .pin4 = 0},
            {.pin1 = 0.0, .pin2 = 0, .pin3 = 0.80, .pin4 = 0},
            {.pin1 = 0.15, .pin2 = 0, .pin3 = 0.85, .pin4 = 0},
            {.pin1 = 0.35, .pin2 = 0, .pin3 = 0.90, .pin4 = 0},
            {.pin1 = 0.55, .pin2 = 0, .pin3 = 0.85, .pin4 = 0},
        }
    };

    initPwm();
}

void X27StepperDriver::prepareOnlyBack(uint8_t time_s, uint16_t fromStep) {
    if (fromStep / (time_s * 1000) < 5) {
        time_s = 2;
    }

    actualStep = fromStep * singleStepSize();
    singleStepTime = (time_s * 1000 * 1000) / actualStep;
    targetStep = 0;
}

void X27StepperDriver::goToStepIn(uint16_t step, uint16_t ms) {
    mutex_enter_blocking(mutex);
    targetStep = step * singleStepSize();
    uint16_t stepDiff = std::abs(actualStep - targetStep);
    uint16_t stepTime = (ms * 1000) / stepDiff;

    if (stepTime > 1000) {
        stepsStep = 1;
    } else if (stepTime > 500) {
        stepsStep = 2;
    }

    singleStepTime = std::max(stepTime * stepsStep, 300);
    mutex_exit(mutex);
}

void X27StepperDriver::initPwm() {
    float clkDiv = static_cast<float>(clock_get_hz(clk_sys)) / static_cast<float>((wrap + 1) * frequency);

    pwmConfig = pwm_get_default_config();
    pwm_config_set_wrap(&pwmConfig, wrap);
    pwm_config_set_clkdiv(&pwmConfig, clkDiv);

    initSinglePwm(pwm1, pin1);
    initSinglePwm(pwm2, pin2);
    initSinglePwm(pwm3, pin3);
    initSinglePwm(pwm4, pin4);
}

void X27StepperDriver::initSinglePwm(PwmPin &pwmData, uint8_t pin) {
    gpio_set_function(pin, GPIO_FUNC_PWM);
    pwmData.slice = pwm_gpio_to_slice_num(pin);
    pwmData.channel = pwm_gpio_to_channel(pin);
    pwm_init(pwmData.slice, &pwmConfig, true);
}

void X27StepperDriver::makeStep() {
    if (time_us_64() - lastUpdateTime > singleStepTime) {
        lastUpdateTime = time_us_64();

        if (actualStep != targetStep) {
            if (!mutex_try_enter(mutex, nullptr)) {
                lastUpdateTime -= singleStepTime;
                return;
            }
            if (actualStep < targetStep) {
                actualStep += stepsStep;
            } else {
                actualStep -= stepsStep;
            }

            uint16_t microStepToApply = actualStep % microSteps.size();
            mutex_exit(mutex);

            pwm_set_chan_level(pwm1.slice, pwm1.channel, wrap - (microSteps[microStepToApply].pin1 * wrap));
            pwm_set_chan_level(pwm2.slice, pwm2.channel, wrap - (microSteps[microStepToApply].pin2 * wrap));
            pwm_set_chan_level(pwm3.slice, pwm3.channel, wrap - (microSteps[microStepToApply].pin3 * wrap));
            pwm_set_chan_level(pwm4.slice, pwm4.channel, wrap - (microSteps[microStepToApply].pin4 * wrap));
        }
    }
}
