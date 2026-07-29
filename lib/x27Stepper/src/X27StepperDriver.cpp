//
// Created by Filip on 19.07.2026.
//

#include "X27StepperDriver.h"

#include <cmath>
#include <iostream>

#include "hardware/clocks.h"

X27StepperDriver::X27StepperDriver(uint8_t pin1, uint8_t pin2, uint8_t pin3, uint8_t pin4, uint16_t maxStep,
                     uint16_t wrap) : pin1(pin1),
                                      pin2(pin2), pin3(pin3), pin4(pin4), maxStep(maxStep), wrap(wrap), middleMicroStep(12) {
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

void X27StepperDriver::prepareOnlyBack(uint8_t time_s) {
    if (maxStep / (time_s * 1000) < 5) {
        time_s = 2;
    }

    actualStep = maxStep;
    targetStep = 0;
    add_repeating_timer_us(time_s * 1000 * 1000 / (maxStep * microSteps.size() / 2 ), x27DriverCallback, this, &timer);
}

void X27StepperDriver::prepareOnlyBackSlow0(uint8_t coarseZeroing_s, uint8_t fineZeroing_s) {
    prepareOnlyBack(coarseZeroing_s);
    sleep_ms(fineZeroing_s * 1000);

    actualStep = 20;
    targetStep = 0;
    add_repeating_timer_us(fineZeroing_s * 1000 * 1000 / (maxStep * microSteps.size() / 2 ), x27DriverCallback, this, &timer);
}

void X27StepperDriver::goToStepIn(uint16_t step, uint16_t ms) {
    // cancel_repeating_timer(&timer);

    uint16_t stepDiff = std::abs(actualStep - step);
    if (step <= maxStep && ms / stepDiff >= 5) {
        targetStep = step;

        add_repeating_timer_us(ms * 1000 / (stepDiff * microSteps.size() / 2 ), x27DriverCallback, this, &timer);
    } else {
        std::cout << "Not set!" << std::endl;
    }
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

void X27StepperDriver::initSinglePwm(PwmPin& pwmData,uint8_t pin) {
    gpio_set_function(pin, GPIO_FUNC_PWM);
    pwmData.slice = pwm_gpio_to_slice_num(pin);
    pwmData.channel = pwm_gpio_to_channel(pin);
    pwm_init(pwmData.slice, &pwmConfig, true);
}

bool X27StepperDriver::makeStep() {
    if (actualStep < targetStep) {
        if (actualMicroStep >= microSteps.size()) {
            actualMicroStep = 0;
            ++actualStep;
        } else {
            ++actualMicroStep;
        }
    } else {
        if (actualMicroStep < 0) {
            actualMicroStep = microSteps.size() - 1;
            --actualStep;
        } else {
            --actualMicroStep;
        }
    }

    if (actualMicroStep == middleMicroStep) {
        if (actualStep < targetStep) {
            ++actualStep;
        } else {
            --actualStep;
        }
    }

    pwm_set_chan_level(pwm1.slice, pwm1.channel, wrap - (microSteps[actualMicroStep].pin1 * wrap));
    pwm_set_chan_level(pwm2.slice, pwm2.channel, wrap - (microSteps[actualMicroStep].pin2 * wrap));
    pwm_set_chan_level(pwm3.slice, pwm3.channel, wrap - (microSteps[actualMicroStep].pin3 * wrap));
    pwm_set_chan_level(pwm4.slice, pwm4.channel, wrap - (microSteps[actualMicroStep].pin4 * wrap));

    if (targetStep == actualStep) {
        cancel_repeating_timer(&timer);
    }

    return targetStep == actualStep;
}

bool x27DriverCallback(repeating_timer_t *timer) {
    auto x27Driver = static_cast<X27StepperDriver *>(timer->user_data);
    x27Driver->makeStep();

    return true;
}