
#include <pico/stdlib.h>
#include <hardware/i2c.h>

#include "X27StepperDriver.h"
#include "pico/i2c_slave.h"
#include "pico/status_led.h"

constexpr uint8_t moduleAddress = 0x16;
#define I2C_PORT i2c0
#define I2C_SDA 0
#define I2C_SCL 1
#define I2C_FREQ 200'000

static void i2cHandler(i2c_inst_t *i2c, i2c_slave_event_t event);

namespace {
    struct Manometers {
        X27StepperDriver m1 = X27StepperDriver(2, 4, 6, 8);
        X27StepperDriver m2 = X27StepperDriver(3, 5, 7, 9);
        X27StepperDriver m3 = X27StepperDriver(10, 12, 14, 16);

        void prepare() {
            m1.prepareOnlyBackSlow0(4, 2);
            m2.prepareOnlyBackSlow0(4,2 );
            m3.prepareOnlyBackSlow0(4, 2);
        }
    };
}

static Manometers mano;
static std::array<uint8_t, 7> receivedData;
static volatile uint8_t receivedCounter = 0;

int main() {
    gpio_set_pulls(I2C_SDA, false, false);
    gpio_set_pulls(I2C_SCL, false, false);

    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);

    i2c_init(I2C_PORT, I2C_FREQ);
    i2c_slave_init(I2C_PORT, moduleAddress, i2cHandler);

    mano.prepare();
    while (true) {
        if (receivedCounter >= 7) {
            uint16_t timeToSet = receivedData[0] * 10;
            uint16_t m1Value = (receivedData[1] << 8) + receivedData[2];
            uint16_t m2Value = (receivedData[3] << 8) + receivedData[4];
            uint16_t m3Value = (receivedData[5] << 8) + receivedData[6];

            mano.m1.goToStepIn(m1Value, timeToSet);
            mano.m2.goToStepIn(m2Value, timeToSet);
            mano.m3.goToStepIn(m3Value, timeToSet);
            receivedCounter = 0;
        }
        tight_loop_contents();
    }
    return 0;
}

void i2cHandler(i2c_inst_t *i2c, i2c_slave_event_t event) {
    if (event == I2C_SLAVE_RECEIVE) {
        // master has written some data
        while (i2c_get_read_available(i2c)) {
            if (receivedCounter < 7) {
                receivedData[receivedCounter] = i2c_read_byte_raw(i2c);
                receivedCounter += 1;
            } else {
                i2c_read_byte_raw(i2c);
            }
        }
    } else if (event == I2C_SLAVE_REQUEST) {
        // master is requesting data
    } else if (event == I2C_SLAVE_FINISH) {
        // master has signalled Stop / Restart
    } else {
    }
}