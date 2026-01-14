//
// Created by Filip on 14.09.2025.
//
#include <bsp/board_api.h>
#include <tusb.h>
#include "MUDDC/MainController.h"

#include "hardware/gpio.h"
#include "hardware/i2c.h"
#include "hardware/pwm.h"

void MainController::initialize() {
    initializeGpio();
    initializePWM();
    initializeI2C();
}

void MainController::cdcTask() {
    if (tud_cdc_n_available(0)) {
        auto count = tud_cdc_n_read(0, datagramIn.data(), datagramIn.size());
        readDevices();
        if (count > 0 && datagramIn.preambleIsValid()) {
            tud_cdc_n_write(0, datagramOut.data(), datagramOut.size());

            tud_cdc_n_write_flush(0);
            tud_cdc_n_write_flush(1);

            writeDevices();

            postTransmissionTask();


            lastTransmissionTime = board_millis();
        } else {
            //TODO protection if transfer start in middle
        }
    }
}

void MainController::blinkTask() {
    if (board_millis() - lastTransmissionTime > disconnectedTimeout) {
        blinkOff  = blinkOff_notConnected;
        blinkOn = blinkOn_notConnected;
    } else {
        blinkOff  = blinkOff_connected;
        blinkOn = blinkOn_connected;
    }

    if (ledState) {
        if (board_millis() - lastBlinkTime > blinkOn) {
            lastBlinkTime = board_millis();
            ledState = false;
        } else {
            return;
        }
    } else {
        if (board_millis() - lastBlinkTime > blinkOff) {
            lastBlinkTime = board_millis();
            ledState = true;
        } else {
            return;
        }
    }
    board_led_write(ledState);
}

void MainController::addMainDeviceGpioInput(uint8_t pin, uint8_t bit, bool negate) {
    mDInputs.emplace_back(pin, bit, negate);
}

void MainController::addMainDeviceGpioOutput(uint8_t pin, uint8_t bit, bool negate) {
    mDOutputs.emplace_back(pin, bit, negate);
}

void MainController::addMainDevicePwm(uint8_t pin, const uint16_t *value, VarType varType) {
    mPwmInputs.emplace_back(pin, const_cast<uint16_t *>(value), varType);
}

void MainController::addI2cExpander(const ExpanderEndpoint &expander) {
    expanders.emplace_back(expander);
}

void MainController::addPicoExpander(const PicoExpander &expander) {
    picoExpanders.emplace_back(expander);
}

void MainController::setPostTransmissionTask(const std::function<void()> &postTransmissionTask) {
    this->postTransmissionTask = postTransmissionTask;
}

DatagramIn & MainController::accessDatagramIn() {
    return datagramIn;
}

void MainController::initializeGpio() {
    for (const auto &gpio: mDInputs) {
        gpio_init(gpio.pin);
        gpio_set_dir(gpio.pin, GPIO_IN);

        if (gpio.negate) {
            gpio_pull_up(gpio.pin);
        } else {
            gpio_pull_down(gpio.pin);
        }

    }

    for (const auto &gpio: mDOutputs) {
        gpio_init(gpio.pin);
        gpio_set_dir(gpio.pin, GPIO_OUT);
    }
}

void MainController::initializeI2C() {
    i2c_init(expanderI2cLine.device, expanderI2cLine.baudrate);
    i2c_init(communicationI2cLine.device, communicationI2cLine.baudrate);
}

void MainController::initializePWM() {
    for (auto &pwm: mPwmInputs) {
        gpio_set_function(pwm.pin, GPIO_FUNC_PWM);
        pwm.slice = pwm_gpio_to_slice_num(pwm.pin);
        pwm.channel = pwm_gpio_to_channel(pwm.pin);
        pwm_set_wrap(pwm.slice, pwmWrap);
        pwm_set_enabled(pwm.slice, true);
    }
}

void MainController::readDevices() {
    readGpioInputs();
    readExpanders();
    readAdc();
    readSecondDevice();
}

void MainController::writeDevices() {
    writeGpio();
    writeExpander();
    writePwm();
    writeSecondDevice();
}

void MainController::readGpioInputs() {
    for (const auto &gpio: mDInputs) {
        bool state = gpio_get(gpio.pin);
        if (gpio.negate) {
            datagramOut.setSwitchState(gpio.bit, !state);
        } else {
            datagramOut.setSwitchState(gpio.bit, state);
        }
    }
}

void MainController::readExpanders() {
    for (const auto &expander: expanders) {
        if (expander.direction == ExpanderDir::Read || expander.direction == ExpanderDir::ReadWrite) {
            uint8_t tmpValue;
            int ret = i2c_read_timeout_us(expanderI2cLine.device, expander.address, &tmpValue, 1, false, 20);
            if (ret < 0) {
                continue;
            }

            for (uint8_t i = 0; i < 8; ++i) {
                if (expander.isInput[i]) {
                    if (expander.negate[i]) {
                        datagramOut.setSwitchState(expander.bit[i], !((tmpValue >> i) & 1));
                    } else {
                        datagramOut.setSwitchState(expander.bit[i], (tmpValue >> i) & 1);
                    }
                }
            }
        }
    }
}

void MainController::readAdc() {
    //TODO implementation
}

void MainController::readSecondDevice() {
    //TODO implementation
}

void MainController::writeGpio() {
    for (const auto &gpio: mDOutputs) {
        if (gpio.negate) {
            gpio_put(gpio.pin, !datagramIn.indicatorState(static_cast<DatagramIn::Indicators>(gpio.bit)));
        } else {
            gpio_put(gpio.pin, datagramIn.indicatorState(static_cast<DatagramIn::Indicators>(gpio.bit)));
        }
    }
}

void MainController::writeExpander() {
    for (const auto &expander: expanders) {
        if (expander.direction == ExpanderDir::Write || expander.direction == ExpanderDir::ReadWrite) {
            uint8_t tmpValue = 0;
            for (uint8_t i = 0; i < 8; ++i) {
                if (expander.isInput[i] == false) {
                    bool tmpBitValue = datagramIn.indicatorState(static_cast<DatagramIn::Indicators>(expander.bit[i]));
                    if (expander.negate[i]) {
                        tmpBitValue = !tmpBitValue;
                    }

                    if (tmpBitValue) {
                        tmpValue |= (1 << i);
                    } else {
                        tmpValue &= ~(1 << i);
                    }
                }
            }

            i2c_write_timeout_us(expanderI2cLine.device, expander.address, &tmpValue, 1, false, 20);
        }
        //TODO implement read
    }
}

void MainController::writeSecondDevice() {
    for (const auto &expander: picoExpanders) {
        if (expander.direction == ExpanderDir::Read || expander.direction == ExpanderDir::ReadWrite) {
            uint32_t tmpValue = 0;
            i2c_read_timeout_us(expanderI2cLine.device, expander.address, reinterpret_cast<uint8_t*>(&tmpValue), 3, false, 20);

            for (uint8_t i = 0; i < 24; ++i) {
                if (expander.isInput[i] == true) {
                    bool value = (tmpValue >> i) & 1;
                    if (expander.negate[i]) {
                        value = !value;
                    }

                    datagramOut.setSwitchState(expander.bit[i], value);
                }
            }

        }
    }

    //TODO implement write
}

void MainController::writePwm() {
    for (const auto &pwm: mPwmInputs) {
        uint16_t pwmValue;
        auto varType = static_cast<uint8_t>(pwm.varType);
        if (varType > 2) {
            pwmValue = *pwm.value / (varType / 2);
        } else if (varType == 4) {
            pwmValue = *pwm.value;
        } else {
            pwmValue = *pwm.value * 2;
        }

        pwm_set_chan_level(pwm.slice, pwm.channel, pwmValue);
    }
}
