#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/uart.h"

#include <string_view>
#include <array>
#include <algorithm>

#include "display.hpp"
#include "button_driver.hpp"
#include "application.hpp"
#include "test_app.hpp"
#include "test_app_2.hpp"
#include "application_manager.hpp"
#include "gpio_app.hpp"
#include "i2c_scanner_app.hpp"
#include "uart_driver.hpp"
#include "uart_app.hpp"

#include "driver/sdspi_host.h"
#include "sdmmc_cmd.h"
#include "stdio.h"
#include "esp_vfs_fat.h"
#include "diskio_sdmmc.h"
#include <string>

#include "scoped_file.hpp"

static const char *TAG{"PocketCore"};

enum class LogLevel
{
    info,
    warning,
    error,
};

void writeLog(LogLevel level, const std::string &str)
{
    std::string lvl{};
    if (level == LogLevel::info)
    {
        lvl = "[INFO]";
    }
    else if (level == LogLevel::warning)
    {
        lvl = "[WARNING]";
    }
    else if (level == LogLevel::error)
    {
        lvl = "[ERROR]";
    }

    std::string msg = lvl + " " + str + '\n';

    ScopedFile file{"/sdcard/logs.txt", "a"};
    if (file.is_open())
    {
        int fp = fputs(msg.c_str(), file.get());

        if (fp != EOF)
            ESP_LOGI(TAG, "Write successful");
        else
            ESP_LOGE(TAG, "Write failed");
    }
    else
    {
        ESP_LOGE(TAG, "Failed to open file for writing");
    }
}

const char *to_string(Button btn)
{
    switch (btn)
    {
    case Button::up:
        return "UP";
    case Button::down:
        return "DOWN";
    case Button::left:
        return "LEFT";
    case Button::right:
        return "RIGHT";
    case Button::ok:
        return "OK";
    case Button::back:
        return "BACK";
    default:
        return "UNKNOWN_BTN";
    }
}

const char *to_string(ButtonEventType type)
{
    switch (type)
    {
    case ButtonEventType::press:
        return "PRESS";
    case ButtonEventType::release:
        return "RELEASE";
    case ButtonEventType::long_press:
        return "LONG_PRESS";
    default:
        return "UNKNOWN_EVENT";
    }
}

constexpr std::size_t menu_size{11};

struct MenuItem
{
    std::string_view name;
};

struct MenuParameters
{
    int text_x = 20;
    int text_y = 20;
    int padding = 3;
    int scale = 2;
};

void move_down(std::size_t &selected_index)
{
    if (selected_index == (menu_size - 1))
    {
        selected_index = 0;
        return;
    }

    selected_index++;
}

void move_up(std::size_t &selected_index)
{
    if (selected_index == 0)
    {
        selected_index = (menu_size - 1);
        return;
    }

    selected_index--;
}

constexpr std::array<MenuItem, menu_size> menu_items = {
    MenuItem{"GPIO Tools"},
    MenuItem{"UART Terminal"},
    MenuItem{"I2C Scanner"},
    MenuItem{"Infrared"},
    MenuItem{"NFC"},
    MenuItem{"Sub-GHz"},
    MenuItem{"BLE Scanner"},
    MenuItem{"Wi-Fi Analyzer"},
    MenuItem{"Files"},
    MenuItem{"System Info"},
    MenuItem{"Settings"},
};

void draw_selected_item(Display &display, const MenuParameters &menu, std::size_t i)
{
    int text_width = menu_items[i].name.size() * 6 * menu.scale;
    int text_height = 7 * menu.scale;

    display.fill_rect(0xFFFF, menu.text_x - menu.padding, menu.text_y - menu.padding + (i * 25),
                      menu.text_x + text_width - 1 + menu.padding,
                      menu.text_y + text_height - 1 + menu.padding + (i * 25));

    display.draw_text(0x0000, menu.text_x, menu.text_y + (i * 25), menu.scale, menu_items[i].name);
}

void draw_default_item(Display &display, const MenuParameters &menu, std::size_t i)
{
    int text_width = menu_items[i].name.size() * 6 * menu.scale;
    int text_height = 7 * menu.scale;

    display.fill_rect(0x0000, menu.text_x - menu.padding, menu.text_y - menu.padding + (i * 25),
                      menu.text_x + text_width - 1 + menu.padding,
                      menu.text_y + text_height - 1 + menu.padding + (i * 25));

    display.draw_text(0xFFFF, menu.text_x, menu.text_y + (i * 25), menu.scale, menu_items[i].name);
}

void draw_menu(Display &display, const MenuParameters &menu, std::size_t selected_index)
{
    display.fill_screen(0x0000);

    for (std::size_t i = 0; i < menu_size; i++)
    {
        if (i == selected_index)
        {
            draw_selected_item(display, menu, i);

            continue;
        }

        draw_default_item(display, menu, i);
    }
}

void draw_startup_screen(Display &display)
{
    display.fill_screen(0x0000);
    display.draw_text(0xFFFF, 50, 150, 2, "POCKETCORE\nv0.1");
}

extern "C" void app_main()
{

    std::size_t selected_index{0};
    std::size_t prev_index{0};

    ButtonDriver buttons;
    buttons.init();

    Display display{
        GPIO_NUM_11,
        GPIO_NUM_12,
        GPIO_NUM_10,
        GPIO_NUM_9,
        GPIO_NUM_8,
        40'000'000};

    esp_err_t init_result = display.init();
    if (init_result != ESP_OK)
    {
        ESP_LOGE("PocketCore", "Display init failed: %s", esp_err_to_name(init_result));
        return;
    }

    ESP_LOGI(TAG, "System started! Initial selected menu: %zu", selected_index);

    draw_startup_screen(display);
    vTaskDelay(pdMS_TO_TICKS(1000));

    MenuParameters menu{};
    draw_menu(display, menu, selected_index);

    spi_bus_config_t bus_cfg{};
    bus_cfg.mosi_io_num = GPIO_NUM_16;
    bus_cfg.miso_io_num = GPIO_NUM_21;
    bus_cfg.sclk_io_num = GPIO_NUM_15;
    bus_cfg.quadhd_io_num = -1;
    bus_cfg.quadwp_io_num = -1;
    esp_err_t result = spi_bus_initialize(SPI3_HOST, &bus_cfg, SPI_DMA_CH_AUTO);
    if (result != ESP_OK)
    {
        ESP_LOGE(TAG, "SPI initialization failed: %s", esp_err_to_name(result));
        return;
    }

    /* Create the SD-over-SPI device configuration with safe default values.
       This describes the SD card as one particular device connected to SPI */
    sdspi_device_config_t sd_device_cfg = SDSPI_DEVICE_CONFIG_DEFAULT();
    sd_device_cfg.host_id = SPI3_HOST;
    sd_device_cfg.gpio_cs = GPIO_NUM_47;

    // This handle will identify SD card device after it is attached to the SPI bus
    sdspi_dev_handle_t sdspi_handle{};

    // Attach the SD card as an SPI device to the already initialized SPI3 bus
    result = sdspi_host_init_device(&sd_device_cfg, &sdspi_handle);
    if (result != ESP_OK)
    {
        ESP_LOGE(TAG, "SD SPI host initialization failed: %s", esp_err_to_name(result));
        return;
    }

    /* Create the interface used by the SD-card protocol driver.
       It contains the functions needed to communicate with an SD card over SPI */
    sdmmc_host_t sd_host = SDSPI_HOST_DEFAULT();

    // Tell the SD protocol driver exactly which SPI SD device it should use
    sd_host.slot = sdspi_handle;

    /* This structure will be filled with information about the real SD card:
       capacity, type, supported speed, sector size, etc */
    sdmmc_card_t sd_card{};

    /* Communicate with the physical SD card, identify it and initialize it.
       After this succeeds, ESP32 can work with raw sectors of the card.
       The filesystem is NOT mounted yet */
    result = sdmmc_card_init(&sd_host, &sd_card);
    if (result != ESP_OK)
    {
        ESP_LOGE(TAG, "SD card initialization failed: %s", esp_err_to_name(result));
        return;
    }
    sdmmc_card_print_info(stdout, &sd_card);

    // Configure how the FAT (File Allocation Table) filesystem will be exposed to program
    esp_vfs_fat_conf_t fat_cfg{};
    fat_cfg.base_path = "/sdcard";
    fat_cfg.fat_drive = "0:";
    fat_cfg.max_files = 3;

    // Pointer to the FatFs (open-source FAT filesystem library) filesystem object
    FATFS *fs = nullptr;

    /* Register the FAT filesystem with ESP-IDF's file interface.
       This prepares the "/sdcard/..." path and creates the FATFS object,
       but it still does NOT read or mount the filesystem from the SD card */
    result = esp_vfs_fat_register(&fat_cfg, &fs);
    if (result != ESP_OK)
    {
        ESP_LOGE(TAG, "FATFS registration failed: %s", esp_err_to_name(result));
        return;
    }

    /* Connect FatFs drive 0 to already initialized physical SD card.
       From now on, when FatFs asks to read/write a sector on drive 0,
       ESP-IDF knows that the operation must go to sd_card */
    ff_diskio_register_sdmmc(0, &sd_card);

    /* Mount the actual FAT filesystem stored on the SD card.
       This is the point where FatFs actually reads the card
       and tries to understand its filesystem */
    FRESULT res = f_mount(fs, "0:", 1);
    if (res == FR_OK)
    {
        ESP_LOGI(TAG, "SD Card mounted successfully! Ready to read/write");
    }
    else if (res == FR_NO_FILESYSTEM)
    {
        ESP_LOGE(TAG, "Card found, but it's not formatted as FAT32/exFAT");
        return;
    }
    else
    {
        ESP_LOGE(TAG, "Failed to mount. Error code: %d", res);
        return;
    }

    GpioApp gpio_app{display};
    I2cScannerApp i2c_app{display};
    UartDriver uart_driver{115200, UART_NUM_1, GPIO_NUM_17, GPIO_NUM_18,
                           UART_DATA_8_BITS, UART_PARITY_DISABLE, UART_STOP_BITS_1};
    uart_driver.init();
    UartTerminalApp uart_app{display, uart_driver};

    ApplicationManager manager{};

    while (true)
    {
        buttons.update();

        while (true)
        {
            auto event = buttons.pop_event();
            if (!event.has_value())
                break;

            if (manager.has_active_app())
            {
                if (event->button == Button::back &&
                    event->type == ButtonEventType::press)
                {
                    manager.close();
                    draw_menu(display, menu, selected_index);
                }
                else
                {
                    manager.process(event.value());
                }
            }
            else
            {
                if (event->button == Button::down &&
                    event->type == ButtonEventType::press)
                {
                    prev_index = selected_index;
                    move_down(selected_index);
                    ESP_LOGI(TAG, "selected: %zu", selected_index);

                    draw_default_item(display, menu, prev_index);
                    draw_selected_item(display, menu, selected_index);
                }
                else if (event->button == Button::up &&
                         event->type == ButtonEventType::press)
                {
                    prev_index = selected_index;
                    move_up(selected_index);
                    ESP_LOGI(TAG, "selected: %zu", selected_index);

                    draw_default_item(display, menu, prev_index);
                    draw_selected_item(display, menu, selected_index);
                }
                else if (event->button == Button::ok &&
                         event->type == ButtonEventType::press &&
                         selected_index == 0)
                {
                    // manager.open(test);
                    manager.open(gpio_app);
                }
                else if (event->button == Button::ok &&
                         event->type == ButtonEventType::press &&
                         selected_index == 1)
                {
                    manager.open(uart_app);
                }
                else if (event->button == Button::ok &&
                         event->type == ButtonEventType::press &&
                         selected_index == 2)
                {
                    manager.open(i2c_app);
                }
            }
        }

        manager.update();

        vTaskDelay(pdMS_TO_TICKS(10));
    }
}
