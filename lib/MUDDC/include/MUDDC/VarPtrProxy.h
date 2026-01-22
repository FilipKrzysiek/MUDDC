//
// Created by Filip on 18.01.2026.
//

#ifndef MUDDC_VARPTRPROXY_H
#define MUDDC_VARPTRPROXY_H
#include <cstdint>

template<typename TargetType>
class VarPtrProxy {
public:
    explicit VarPtrProxy(uint8_t *source)
        : source(source) {
    }

    explicit VarPtrProxy(const uint8_t *source)
        : source(const_cast<uint8_t *>(source)) {
    }

    TargetType operator()() const {
        TargetType value = *source;
        for (int i = 1; i < sizeof(TargetType); ++i) {
            value += *(source + i) << (i * 8);
        }
        return value;
    }

    constexpr TargetType getValue() const {
        return operator()();
    }

private:
    uint8_t *source;
};


#endif //MUDDC_VARPTRPROXY_H