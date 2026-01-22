//
// Created by Filip on 22.01.2026.
//

#ifndef MUDDC_BITFUNCTIONS_H
#define MUDDC_BITFUNCTIONS_H
#include <cstdint>

template<typename T>
constexpr void setValueOnBit(T& destination, uint8_t bit, bool value) {
    if (value == true) {
        destination |= (1 << bit);
    } else {
        destination &= ~(1 << bit);
    }
}

template<typename T>
constexpr bool getValueOnBit(T source, uint8_t bit) {
    return (source & (1 << bit));
}
#endif //MUDDC_BITFUNCTIONS_H
