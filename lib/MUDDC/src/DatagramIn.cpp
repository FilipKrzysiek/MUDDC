//
// Created by Filip on 14.09.2025.
//

#include "MUDDC/DatagramIn.h"

bool DatagramIn::preambleIsValid() const {
    return rawData[0] == 0xEF && rawData[1] == 0xEF && rawData[2] == 0xEF && rawData[3] == 0xEF;
}

const uint16_t & DatagramIn::tacho() {
    return *reinterpret_cast<const uint16_t *>(&rawData[4]);
}

bool DatagramIn::indicatorState(Indicators indicatorIndex) const {
    return rawData[6 + indicatorIndex / 8] >> (indicatorIndex % 8) & 1;
}

VarPtrProxy<uint16_t> DatagramIn::breakPress() const {
    return VarPtrProxy<uint16_t>(&rawData[11]);
}

VarPtrProxy<uint16_t> DatagramIn::pipePress() const {
    return VarPtrProxy<uint16_t>(&rawData[13]);
}

VarPtrProxy<uint16_t> DatagramIn::tankPress() const {
    return VarPtrProxy<uint16_t>(&rawData[15]);
}

VarPtrProxy<uint16_t> DatagramIn::hvVoltage() const {
    return VarPtrProxy<uint16_t>(&rawData[17]);
}

VarPtrProxy<uint16_t> DatagramIn::hvCurrent1() const {
    return VarPtrProxy<uint16_t>(&rawData[19]);
}

VarPtrProxy<uint16_t> DatagramIn::hvCurrent2() const {
    return VarPtrProxy<uint16_t>(&rawData[21]);
}

VarPtrProxy<uint16_t> DatagramIn::hvCurrent3() const {
    return VarPtrProxy<uint16_t>(&rawData[23]);
}

uint8_t DatagramIn::year() const {
    const uint16_t &months = *reinterpret_cast<const uint16_t *>(&rawData[25]);
    return months / 12;
}

uint8_t DatagramIn::month() const {
    const uint16_t &months = *reinterpret_cast<const uint16_t *>(&rawData[25]);
    return months % 12;
}

uint8_t DatagramIn::day() const {
    const uint16_t &minutes = *reinterpret_cast<const uint16_t *>(&rawData[27]);
    return minutes / 24 / 60;
}

uint8_t DatagramIn::hour() const {
    const uint16_t &minutes = *reinterpret_cast<const uint16_t *>(&rawData[27]);
    return (minutes / 60) % 24;
}

uint8_t DatagramIn::minute() const {
    const uint16_t &minutes = *reinterpret_cast<const uint16_t *>(&rawData[27]);
    return minutes % 60;
}

uint8_t DatagramIn::second() const {
    const uint16_t &msecs = *reinterpret_cast<const uint16_t *>(&rawData[29]);
    return msecs / 1000;
}

uint8_t DatagramIn::millisecond() const {
    const uint16_t &msecs = *reinterpret_cast<const uint16_t *>(&rawData[29]);
    return msecs % 1000;
}

VarPtrProxy<uint32_t> DatagramIn::odometer() const {
    return VarPtrProxy<uint32_t>(&rawData[31]);
}

VarPtrProxy<uint16_t> DatagramIn::lvVoltage() const {
    return VarPtrProxy<uint16_t>(&rawData[35]);
}

const uint8_t &DatagramIn::radioChanel() const {
    return rawData[37];
}

VarPtrProxy<uint16_t> DatagramIn::pantographPress() const {
    return VarPtrProxy<uint16_t>(&rawData[38]);
}
