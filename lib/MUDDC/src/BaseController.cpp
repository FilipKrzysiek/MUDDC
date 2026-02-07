//
// Created by Filip on 22.01.2026.
//
#include <bsp/board_api.h>
#include "MUDDC/BaseController.h"

#include "hardware/gpio.h"
#include "hardware/i2c.h"
#include "hardware/pwm.h"
#include "MUDDC/bitFunctions.h"

void BaseController::initialize() {
    initializeGpio();
    initializePWM();
    initializeI2C();
}

void BaseController::cdcTask() {
    if (tud_cdc_n_available(0)) {
        auto count = tud_cdc_n_read(0, datagramIn.data(), datagramIn.size());
        readDevices();
        postReceiveAndReadDevTask();

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

void BaseController::blinkTask() {
    if (board_millis() - lastTransmissionTime > disconnectedTimeout) {
        blinkOff = blinkOff_notConnected;
        blinkOn = blinkOn_notConnected;
    } else {
        blinkOff = blinkOff_connected;
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

void BaseController::initializeGpio() {
    for (const auto& gpio : masterDeviceInputs) {
        gpio_init(gpio.pin);
        gpio_set_dir(gpio.pin, GPIO_IN);

        if (gpio.negate) {
            gpio_pull_up(gpio.pin);
        } else {
            gpio_pull_down(gpio.pin);
        }
    }

    for (const auto& gpio : masterDeviceOutputs) {
        gpio_init(gpio.pin);
        gpio_set_dir(gpio.pin, GPIO_OUT);
    }
}

void BaseController::initializeI2C() {
    i2c_init(expanderI2cLine.device, expanderI2cLine.baudrate);
    i2c_init(communicationI2cLine.device, communicationI2cLine.baudrate);

    gpio_set_function(expanderI2cLine.sdaPin, GPIO_FUNC_I2C);
    gpio_set_function(expanderI2cLine.sclPin, GPIO_FUNC_I2C);

    gpio_set_function(communicationI2cLine.sdaPin, GPIO_FUNC_I2C);
    gpio_set_function(communicationI2cLine.sclPin, GPIO_FUNC_I2C);
}

void BaseController::initializePWM() {
    for (auto& pwm : masterDevicePwm) {
        gpio_set_function(pwm.getPin(), GPIO_FUNC_PWM);
        pwm.slice = pwm_gpio_to_slice_num(pwm.getPin());
        pwm.channel = pwm_gpio_to_channel(pwm.getPin());
        pwm_set_wrap(pwm.slice, bde::PwmEp::MAX_VAL);
        pwm_set_enabled(pwm.slice, true);
    }
}

void BaseController::readDevices() {
    readGpioInputs();
    readExpanders();
    readAdc();
    readSecondDevice();
}

void BaseController::writeDevices() {
    writeGpio();
    writeExpander();
    writePwm();
    writeSecondDevice();
}

void BaseController::readGpioInputs() {
    for (auto& gpio : masterDeviceInputs) {
        bool state = gpio_get(gpio.pin);
        if (gpio.negate) {
            state = !state;
        }

        gpio.value = state;

        if (gpio.bit >= 0) {
            datagramOut.setSwitchState(gpio.bit, state);
        }
    }
}

void BaseController::readExpanders() {
    for (auto& expander : expanders) {
        if (expander->overallDir == bde::CommunicationDir::Read || expander->overallDir ==
            bde::CommunicationDir::ReadWrite) {
            uint8_t tmpValue = 0;
            int ret = i2c_read_timeout_us(expanderI2cLine.device, expander->address, &tmpValue, 1, false, 400);
            if (ret < 0) {
                std::string tmp = "Communication error ex2: " + std::to_string(ret) + "\n\r";
                tud_cdc_n_write(1, tmp.c_str(), tmp.size());
                continue;
            }

            for (uint8_t i = 0; i < 8; ++i) {
                if (expander->isInput[i]) {
                    bool tmpBitValue = (tmpValue >> i) & 1;
                    if (expander->negate[i]) {
                        tmpBitValue = !tmpBitValue;
                    }

                    setValueOnBit(expander->value, i, tmpBitValue);

                    if (expander->bitInDatagram[i] >= 0) {
                        datagramOut.setSwitchState(expander->bitInDatagram[i], tmpBitValue);
                    }
                }
            }
            std::string tmp = "Ex2 value: " + std::to_string(tmpValue) + " -> " + std::to_string(expander->value) + "\n\r";
            tud_cdc_n_write(1, tmp.c_str(), tmp.size());
        }
    }
}

void BaseController::readAdc() {
    //TODO implement read master device Adc

}

void BaseController::readSecondDevice() {
    for (auto &expander: piPicoExpanders) {
        if (expander->overallDir == bde::CommunicationDir::Read || expander->overallDir == bde::CommunicationDir::ReadWrite) {
            uint32_t tmpValue = 0;
            int ret = i2c_read_timeout_us(communicationI2cLine.device, expander->address,
                                          reinterpret_cast<uint8_t *>(&tmpValue), 3, false, 400);
            if (ret < 0) {
                continue;
            }

            for (uint8_t i = 0; i < 24; ++i) {
                if (expander->isInput[i] == true) {
                    bool tmpBitValue = (tmpValue >> i) & 1;
                    if (expander->negate[i]) {
                        tmpBitValue = !tmpBitValue;
                    }
                    setValueOnBit(expander->value, i, tmpBitValue);

                    if (expander->bitInDatagram[i] >= 0)
                        datagramOut.setSwitchState(expander->bitInDatagram[i], tmpBitValue);
                }
            }
        }
    }
    //TODO refactor duplicated code
}

void BaseController::writeGpio() {
    for (auto &gpio: masterDeviceOutputs) {
        bool state = datagramIn.indicatorState(static_cast<DatagramIn::Indicators>(gpio.bit));
        if (gpio.negate) {
            gpio_put(gpio.pin, !state);
            gpio.value = !state;
        } else {
            gpio_put(gpio.pin, state);
            gpio.value = state;
        }
    }
}

void BaseController::writeExpander() {
    for (const auto &expander: expanders) {
        if (expander->overallDir == bde::CommunicationDir::Write || expander->overallDir == bde::CommunicationDir::ReadWrite) {
            uint8_t tmpValue = 0;
            for (uint8_t i = 0; i < 8; ++i) {
                if (expander->isInput[i] == false) {
                    bool tmpBitValue = datagramIn.indicatorState(static_cast<DatagramIn::Indicators>(expander->bitInDatagram[i]));
                    if (expander->negate[i]) {
                        tmpBitValue = !tmpBitValue;
                    }

                    setValueOnBit(expander->value, i, tmpBitValue);
                    setValueOnBit(tmpValue, i, tmpBitValue);
                }
            }

            i2c_write_timeout_us(expanderI2cLine.device, expander->address, &tmpValue, 1, false, 20);
        }
    }
}

void BaseController::writeSecondDevice() {
    //TODO implement
}

void BaseController::writePwm() {
    for (const auto &pwm: masterDevicePwm) {
        pwm_set_chan_level(pwm.slice, pwm.channel, pwm.calculateValue());
    }
}
