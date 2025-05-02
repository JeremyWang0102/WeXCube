#include <esp_log.h>
#include <esp_err.h>
#include <nvs.h>
#include <nvs_flash.h>
#include <driver/gpio.h>
#include <esp_event.h>

#include "application.h"
#include "system_info.h"

#include "settings.h"

#define TAG "main"

namespace iot
{
    std::string userDev1Name;
    bool userDev1Enable;

    std::string userDev2Name;
    bool userDev2Enable;
}

extern "C" void app_main(void)
{
    // Initialize the default event loop
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    // Initialize NVS flash for WiFi configuration
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_LOGW(TAG, "Erasing NVS flash to fix corruption");
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    Settings settings("userData", false);
    iot::userDev1Name = settings.GetString("userDev1Name", "舵机");
    iot::userDev2Name = settings.GetString("userDev2Name", "氛围灯");
    iot::userDev1Enable = settings.GetInt("userDev1Enable", 0);
    iot::userDev2Enable = settings.GetInt("userDev2Enable", 0);

    // Launch the application
    Application::GetInstance().Start();
    // The main thread will exit and release the stack memory
}
