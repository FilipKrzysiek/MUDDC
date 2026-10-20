
#include <pico/stdlib.h>
#include <hardware/i2c.h>

#include "X27StepperDriver.h"
#include "pico/i2c_slave.h"
#include "pico/multicore.h"
#include "pico/status_led.h"

constexpr uint8_t moduleAddress = 0x16;
#define I2C_PORT i2c0
#define I2C_SDA 0
#define I2C_SCL 1
#define I2C_FREQ 400'000

constexpr uint16_t pwmPin_V_LV = 11;
constexpr uint16_t pwmPin_C_LV = 13;
constexpr uint16_t pwmPin_V_HV = 15;
constexpr uint16_t pwmPin_C_TE = 17;
constexpr uint16_t pwmPin_C_LVRef = 18;


auto_init_mutex(m1_mutex);
auto_init_mutex(m2_mutex);
auto_init_mutex(m3_mutex);

static void i2cHandler(i2c_inst_t *i2c, i2c_slave_event_t event);

bool updateMotorsCallback(repeating_timer_t *timer);

PwmPin configPwm(uint16_t pin, pwm_config &config);

void setToPwm(const PwmPin &pwmPin, uint8_t value);

namespace {
    struct Manometers {
        X27StepperDriver m1 = X27StepperDriver(2, 4, 6, 8, &m1_mutex);
        X27StepperDriver m2 = X27StepperDriver(3, 5, 7, 9, &m2_mutex);
        X27StepperDriver m3 = X27StepperDriver(10, 12, 14, 16, &m3_mutex);

        void prepare() {
            m1.prepareOnlyBackFull();
            m2.prepareOnlyBackFull();
            m3.prepareOnlyBackFull();

            sleep_ms(4050);

            m1.prepareOnlyBack();
            m2.prepareOnlyBack();
            m3.prepareOnlyBack();
            sleep_ms(4050);
        }
    };
}

static Manometers mano;
static std::array<uint8_t, 11> receivedData;
static volatile uint8_t receivedCounter = 0;
static volatile bool receivedFinished = false;
repeating_timer timer;

int main() {
    gpio_set_pulls(I2C_SDA, false, false);
    gpio_set_pulls(I2C_SCL, false, false);

    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);

    i2c_init(I2C_PORT, I2C_FREQ);
    i2c_slave_init(I2C_PORT, moduleAddress, i2cHandler);

    auto pwmConfig = pwm_get_default_config();
    pwm_config_set_wrap(&pwmConfig, 255);

    gpio_set_function(pwmPin_V_LV, GPIO_FUNC_PWM);

    PwmPin pwm_LV = configPwm(pwmPin_V_LV, pwmConfig);
    PwmPin pwm_HV = configPwm(pwmPin_V_HV, pwmConfig);
    PwmPin pwm_TE = configPwm(pwmPin_C_TE, pwmConfig);
    PwmPin pwm_C_LV = configPwm(pwmPin_C_LV, pwmConfig);

    gpio_init(pwmPin_C_LVRef);
    gpio_set_dir(pwmPin_C_LVRef, GPIO_OUT);
    gpio_put(pwmPin_C_LVRef, false);

    uint8_t voltLV = 0;
    uint8_t voltHV = 0;
    uint8_t currTE = 0;
    int16_t currLV = 0;

    uint16_t timeToSet = 0;
    uint16_t m1Value = 0;
    uint16_t m2Value = 0;
    uint16_t m3Value = 0;

    add_repeating_timer_us(100, updateMotorsCallback, nullptr, &timer);

    mano.prepare();

    while (true) {
        // if (receivedCounter >= 11) {
        if (receivedFinished) {
            receivedFinished = false;
            if (receivedCounter >= 12) {
                timeToSet = receivedData[0] * 10;
                m1Value = (receivedData[1] << 8) + receivedData[2];
                m2Value = (receivedData[3] << 8) + receivedData[4];
                m3Value = (receivedData[5] << 8) + receivedData[6];

                voltLV = receivedData[7];
                voltHV = receivedData[8];
                currTE = receivedData[9];
                currLV = (receivedData[10] << 8) + receivedData[11];

                mano.m1.goToStepIn(m1Value, timeToSet);
                mano.m2.goToStepIn(m2Value, timeToSet);
                mano.m3.goToStepIn(m3Value, timeToSet);

                setToPwm(pwm_LV, voltLV);
                setToPwm(pwm_HV, voltHV);
                setToPwm(pwm_TE, currTE);

                if (currLV >= 0) {
                    gpio_put(pwmPin_C_LVRef, false);
                } else {
                    gpio_put(pwmPin_C_LVRef, true);
                    currLV = 255 - (-currLV);
                }
                setToPwm(pwm_C_LV, currLV);
            } else {
                //TODO what to do when received not enough data
            }
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
            if (receivedCounter < 12) {
                receivedData[receivedCounter] = i2c_read_byte_raw(i2c);
                receivedCounter += 1;
            } else {
                i2c_read_byte_raw(i2c);
            }
        }
    } else if (event == I2C_SLAVE_REQUEST) {
        // master is requesting data
    } else if (event == I2C_SLAVE_FINISH) {
        receivedFinished = true;
        // master has signalled Stop / Restart
    } else {
    }
}

bool updateMotorsCallback(repeating_timer_t *timer) {
    mano.m1.makeStep();
    mano.m2.makeStep();
    mano.m3.makeStep();

    return true;
}

PwmPin configPwm(uint16_t pin, pwm_config &config) {
    PwmPin pwm;
    pwm.slice = pwm_gpio_to_slice_num(pin);
    pwm.channel = pwm_gpio_to_channel(pin);
    pwm_init(pwm.slice, &config, true);
    return pwm;
}

void setToPwm(const PwmPin &pwmPin, uint8_t value) {
    pwm_set_chan_level(pwmPin.slice, pwmPin.channel, value);
}
