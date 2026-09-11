#include "bme280_driver.hpp"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#include <limits>
#include <array>

static const char *TAG{"PocketCore | Bme280Driver"};

Bme280Driver::Bme280Driver(i2c_master_bus_handle_t &i2c_bus_handle) : i2c_bus_handle_{i2c_bus_handle} {}

esp_err_t Bme280Driver::read_register(const std::uint8_t reg_addr, std::span<std::uint8_t> output)
{
    auto err = i2c_master_transmit_receive(i2c_dev_handle_, &reg_addr, 1, output.data(), output.size_bytes(), 50);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "I2C transaction failed: %s", esp_err_to_name(err));
        return err;
    }

    return ESP_OK;
}

esp_err_t Bme280Driver::sensor_is_measuring(bool &measuring)
{
    std::uint8_t reg_status_addr{0xF3};
    std::uint8_t status_data{};
    auto err = read_register(reg_status_addr, std::span<std::uint8_t>{&status_data, 1});
    if (err != ESP_OK)
    {
        return err;
    }

    measuring = ((status_data >> 3) & 1) != 0;

    return ESP_OK;
}

bool Bme280Driver::wait_for_measurement()
{
    bool measuring{true};
    bool measurement_ready{false};

    for (int attempt = 0; attempt < 20; attempt++)
    {
        auto err = sensor_is_measuring(measuring);

        if (err != ESP_OK)
        {
            ESP_LOGE(TAG, "Failed to read measurement status: %s", esp_err_to_name(err));
            return measurement_ready;
        }

        if (!measuring)
        {
            measurement_ready = true;
            return measurement_ready;
        }

        vTaskDelay(pdMS_TO_TICKS(2));
    }

    return measurement_ready;
}

std::int32_t Bme280Driver::read_tfine(std::int32_t raw_temp)
{
    std::int32_t var1, var2;

    var1 = ((((raw_temp >> 3) - (static_cast<std::int32_t>(dig_T1) << 1))) * (static_cast<std::int32_t>(dig_T2))) >> 11;

    var2 = (((((raw_temp >> 4) - (static_cast<std::int32_t>(dig_T1))) * ((raw_temp >> 4) - (static_cast<std::int32_t>(dig_T1)))) >> 12) *
            (static_cast<std::int32_t>(dig_T3))) >>
           14;

    return var1 + var2;
}

std::optional<Bme280Measurement> Bme280Driver::read()
{
    Bme280Measurement measurement{};

    // osrs_t = 001; osrs_p = 001; mode = 01; == 0x25
    std::uint8_t ctrl_meas_pt[2] = {0xF4, 0x25};
    auto err = i2c_master_transmit(i2c_dev_handle_, ctrl_meas_pt, 2, 50);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "I2C transaction failed: %s", esp_err_to_name(err));
        return std::nullopt;
    }

    bool measurement_ready = wait_for_measurement();
    if (!measurement_ready)
    {
        ESP_LOGE(TAG, "Measurement timeout");
        return std::nullopt;
    }

    std::uint8_t reg_data_addr{0xF7};
    std::array<std::uint8_t, 8> read_data;

    err = read_register(reg_data_addr, read_data);
    if (err != ESP_OK)
    {
        return std::nullopt;
    }

    std::uint8_t msb_press = read_data[0];
    std::uint8_t lsb_press = read_data[1];
    std::uint8_t xlsb_press = read_data[2];
    std::int32_t raw_press = (static_cast<std::uint32_t>(msb_press) << 12) |
                             (static_cast<std::uint32_t>(lsb_press) << 4) |
                             (static_cast<std::uint32_t>(xlsb_press) >> 4);

    std::uint8_t msb_temp = read_data[3];
    std::uint8_t lsb_temp = read_data[4];
    std::uint8_t xlsb_temp = read_data[5];
    std::int32_t raw_temp = (static_cast<std::int32_t>(msb_temp) << 12) |
                            (static_cast<std::int32_t>(lsb_temp) << 4) |
                            (static_cast<std::int32_t>(xlsb_temp) >> 4);

    std::uint8_t msb_hum = read_data[6];
    std::uint8_t lsb_hum = read_data[7];
    std::int32_t raw_hum = (static_cast<std::int32_t>(msb_hum) << 8) |
                           (static_cast<std::int32_t>(lsb_hum));

    const std::int32_t t_fine = read_tfine(raw_temp);

    // temperature calculation

    std::int32_t T = (t_fine * 5 + 128) >> 8;

    float temp = T / 100.0f;

    ESP_LOGI(TAG, "Temperature: %.2f", temp);

    measurement.temperature = temp;

    //---------

    // pressure calculation

    std::int32_t var1, var2;
    std::uint32_t press;

    var1 = ((static_cast<std::int32_t>(t_fine)) >> 1) - static_cast<std::int32_t>(64000);

    var2 = (((var1 >> 2) * (var1 >> 2)) >> 11) * (static_cast<std::int32_t>(dig_P6));

    var2 = var2 + ((var1 * (static_cast<std::int32_t>(dig_P5))) << 1);

    var2 = (var2 >> 2) + ((static_cast<std::int32_t>(dig_P4)) << 16);

    var1 = (((dig_P3 *
              (((var1 >> 2) * (var1 >> 2)) >> 13)) >>
             3) +
            (((static_cast<std::int32_t>(dig_P2)) * var1) >> 1)) >>
           18;

    var1 = ((((32768 + var1)) *
             (static_cast<std::int32_t>(dig_P1))) >>
            15);

    if (var1 == 0)
    {
        return std::nullopt;
    }

    press = (static_cast<std::uint32_t>(((static_cast<std::int32_t>(1048576) - raw_press) -
                                         (var2 >> 12)))) *
            3125;

    if (press < 0x80000000)
    {
        press = (press << 1) / (static_cast<std::uint32_t>(var1));
    }
    else
    {
        press = (press / static_cast<std::uint32_t>(var1)) * 2;
    }

    var1 = ((static_cast<std::int32_t>(dig_P9)) *
            (static_cast<std::int32_t>((((press >> 3) * (press >> 3)) >> 13)))) >>
           12;

    var2 = ((static_cast<std::int32_t>((press >> 2))) *
            (static_cast<std::int32_t>(dig_P8))) >>
           13;

    press = static_cast<std::uint32_t>((static_cast<std::int32_t>(press) +
                                        ((var1 + var2 + dig_P7) >> 4)));

    ESP_LOGI(TAG, "Pressure: %.2f", static_cast<float>(press));

    measurement.pressure = press;

    //---------

    // humidity calculation

    double hum;

    hum = ((static_cast<double>(t_fine)) - 76800.0);

    hum = (raw_hum - ((static_cast<double>(dig_H4)) * 64.0 +
                      (static_cast<double>(dig_H5)) / 16384.0 * hum)) *
          ((static_cast<double>(dig_H2)) / 65536.0 *
           (1.0 + (static_cast<double>(dig_H6)) / 67108864.0 * hum *
                      (1.0 + (static_cast<double>(dig_H3)) / 67108864.0 * hum)));

    hum = hum *
          (1.0 - ((double)dig_H1) * hum / 524288.0);

    if (hum > 100.0)
        hum = 100.0;
    else if (hum < 0.0)
        hum = 0.0;

    ESP_LOGI(TAG, "Humidity: %.2f", static_cast<float>(hum));

    measurement.humidity = hum;

    //---------

    return measurement;
}

esp_err_t Bme280Driver::init()
{
    i2c_device_config_t i2c_dev_cfg{};
    i2c_dev_cfg.dev_addr_length = I2C_ADDR_BIT_LEN_7;
    i2c_dev_cfg.device_address = 0x76;
    i2c_dev_cfg.scl_speed_hz = 100'000;

    auto err = i2c_master_bus_add_device(i2c_bus_handle_, &i2c_dev_cfg, &i2c_dev_handle_);
    if (err != ESP_OK)
    {
        return err;
    }

    std::uint8_t reg_id_addr{0xD0};
    std::uint8_t read_id{};

    err = read_register(reg_id_addr, std::span<std::uint8_t>{&read_id, 1});
    if (err != ESP_OK)
    {
        return err;
    }

    ESP_LOGI(TAG, "Read id: 0x%02X", read_id);
    if (read_id == 0x60)
    {
        ESP_LOGI(TAG, "BME280 detected");
    }
    else
    {
        ESP_LOGI(TAG, "Another device detected");
        return err;
    }

    std::uint8_t reg_temp_coeff_addr{0x88};
    std::array<std::uint8_t, 6> temp_coeff;
    err = read_register(reg_temp_coeff_addr, temp_coeff);
    if (err != ESP_OK)
    {
        return err;
    }
    dig_T1 = (std::uint16_t)(temp_coeff[1] << 8) | (std::uint16_t)(temp_coeff[0]);
    dig_T2 = static_cast<std::int16_t>((std::uint16_t)(temp_coeff[3] << 8) | (std::uint16_t)(temp_coeff[2]));
    dig_T3 = static_cast<std::int16_t>((std::uint16_t)(temp_coeff[5] << 8) | (std::uint16_t)(temp_coeff[4]));

    std::uint8_t reg_press_coeff_addr{0x8E};
    std::array<std::uint8_t, 18> press_coeff;
    err = read_register(reg_press_coeff_addr, press_coeff);
    if (err != ESP_OK)
    {
        return err;
    }
    dig_P1 = static_cast<std::uint16_t>(press_coeff[1] << 8) |
             static_cast<std::uint16_t>(press_coeff[0]);
    dig_P2 = static_cast<std::int16_t>(static_cast<std::uint16_t>(press_coeff[3] << 8) |
                                       (static_cast<std::uint16_t>(press_coeff[2])));

    dig_P3 = static_cast<std::int16_t>(static_cast<std::uint16_t>(press_coeff[5] << 8) |
                                       (static_cast<std::uint16_t>(press_coeff[4])));

    dig_P4 = static_cast<std::int16_t>(static_cast<std::uint16_t>(press_coeff[7] << 8) |
                                       (static_cast<std::uint16_t>(press_coeff[6])));

    dig_P5 = static_cast<std::int16_t>(static_cast<std::uint16_t>(press_coeff[9] << 8) |
                                       (static_cast<std::uint16_t>(press_coeff[8])));

    dig_P6 = static_cast<std::int16_t>(static_cast<std::uint16_t>(press_coeff[11] << 8) |
                                       (static_cast<std::uint16_t>(press_coeff[10])));

    dig_P7 = static_cast<std::int16_t>(static_cast<std::uint16_t>(press_coeff[13] << 8) |
                                       (static_cast<std::uint16_t>(press_coeff[12])));

    dig_P8 = static_cast<std::int16_t>(static_cast<std::uint16_t>(press_coeff[15] << 8) |
                                       (static_cast<std::uint16_t>(press_coeff[14])));

    dig_P9 = static_cast<std::int16_t>(static_cast<std::uint16_t>(press_coeff[17] << 8) |
                                       (static_cast<std::uint16_t>(press_coeff[16])));

    std::uint8_t reg_hum_h1_coeff_addr{0xA1};
    err = read_register(reg_hum_h1_coeff_addr, std::span<std::uint8_t>{&dig_H1, 1});
    if (err != ESP_OK)
    {
        return err;
    }
    std::uint8_t reg_hum_coeff_addr{0xE1};
    std::array<std::uint8_t, 7> hum_coeff;
    err = read_register(reg_hum_coeff_addr, hum_coeff);
    if (err != ESP_OK)
    {
        return err;
    }
    dig_H2 = static_cast<std::int16_t>(static_cast<std::uint16_t>(hum_coeff[1] << 8) |
                                       (static_cast<std::uint16_t>(hum_coeff[0])));

    dig_H3 = hum_coeff[2];

    std::uint8_t lower_bits = hum_coeff[4] & 0b00001111;
    dig_H4 = static_cast<std::int16_t>((static_cast<std::uint16_t>(hum_coeff[3] << 4) |
                                        (static_cast<std::uint16_t>(lower_bits)))
                                       << 4) >>
             4;

    dig_H5 = static_cast<std::int16_t>(((static_cast<std::uint16_t>(hum_coeff[5] << 4)) |
                                        (static_cast<std::uint16_t>(hum_coeff[4]) >> 4))
                                       << 4) >>
             4;

    dig_H6 = static_cast<std::int8_t>(hum_coeff[6]);

    // osrs_h = 001; == 0x01
    std::uint8_t ctrl_meas_h[2] = {0xF2, 0x01};
    err = i2c_master_transmit(i2c_dev_handle_, ctrl_meas_h, 2, 50);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "I2C transaction failed: %s", esp_err_to_name(err));
        return err;
    }

    return ESP_OK;
}

Bme280Driver::~Bme280Driver()
{
    if (i2c_dev_handle_ != nullptr)
    {
        i2c_master_bus_rm_device(i2c_dev_handle_);
    }
}