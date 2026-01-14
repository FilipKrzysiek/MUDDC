//
// Created by Filip on 07.09.2025.
//

#include "RawData.h"

bool RawData::getIndicatorState(Indicators indicator) {
    return (dataInStruct.indicatorGroup0 >> indicator) & 1;
}

void RawData::setSwitchState(uint8_t bit, bool state) {
    if (state) {
        dataOutStruct.switchGroup0 |= (1 << bit);
    } else {
        dataOutStruct.switchGroup0 &= ~(1 << bit);
    }
}

std::pair<uint8_t *, int> RawData::getInDataAsPointer() {
    return {reinterpret_cast<uint8_t*>(&dataInStruct), sizeof(dataInStruct) };
}

std::pair<uint8_t *, int> RawData::getOutDataAsPointer() {
    return {reinterpret_cast<uint8_t*>(&dataOutStruct), sizeof(dataOutStruct) };
}

bool RawData::dataInIsOk() const {
    return preambleValue == *reinterpret_cast<uint32_t*>(*dataInStruct.preamble);
}
