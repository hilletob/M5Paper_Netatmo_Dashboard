#include "battery.h"

void Battery::init() {
    // Set ADC attenuation to 11dB for 0-3.3V range
    analogSetAttenuation(ADC_11db);
    ESP_LOGI("battery", "Battery monitor initialized on pin A%d", BATTERY_PIN);
}

uint32_t Battery::readVoltage() {
    // Read ADC value (averaged over multiple readings for stability)
    uint32_t adcSum = 0;
    const int samples = 10;

    for (int i = 0; i < samples; i++) {
        adcSum += analogReadMilliVolts(BATTERY_PIN);
        delay(10);
    }

    uint32_t adcMv = adcSum / samples;

    // XIAO has a 2:1 voltage divider (two 200kΩ resistors)
    // So we need to double the ADC reading to get actual battery voltage
    uint32_t batteryMv = adcMv * 2;

    ESP_LOGD("battery", "ADC: %u mV, Battery: %u mV", adcMv, batteryMv);

    return batteryMv;
}

uint8_t Battery::readPercentage() {
    uint32_t voltageMv = readVoltage();

    // Map battery voltage to percentage
    // LiPo battery range: 3.3V (empty) to 4.35V (full)
    // Using linear mapping (not perfectly accurate but simple)

    if (voltageMv >= BATTERY_MAX_MV) {
        return 100;
    }

    if (voltageMv <= BATTERY_MIN_MV) {
        return 0;
    }

    // Linear interpolation
    float percentage = (float)(voltageMv - BATTERY_MIN_MV) /
                      (float)(BATTERY_MAX_MV - BATTERY_MIN_MV) * 100.0f;

    // Clamp to 0-100 range
    if (percentage < 0) percentage = 0;
    if (percentage > 100) percentage = 100;

    return (uint8_t)percentage;
}

void Battery::read(uint32_t& voltageMv, uint8_t& percent) {
    voltageMv = readVoltage();

    // Calculate percentage from voltage
    if (voltageMv >= BATTERY_MAX_MV) {
        percent = 100;
    } else if (voltageMv <= BATTERY_MIN_MV) {
        percent = 0;
    } else {
        float percentage = (float)(voltageMv - BATTERY_MIN_MV) /
                          (float)(BATTERY_MAX_MV - BATTERY_MIN_MV) * 100.0f;
        percent = (uint8_t)percentage;
    }

    ESP_LOGI("battery", "%u mV, %u%%", voltageMv, percent);
}
