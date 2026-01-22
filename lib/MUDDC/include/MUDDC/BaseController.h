//
// Created by Filip on 22.01.2026.
//

#ifndef MUDDC_BASECONTROLLER_H
#define MUDDC_BASECONTROLLER_H
#include "BaseDevicesAndEndpoint.h"
#include "DatagramIn.h"
#include "DatagramOut.h"


class BaseController {
public:
    BaseController(const bde::I2cDevice &expanderI2cline, const bde::I2cDevice &communicationI2CLine)
        : expanderI2cLine(expanderI2cline),
          communicationI2cLine(communicationI2CLine) {
    }

    virtual ~BaseController() = default;

    /**
     * Configure, add all connected devices
     */
    virtual void setup() = 0;

    /**
     * Task called after transmission. Task called on end cdcTask (after receive, read data from devices, send data and write to devices)
     */
    virtual void  postTransmissionTask() = 0;

    /**
     * Task called after receive and read data from devices.
     */
    virtual void postReceiveAndReadDevTask() = 0;

    /**
     * Initialize all devices added in `setup` method on board (gpio, pwm, i2c)
     */
    void initialize();

    /**
     * UART task must be callable in endless loop
     */
    void cdcTask();

    /**
     * Onboard blink task informating of connection status. Must be callable in endless loop
     */
    void blinkTask();

    constexpr inline void addMasterDevicePwm(uint8_t pin, const VarPtrProxy<uint16_t> &raw_value, const std::map<uint16_t, uint16_t> &valuePoints);

    constexpr inline void addMasterDeviceGpioInput(uint8_t pin, bde::datBit_t bit, bool negate = false);

    constexpr inline void addMasterDeviceGpioOutput(uint8_t pin, bde::datBit_t bit, bool negate = false);


protected:
    DatagramIn datagramIn;
    DatagramOut datagramOut;
    bde::I2cDevice expanderI2cLine;
    bde::I2cDevice communicationI2cLine;

    std::vector<bde::GpioEp> masterDeviceInputs;
    std::vector<bde::GpioEp> masterDeviceOutputs;
    std::vector<bde::PwmEp> masterDevicePwm;
    std::vector<bde::ExpanderEp_8*> expanders;
    std::vector<bde::PiPicoExpanderEp*> piPicoExpanders;

private:
    uint32_t lastTransmissionTime = 0;
    uint32_t lastBlinkTime = 0;
    static constexpr uint32_t disconnectedTimeout = 2000;

    static constexpr uint32_t blinkOff_notConnected = 100;
    static constexpr uint32_t blinkOn_notConnected = 100;

    static constexpr uint32_t blinkOff_connected = 500;
    static constexpr uint32_t blinkOn_connected = 500;

    uint32_t blinkOff = blinkOff_notConnected;
    uint32_t blinkOn = blinkOn_notConnected;
    bool ledState = false;

    void initializeGpio();

    void initializeI2C();

    void initializePWM();

    void readDevices();

    void writeDevices();

    void readGpioInputs();

    void readExpanders();

    void readAdc();

    void readSecondDevice();

    void writeGpio();

    void writeExpander();

    void writeSecondDevice();

    void writePwm();
};

constexpr void BaseController::addMasterDevicePwm(uint8_t pin, const VarPtrProxy<uint16_t>& raw_value,
    const std::map<uint16_t, uint16_t>& valuePoints) {
    masterDevicePwm.emplace_back(pin, raw_value, valuePoints);
}

constexpr void BaseController::addMasterDeviceGpioInput(uint8_t pin, bde::datBit_t bit, bool negate) {
    masterDeviceInputs.emplace_back(pin, bit, negate);
}

constexpr void BaseController::addMasterDeviceGpioOutput(uint8_t pin, bde::datBit_t bit, bool negate) {
    masterDeviceOutputs.emplace_back(pin, bit, negate);
}


#endif //MUDDC_BASECONTROLLER_H
