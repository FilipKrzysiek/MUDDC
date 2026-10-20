//
// Created by Filip on 22.01.2026.
//

#ifndef MUDDC_BASEDEVICESANDENDPOINT_H
#define MUDDC_BASEDEVICESANDENDPOINT_H

#include <bitset>
#include <cstdint>
#include <map>
#include <vector>

#include "VarPtrProxy.h"
#include "hardware/i2c.h"

#include "bitFunctions.h"

namespace bde {
    using datBit_t = int8_t;
    //!< Datagram bit in buttons/ switches section. If positive value is in datagram if below 0 is only name for value

    template<typename enumType>
    constexpr datBit_t u(enumType enumValue) noexcept {
        return static_cast<std::underlying_type_t<enumType>>(enumValue);
    }

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
    struct GpioEp {
        uint8_t pin; //!< Gpio pin where is connected on board
        datBit_t bit;
        //!< Bit in datagram input/output (calculated from first byte for indicators/buttons). For inputs are declared enum, for output declare own enum!
        bool negate; //!< Negate logic. true - value will be negated
        bool value; //!< Last read value or value after save

        GpioEp(uint8_t pin, datBit_t bit, bool negate = false)
            : pin(pin),
              bit(bit),
              negate(negate),
              value(false) {
        }
    };

    /**
     * Communication direction
     */
    enum class CommunicationDir {
        Undefined,
        Read, //!< Read values from expander
        Write, //!< Write values to expander
        ReadWrite, //!< Some pins are set to write and some pins are set as write
    };

    template<uint8_t gpios, typename valueType>
    struct ExpanderEp {
        uint8_t address = 0; //!< Expander address
        std::bitset<gpios> isInput; //!< Expander output is output or input (true is input)
        std::bitset<gpios> negate; //!< Negate read information from expander
        /**
         * A bit in datagram input/output (calculated from first byte for indicators/buttons).
         * For inputs are declared enum, for output declare own enum!
         * If value don't have assigment in datagram use enum with -1 value
         */
        std::array<datBit_t, gpios> bitInDatagram;
        CommunicationDir overallDir = CommunicationDir::Undefined;
        //!< Expander direction is input, output or both direction expander
        valueType value; //!< Value read from expander or saved to it

        ExpanderEp() = default;

        ExpanderEp(uint8_t address, const std::bitset<gpios> &isInput, const std::bitset<gpios> &negate,
                   const std::array<datBit_t, gpios> &bit_in_datagram, CommunicationDir overall_dir) : address(address),
            isInput(isInput),
            negate(negate),
            bitInDatagram(bit_in_datagram),
            overallDir(overall_dir),
            value(0) {
        }

        /**
         * Get value of one pin in expander
         * @param pin gpio pin
         * @return value on pin
         */
        [[nodiscard]] bool getValueOnPin(uint8_t pin) const {
            return (value >> pin) & 1;
        }

        /**
         * Get value of one pin basing on datagramBit
         * @param bit datagramBit
         * @return value on pin basing on datagramBit. If not found return false
         */
        [[nodiscard]] bool getValueOnDataBit(datBit_t bit) const {
            for (int pin = 0; pin < gpios; ++pin) {
                if (bitInDatagram[pin] == bit) {
                    return value >> pin & 1;
                }
            }

            //TODO do this better
            return false;
        }

        void setValueOnPin(uint8_t pin, bool newValue) {
            setValueOnBit(value, pin, newValue);
        }

        void setValueOnDataBit(datBit_t bit, bool newValue) const {
            for (int pin = 0; pin < gpios; ++pin) {
                if (bitInDatagram[pin] == bit) {
                    setValueOnPin(pin, newValue);
                    return;
                }
            }
        }
    };

    using ExpanderEp_8 = ExpanderEp<8, uint8_t>;
    using PiPicoExpanderEp = ExpanderEp<24, uint32_t>;

    template<typename TValue, typename TRaw>
    class ValueEp {
    public:
        struct Point {
            TValue value;
            TRaw raw;
        };

        TValue minValue;
        TValue maxValue;

        TRaw minRaw;
        TRaw maxRaw;

        ValueEp() {
            minValue = std::numeric_limits<TValue>::min();
            maxValue = std::numeric_limits<TValue>::max();

            minRaw = std::numeric_limits<TRaw>::min();
            maxRaw = std::numeric_limits<TRaw>::max();
            valuePoints.emplace_back(minValue, minRaw);
            valuePoints.emplace_back(maxValue, maxRaw);
        }

        /**
         * Create with native system min max value. Values in array must be ordered!
         * @return
         */
        ValueEp(const std::vector<Point> &inputPoints) : valuePoints(inputPoints) {
            minValue = std::numeric_limits<TValue>::min();
            maxValue = std::numeric_limits<TValue>::max();

            minRaw = std::numeric_limits<TRaw>::min();
            maxRaw = std::numeric_limits<TRaw>::max();

            if (valuePoints.empty()) {
                valuePoints.emplace_back(minValue, std::numeric_limits<TRaw>::min());
                valuePoints.emplace_back(maxValue, std::numeric_limits<TRaw>::max());
            } else {
                if (valuePoints[0].value != minValue) {
                    valuePoints.insert(valuePoints.begin(), {minValue, std::numeric_limits<TRaw>::min()});
                }

                if (valuePoints[valuePoints.size() - 1].value != maxValue) {
                    valuePoints.emplace_back(minValue, std::numeric_limits<TRaw>::min());
                }
            }
        }

        /**
         * Create with min and max defined by user. Remember to add first value as type min and last value as type max.
         * Values in array must be ordered!
         * @return
         */
        ValueEp(const std::vector<Point> &inputPoints,TRaw minRaw, TRaw maxRaw, TValue minValue = std::numeric_limits<TValue>::min(), TValue maxValue = std::numeric_limits<TValue>::max()) : valuePoints(inputPoints),
            minValue(minValue), maxValue(maxValue), minRaw(minRaw), maxRaw(maxRaw) {
        }

        template <typename TVar>
        [[nodiscard]] TRaw calculateValue(const VarPtrProxy<TVar> &value) {
            return calculateValue(value());
        }

        template <typename TVar>
        [[nodiscard]] TRaw calculateValue(TVar value) {
            if (value <= minValue) {
                return valuePoints[0].raw;
            }

            if (value >= maxValue) {
                return valuePoints[valuePoints.size() - 1].raw;
            }

            for (int i = 1; i < valuePoints.size(); ++i) {
                if (valuePoints[i].value == value) {
                    return valuePoints[i].raw;
                }

                if (valuePoints[i].value > value) {
                    return (value - valuePoints[i - 1].value) * (valuePoints[i].raw - valuePoints[i - 1].raw) / (
                               valuePoints[i].value - valuePoints[i - 1].value) + valuePoints[i - 1].raw;
                }
            }
            return maxRaw;
        }

        [[nodiscard]] constexpr TRaw fromPercentage(uint8_t percentage) const {
            return (static_cast<uint32_t>(maxValue - minValue) * percentage) / 100 + minValue;
        }

    private:
        std::vector<Point> valuePoints; //!< Inside structure <raw value, final value>
    };

    class PwmEp {
    public:
        PwmEp(uint8_t pin, const VarPtrProxy<uint16_t> &raw_value, const std::map<uint16_t, uint16_t> &valuePoints);

        [[nodiscard]] const uint8_t &getPin() const {
            return pin;
        }

        [[nodiscard]] uint16_t calculateValue() const;

        constexpr static uint16_t fromPercentage(uint8_t percentage);

        uint slice;
        uint8_t channel;
        static constexpr uint16_t MAX_VAL = 0xffff;

    private:
        uint8_t pin;
        VarPtrProxy<uint16_t> rawValue;
        std::vector<std::pair<uint16_t, uint16_t> > valuePoints; //!< Inside structure <raw value, final value>
    };

    constexpr uint16_t PwmEp::fromPercentage(uint8_t percentage) {
        //TODO add validation
        return (static_cast<uint32_t>(MAX_VAL) * percentage) / 100;
    }
}

#endif //MUDDC_BASEDEVICESANDENDPOINT_H
