//
// Created by Filip on 14.09.2025.
//

#ifndef MUDDC_MASTER_DATAGRAMOUT_H
#define MUDDC_MASTER_DATAGRAMOUT_H
#include <array>
#include <cstdint>


/**
 * Datagram send to Maszyna (PC)
 * @author Krzysztof Swałdek
 */
class DatagramOut {
public:
    explicit DatagramOut() {
        rawData[0] = 0xEF;
        rawData[1] = 0xEF;
        rawData[2] = 0xEF;
        rawData[3] = 0xEF;

        for (int i = 4; i < rawData.size(); i++) {
            rawData[i] = 0;
        }
    }

    /**
     * Raw datagram data access
     * @return data access
     */
    [[nodiscard]] const uint8_t *data();

    [[nodiscard]] const uint8_t *data() const;

    /**
     * Get raw data datagram size
     * @return size of raw data
     */
    [[nodiscard]] constexpr uint32_t size() const;

    /**
     * Set switch state in datagram
     * @param index bit of value in datagram calculated from first switch byte.
     * For switch byte 0 bit 0 `index` wil be 0, for byte 0, bit  5, `index` will be 5, for byte 1 bit 1 `index` will be
     * 9. Use own declared enum to keep this values!
     * @param state state to set
     */
    void setSwitchState(uint8_t index, bool state);

    /**
     * Set master controller value
     * @param value
     */
    void setMasterController(uint8_t value);

    void setSecondController(uint8_t value);

    void setTrainBrake(uint16_t value);

    void setIndependentBrake(uint16_t value);
private:
    std::array<uint8_t, 20> rawData{};
};

constexpr uint32_t DatagramOut::size() const {
    return rawData.size();
}

inline const uint8_t * DatagramOut::data() {
    return rawData.data();
}

inline const uint8_t * DatagramOut::data() const {
    return rawData.data();
}




#endif //MUDDC_MASTER_DATAGRAMOUT_H
