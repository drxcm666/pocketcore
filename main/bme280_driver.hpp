#pragma once

#include "driver/i2c_master.h"

#include <optional>
#include <cstdint>
#include <span>

struct Bme280Measurement
{
    float temperature;
    float pressure;
    float humidity;
};

class Bme280Driver
{
private:
    i2c_master_bus_handle_t i2c_bus_handle_;
    i2c_master_dev_handle_t i2c_dev_handle_{nullptr};

    std::uint16_t dig_T1{};
    std::int16_t dig_T2{};
    std::int16_t dig_T3{};

    std::uint16_t dig_P1{};
    std::int16_t dig_P2{};
    std::int16_t dig_P3{};
    std::int16_t dig_P4{};
    std::int16_t dig_P5{};
    std::int16_t dig_P6{};
    std::int16_t dig_P7{};
    std::int16_t dig_P8{};
    std::int16_t dig_P9{};

    std::uint8_t dig_H1{};
    std::int16_t dig_H2{};
    std::uint8_t dig_H3{};
    std::int16_t dig_H4{};
    std::int16_t dig_H5{};
    std::int8_t dig_H6{};

    esp_err_t read_register(const std::uint8_t reg_addr, std::span<std::uint8_t> output);
    esp_err_t sensor_is_measuring(bool &measuring);
    bool wait_for_measurement();
    std::int32_t read_tfine(std::int32_t raw_temp);

public:
    Bme280Driver(i2c_master_bus_handle_t &i2c_bus_handle);
    esp_err_t init();
    std::optional<Bme280Measurement> read();

    ~Bme280Driver();
};