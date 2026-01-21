//
// Created by Filip on 14.09.2025.
//
#include <bsp/board_api.h>
#include <tusb.h>
#include "MUDDC/MainController.h"

#include <string>

#include "hardware/gpio.h"
#include "hardware/i2c.h"
#include "hardware/pwm.h"

MainController::PwmEndpoint::PwmEndpoint(uint8_t pin, const VarPtrProxy<uint16_t> &raw_value,
    const std::map<uint16_t, uint16_t> &valuePoints): slice(0), channel(0), pin(pin),
                                                      rawValue(raw_value) {
    if (!valuePoints.contains(0)) {
        this->valuePoints.emplace_back(0, 0);
    }

    for (auto [raw, destVal]: valuePoints) {
        this->valuePoints.emplace_back(raw, destVal);
    }

    if (!valuePoints.contains(MAX_VAL)) {
        this->valuePoints.emplace_back(MAX_VAL, MAX_VAL);
    }
}

uint16_t MainController::PwmEndpoint::calculateValue() const {
    for (int i = 1; i < valuePoints.size(); ++i) {
        if (valuePoints[i].first == rawValue()) {
            return valuePoints[i].second;
        }

        if (valuePoints[i].first > rawValue()) {
            return (valuePoints[i].second - valuePoints[i - 1].second) / (valuePoints[i].first - valuePoints[i - 1].first) * rawValue();
        }
    }
    return MAX_VAL;
}

MainController::PicoExpander::PicoExpander(uint8_t address, const std::array<bool, 24> &is_input,
                                           const std::array<bool, 24> &negate, const std::array<uint8_t, 24> &bit, ExpanderDir direction): address(address),
                                                                                                                                           isInput(is_input),
                                                                                                                                           negate(negate),
                                                                                                                                           bit(bit),
                                                                                                                                           direction(direction) {
}

MainController::PicoExpander::PicoExpander(uint8_t address, uint32_t isInputMask, uint32_t negateMask,
    const std::array<uint8_t, 24> &bit, ExpanderDir direction): address(address), bit(bit), direction(direction) {
    for (int i = 0; i < isInput.size(); i++) {
        isInput[i] = isInputMask >> i & 1;
        negate[i] = negateMask >> i & 1;
    }
}

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

void MainController::addMainDevicePwm(const PwmEndpoint &pwmEndpoint) {
    mPwmInputs.emplace_back(pwmEndpoint);
}

void MainController::addI2cExpander(const ExpanderEndpoint &expander) {
    expanders.emplace_back(expander);
}

void MainController::addPicoExpander(const PicoExpander &expander) {
    picoExpanders.emplace_back(expander);
}

void MainController::setPostTransmissionTask(const std::function<void()> &postTransmissionTaskP) {
    postTransmissionTask = postTransmissionTaskP;
}

const DatagramIn & MainController::accessDatagramIn() const {
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
        gpio_set_function(pwm.getPin(), GPIO_FUNC_PWM);
        pwm.slice = pwm_gpio_to_slice_num(pwm.getPin());
        pwm.channel = pwm_gpio_to_channel(pwm.getPin());
        pwm_set_wrap(pwm.slice, PwmEndpoint::MAX_VAL);
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
    for (const auto &expander: picoExpanders) {
        if (expander.direction == ExpanderDir::Read || expander.direction == ExpanderDir::ReadWrite) {
            uint32_t tmpValue = 0;
            int ret = i2c_read_timeout_us(expanderI2cLine.device, expander.address, reinterpret_cast<uint8_t*>(&tmpValue), 3, false, 20);
            if (ret < 0) {
                continue;
            }

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
    }
}

void MainController::writeSecondDevice() {
    //TODO implement write
}

void MainController::writePwm() {
    for (const auto &pwm: mPwmInputs) {
        pwm_set_chan_level(pwm.slice, pwm.channel, pwm.calculateValue());
    }
}
