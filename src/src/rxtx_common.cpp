#include "targets.h"
#include "common.h"
#include "config.h"
#include "logging.h"

#include <functional>
#include <Wire.h>

static const int maxDeferredFunctions = 3;

#ifndef GIGLRS_RADIO_BEGIN_ATTEMPTS
#define GIGLRS_RADIO_BEGIN_ATTEMPTS 5
#endif

#ifndef GIGLRS_RADIO_BEGIN_RETRY_DELAY_MS
#define GIGLRS_RADIO_BEGIN_RETRY_DELAY_MS 100
#endif

struct deferred_t {
    unsigned long started;
    unsigned long timeout;
    std::function<void()> function;
};

static deferred_t deferred[maxDeferredFunctions] = {
    {0, 0, nullptr},
    {0, 0, nullptr},
    {0, 0, nullptr},
};

boolean i2c_enabled = false;
static unsigned long rebootTime_Ms = 0;

static void setupWire()
{
    int gpio_scl = GPIO_PIN_SCL;
    int gpio_sda = GPIO_PIN_SDA;

#if defined(TARGET_RX)
    for (int ch = 0 ; ch < GPIO_PIN_PWM_OUTPUTS_COUNT ; ++ch)
    {
        auto pin = GPIO_PIN_PWM_OUTPUTS[ch];
        auto pwm = config.GetPwmChannel(ch);
        // if the PWM pin is nominated as SDA or SCL, and it's not configured for I2C then undef the pins
        if ((pin == GPIO_PIN_SCL && pwm->val.mode != somSCL) || (pin == GPIO_PIN_SDA && pwm->val.mode != somSDA))
        {
            gpio_scl = UNDEF_PIN;
            gpio_sda = UNDEF_PIN;
            break;
        }
        // If I2C pins are not defined in the hardware, then look for configured I2C
        if (GPIO_PIN_SCL == UNDEF_PIN && pwm->val.mode == somSCL)
        {
            gpio_scl = pin;
        }
        if (GPIO_PIN_SCL == UNDEF_PIN && pwm->val.mode == somSDA)
        {
            gpio_sda = pin;
        }
    }
#endif
    if(gpio_sda != UNDEF_PIN && gpio_scl != UNDEF_PIN)
    {
        DBGLN("Starting wire on SCL %d, SDA %d", gpio_scl, gpio_sda);
        // ESP hopes to get Wire::begin(int, int)
        // ESP32 hopes to get Wire::begin(int = -1, int = -1, uint32 = 0)
        Wire.begin(gpio_sda, gpio_scl);
        Wire.setClock(400000);
        i2c_enabled = true;
    }
}

void setupTargetCommon()
{
    setupWire();
}

#if !defined(UNIT_TEST)
bool beginRadioWithRetries(uint32_t minimumFrequency, uint32_t maximumFrequency)
{
    for (uint8_t attempt = 1; attempt <= GIGLRS_RADIO_BEGIN_ATTEMPTS; attempt++)
    {
        if (Radio.Begin(minimumFrequency, maximumFrequency))
        {
            if (attempt > 1)
            {
                DBGLN("RF chipset detected after %u attempts", attempt);
            }
            return true;
        }

        DBGLN("RF chipset detect attempt %u/%u failed", attempt, GIGLRS_RADIO_BEGIN_ATTEMPTS);
        if (attempt < GIGLRS_RADIO_BEGIN_ATTEMPTS)
        {
            delay(GIGLRS_RADIO_BEGIN_RETRY_DELAY_MS);
        }
    }

    return false;
}
#endif

void deferExecutionMicros(unsigned long us, std::function<void()> f)
{
    for (int i=0 ; i<maxDeferredFunctions ; i++)
    {
        if (deferred[i].function == nullptr)
        {
            deferred[i].started = micros();
            deferred[i].timeout = us;
            deferred[i].function = f;
            return;
        }
    }

    // Bail out, there are no slots available!
    DBGLN("No more deferred function slots available!");
}

void executeDeferredFunction(unsigned long now)
{
    // execute deferred function if its time has elapsed
    for (int i=0 ; i<maxDeferredFunctions ; i++)
    {
        if (deferred[i].function != nullptr && (now - deferred[i].started) > deferred[i].timeout)
        {
            deferred[i].function();
            deferred[i].function = nullptr;
        }
    }
}

/***
 * @brief Set a time in milliseconds to reboot the MCU from the main loop thread
 * */
void scheduleRebootTime(unsigned long inMs)
{
    rebootTime_Ms = millis() + inMs;
}

/**
 * @brief Call from the main thread to check if it is time to reboot. May not return.
 */
void checkRebootTime(unsigned long now)
{
    // If the reboot time is set and the current time is past the reboot time then reboot.
    // Wait for any pending config change to be committed first
    if (rebootTime_Ms != 0 && !config.IsModified() && now > rebootTime_Ms ) {
        ESP.restart();
    }
}
