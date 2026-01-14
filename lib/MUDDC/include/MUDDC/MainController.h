//
// Created by Filip on 14.09.2025.
//

#ifndef MUDDC_MASTER_MAINCONTROLLER_H
#define MUDDC_MASTER_MAINCONTROLLER_H
#include <functional>

#include "DatagramIn.h"
#include "DatagramOut.h"
#include <vector>

#include "hardware/i2c.h"


class MainController {
public:
    /**
     * Object describing configuration of i2c on board
     */
    struct I2cDevice {
        i2c_inst_t *device;
        uint8_t sdaPin;
        uint8_t sclPin;
        uint baudrate;
    };

    /**
     * Structure keep information about gpio endpoint
     */
    struct GpioEndpoint {
        uint8_t pin; //!< Gpio pin where is connected
        uint8_t bit; //!< Bit in datagram input/output (calculated from first byte for indicators/buttons). For inputs are declared enum, for output declare own enum!
        bool negate; //!< Negate logic. true - value will be negated
    };

    /**
     * Overall direction of expander
     */
    enum class ExpanderDir {
        Read, //!< Read values from expander
        Write, //!< Write values to expander
        ReadWrite //!< Some pins are set to write and some pins are set as write
    };

    /**
     * Structure keep information about gpio expanders
     */
    struct ExpanderEndpoint {
        uint8_t address = 0; //!< Expander address
        std::array<bool, 8> isInput {false, false, false, false, false, false, false, false}; //!< Expander output is output or input
        std::array<bool, 8> negate {false, false, false, false, false, false, false, false}; //!< Negate read information from expander
        std::array<uint8_t, 8> bit {0}; //!< Bit in datagram input/output (calculated from first byte for indicators/buttons). For inputs are declared enum, for output declare own enum!
        ExpanderDir direction = ExpanderDir::ReadWrite; //!< Expander direction is input expander or input direction

        //TODO add constructors to initialize like struct and use masks
    };

    /**
     * Var types for PWM
     */
    enum class VarType: uint8_t {
        uint8_t = 1,
        uint16_t = 2,
        uint32_t = 4,
        uint64_t = 8,
    };

    struct PwmEndpoint {
        uint8_t pin;
        uint16_t *value;
        VarType varType;
        uint slice;
        uint8_t channel;
    };

    //TODO add ADC endpoint on i2c


    struct PicoExpander {
        uint8_t address = 0; //!< Expander address
        std::array<bool, 24> isInput {false, false, false, false, false, false, false, false}; //!< Expander output is output or input
        std::array<bool, 24> negate {false, false, false, false, false, false, false, false}; //!< Negate read information from expander
        std::array<uint8_t, 24> bit {false}; //!< Bit in datagram input/output (calculated from first byte for indicators/buttons). For inputs are declared enum, for output declare own enum!
        ExpanderDir direction = ExpanderDir::ReadWrite; //!< Expander direction is input expander or input direction

        //TODO add constructors to initialize like struct and use masks
    };
    //TODO add endpoint second Pico with setting data bit 0 sth, bit 1 sth, bit 3 uint8_t sth ...

    MainController(const I2cDevice &expander_i2_c_line, const I2cDevice &communication_i2_c_line)
        : expanderI2cLine(expander_i2_c_line),
          communicationI2cLine(communication_i2_c_line) {
    }

    /**
     * Initialize all devices on board (gpio, pwm i2c)
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

    /**
     * Configure data read from pi pico master board and how to interpret it from datagram
     * @param pin pin on pi pico master board
     * @param bit bit in datagram starting on switch group 0 (switch group 0, bit 0 - 0, switch group 1, bit 0 - 8). Define it as enum
     * @param negate negate read state default false
     */
    void addMainDeviceGpioInput(uint8_t pin, uint8_t bit, bool negate = false);

    /**
     * Configure data set to pi pico master board and how to interpret it from datagram
     * @param pin pin on pi pico master board
     * @param bit set bit in datagram. Use @rel DatagramIn::Indicators
     * @param negate negate state default false
     */
    void addMainDeviceGpioOutput(uint8_t pin, uint8_t bit, bool negate = false);

    /**
     * Configure data set to pi pico master board PWM and how to interpret it from datagram
     * @param pin pin on pi pico master board
     * @param value reference (pointer) to value which must be set on PWM device. See @rel DatagramIn
     * @param varType type of variable
     * @brief Usage example
     * @code
     * MainController mc {...};
     * mc.addMainDevicePwm(18, &mc.accessDatagramIn().hvCurrent1(), MainController::VarType::uint16_t);
     * @endcode
     */
    void addMainDevicePwm(uint8_t pin, const uint16_t *value, VarType varType);

    /**
     * Configure expander which can be as output or as input, see @rel MainController::ExpanderEndpoint
     * @param expander expander configuration
     */
    void addI2cExpander(const ExpanderEndpoint &expander);

    /**
     * Configure pi pico expander which can be as output or as input, see @rel MainController::PicoExpander
     * @param expander expander configuration
     */
    void addPicoExpander(const PicoExpander &expander);

    /**
     * Add post UART transmission task
     * @param postTransmissionTask task
     */
    void setPostTransmissionTask(const std::function<void()> &postTransmissionTask);

    /**
     *
     * @return access to datagramIn object
     */
    DatagramIn &accessDatagramIn();

private:
    DatagramIn datagramIn;
    DatagramOut datagramOut;
    uint32_t lastTransmissionTime = 0;
    uint32_t lastBlinkTime = 0;
    uint16_t pwmWrap = 65535;
    static constexpr uint32_t disconnectedTimeout = 2000;

    static constexpr uint32_t blinkOff_notConnected = 100;
    static constexpr uint32_t blinkOn_notConnected = 100;

    static constexpr uint32_t blinkOff_connected = 500;
    static constexpr uint32_t blinkOn_connected = 500;

    uint32_t blinkOff = blinkOff_notConnected;
    uint32_t blinkOn = blinkOn_notConnected;
    bool ledState = false;

    I2cDevice expanderI2cLine;
    I2cDevice communicationI2cLine;

    std::vector<GpioEndpoint> mDInputs;
    std::vector<GpioEndpoint> mDOutputs;
    std::vector<PwmEndpoint> mPwmInputs;
    std::vector<ExpanderEndpoint> expanders;
    std::vector<PicoExpander> picoExpanders;

    std::function<void()> postTransmissionTask = []{};

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


#endif //MUDDC_MASTER_MAINCONTROLLER_H
