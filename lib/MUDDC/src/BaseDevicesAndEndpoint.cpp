//
// Created by Filip on 22.01.2026.
//

#include "MUDDC/BaseDevicesAndEndpoint.h"

#include "MUDDC/bitFunctions.h"

bde::PwmEp::PwmEp(uint8_t pin, const VarPtrProxy<uint16_t> &raw_value,
                    const std::map<uint16_t, uint16_t> &valuePoints) : slice(0), channel(0),
                                                                       pin(pin),
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

uint16_t bde::PwmEp::calculateValue() const {
    for (int i = 1; i < valuePoints.size(); ++i) {
        if (valuePoints[i].first == rawValue()) {
            return valuePoints[i].second;
        }

        if (valuePoints[i].first > rawValue()) {
            return (valuePoints[i].second - valuePoints[i - 1].second) / (
                       valuePoints[i].first - valuePoints[i - 1].first) * rawValue();
        }
    }
    return MAX_VAL;
}

