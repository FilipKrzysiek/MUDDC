//
// Created by Filip on 14.09.2025.
//

#include "MUDDC/DatagramOut.h"

void DatagramOut::setSwitchState(uint8_t index, bool state) {
    if (state) {
        rawData[index / 8 + 4] |= (1 << (index % 8));
    } else {
        rawData[index / 8 + 4] &= ~(1 << (index % 8));
    }
}

void DatagramOut::setMasterController(uint8_t value) {
    rawData[10] = value;
}

void DatagramOut::setSecondController(uint8_t value) {
    rawData[11] = value;
}

void DatagramOut::setTrainBrake(uint16_t value) {
    *reinterpret_cast<uint16_t*>(&rawData[12]) = value;
}

void DatagramOut::setIndependentBrake(uint16_t value) {
    *reinterpret_cast<uint16_t*>(&rawData[14]) = value;
}
