#include "wifi_board.h"
#include "audio_codecs/no_audio_codec.h"
#include "display/oled_display.h"
#include "system_reset.h"
#include "application.h"
#include "button.h"
#include "config.h"
#include "iot/thing_manager.h"
#include "led/single_led.h"
#include "assets/lang_config.h"

#include <wifi_station.h>
#include <esp_log.h>
#include <driver/i2c_master.h>
#include <esp_lcd_panel_ops.h>
#include <esp_lcd_panel_vendor.h>

#include "driver/uart.h"
#include <driver/ledc.h>
#include <driver/rmt_tx.h>

#ifdef SH1106
#include <esp_lcd_panel_sh1106.h>
#endif

#define TAG "CompactWifiBoard"

LV_FONT_DECLARE(font_puhui_14_1);
LV_FONT_DECLARE(font_awesome_14_1);

class CompactWifiBoard : public WifiBoard {
private:
    i2c_master_bus_handle_t display_i2c_bus_;
    esp_lcd_panel_io_handle_t panel_io_ = nullptr;
    esp_lcd_panel_handle_t panel_ = nullptr;
    Display* display_ = nullptr;
    Button boot_button_;
    Button touch_button_;
    Button volume_up_button_;
    Button volume_down_button_;

    uint8_t servoAngle_; // 当前舵机角度值
    int rgbColor_; // 当前 RGB 颜色值
    ledc_channel_config_t servoLedcChannel;

    static const int NUM_LEDS = 3;                         // LED 数量

    rmt_channel_handle_t tx_channel_ = nullptr;
    rmt_encoder_handle_t encoder_ = nullptr;

    void InitializeDisplayI2c() {
        i2c_master_bus_config_t bus_config = {
            .i2c_port = (i2c_port_t)0,
            .sda_io_num = DISPLAY_SDA_PIN,
            .scl_io_num = DISPLAY_SCL_PIN,
            .clk_source = I2C_CLK_SRC_DEFAULT,
            .glitch_ignore_cnt = 7,
            .intr_priority = 0,
            .trans_queue_depth = 0,
            .flags = {
                .enable_internal_pullup = 1,
            },
        };
        ESP_ERROR_CHECK(i2c_new_master_bus(&bus_config, &display_i2c_bus_));
    }

    void InitializeSsd1306Display() {
        // SSD1306 config
        esp_lcd_panel_io_i2c_config_t io_config = {
            .dev_addr = 0x3C,
            .on_color_trans_done = nullptr,
            .user_ctx = nullptr,
            .control_phase_bytes = 1,
            .dc_bit_offset = 6,
            .lcd_cmd_bits = 8,
            .lcd_param_bits = 8,
            .flags = {
                .dc_low_on_data = 0,
                .disable_control_phase = 0,
            },
            .scl_speed_hz = 400 * 1000,
        };

        ESP_ERROR_CHECK(esp_lcd_new_panel_io_i2c_v2(display_i2c_bus_, &io_config, &panel_io_));

        ESP_LOGI(TAG, "Install SSD1306 driver");
        esp_lcd_panel_dev_config_t panel_config = {};
        panel_config.reset_gpio_num = -1;
        panel_config.bits_per_pixel = 1;

        esp_lcd_panel_ssd1306_config_t ssd1306_config = {
            .height = static_cast<uint8_t>(DISPLAY_HEIGHT),
        };
        panel_config.vendor_config = &ssd1306_config;

#ifdef SH1106
        ESP_ERROR_CHECK(esp_lcd_new_panel_sh1106(panel_io_, &panel_config, &panel_));
#else
        ESP_ERROR_CHECK(esp_lcd_new_panel_ssd1306(panel_io_, &panel_config, &panel_));
#endif
        ESP_LOGI(TAG, "SSD1306 driver installed");

        // Reset the display
        ESP_ERROR_CHECK(esp_lcd_panel_reset(panel_));
        if (esp_lcd_panel_init(panel_) != ESP_OK) {
            ESP_LOGE(TAG, "Failed to initialize display");
            display_ = new NoDisplay();
            return;
        }

        // Set the display to on
        ESP_LOGI(TAG, "Turning display on");
        ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(panel_, true));

        display_ = new OledDisplay(panel_io_, panel_, DISPLAY_WIDTH, DISPLAY_HEIGHT, DISPLAY_MIRROR_X, DISPLAY_MIRROR_Y,
            {&font_puhui_14_1, &font_awesome_14_1});
    }

    void InitializeButtons() {
        boot_button_.OnClick([this]() {
            auto& app = Application::GetInstance();
            if (app.GetDeviceState() == kDeviceStateStarting && !WifiStation::GetInstance().IsConnected()) {
                ResetWifiConfiguration();
            }
            app.ToggleChatState();
        });
        touch_button_.OnPressDown([this]() {
            Application::GetInstance().StartListening();
        });
        touch_button_.OnPressUp([this]() {
            Application::GetInstance().StopListening();
        });

        volume_up_button_.OnClick([this]() {
            auto codec = GetAudioCodec();
            auto volume = codec->output_volume() + 10;
            if (volume > 100) {
                volume = 100;
            }
            codec->SetOutputVolume(volume);
            GetDisplay()->ShowNotification(Lang::Strings::VOLUME + std::to_string(volume));
        });

        volume_up_button_.OnLongPress([this]() {
            GetAudioCodec()->SetOutputVolume(100);
            GetDisplay()->ShowNotification(Lang::Strings::MAX_VOLUME);
        });

        volume_down_button_.OnClick([this]() {
            auto codec = GetAudioCodec();
            auto volume = codec->output_volume() - 10;
            if (volume < 0) {
                volume = 0;
            }
            codec->SetOutputVolume(volume);
            GetDisplay()->ShowNotification(Lang::Strings::VOLUME + std::to_string(volume));
        });

        volume_down_button_.OnLongPress([this]() {
            GetAudioCodec()->SetOutputVolume(0);
            GetDisplay()->ShowNotification(Lang::Strings::MUTED);
        });
    }

    // 物联网初始化，添加对 AI 可见设备
    void InitializeIot() {
        auto& thing_manager = iot::ThingManager::GetInstance();
        thing_manager.AddThing(iot::CreateThing("Speaker"));
        //thing_manager.AddThing(iot::CreateThing("Lamp"));

        if (iot::userDev1Enable) thing_manager.AddThing(iot::CreateThing("Servo"));
        if (iot::userDev2Enable) thing_manager.AddThing(iot::CreateThing("RGB"));
    }

    // BLE 模块透传 uart 接口初始化
    void InitializeBleUart() {
        // UART configuration
        const uart_config_t uart_config = {
            .baud_rate = 115200,
            .data_bits = UART_DATA_8_BITS,
            .parity = UART_PARITY_DISABLE,
            .stop_bits = UART_STOP_BITS_1,
            .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        };

        // Configure UART parameters
        ESP_ERROR_CHECK(uart_param_config(UART_NUM_1, &uart_config));

        // Set UART pins
        ESP_ERROR_CHECK(uart_set_pin(UART_NUM_1, GPIO_NUM_17, GPIO_NUM_18, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));

        // Install UART driver
        ESP_ERROR_CHECK(uart_driver_install(UART_NUM_1, 256, 256, 0, NULL, 0));
    }

    // 舵机初始化
    void InitializeServo()
    {
        ESP_LOGI(TAG, "Initializing servo...");

        // 配置 GPIO 为输出模式
        gpio_config_t io_conf = {};
        io_conf.intr_type = GPIO_INTR_DISABLE;
        io_conf.mode = GPIO_MODE_OUTPUT;
        io_conf.pin_bit_mask = (1ULL << GPIO_NUM_14);
        io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
        io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
        esp_err_t ret = gpio_config(&io_conf);
        if (ret != ESP_OK)
        {
            ESP_LOGE(TAG, "Failed to configure GPIO: %s", esp_err_to_name(ret));
            return;
        }
        ESP_LOGI(TAG, "GPIO configured successfully");

        // 配置 LEDC 定时器
        ledc_timer_config_t ledc_timer = {
            .speed_mode = LEDC_LOW_SPEED_MODE,
            .duty_resolution = LEDC_TIMER_14_BIT, // 使用 14 位分辨率
            .timer_num = LEDC_TIMER_2,
            .freq_hz = 50, // 50Hz
            .clk_cfg = LEDC_AUTO_CLK};
        ret = ledc_timer_config(&ledc_timer);
        if (ret != ESP_OK)
        {
            ESP_LOGE(TAG, "Failed to configure LEDC timer: %s", esp_err_to_name(ret));
            return;
        }
        ESP_LOGI(TAG, "LEDC timer configured successfully");

        // 配置 LEDC 通道
        servoLedcChannel = {
            .gpio_num = GPIO_NUM_14,
            .speed_mode = LEDC_LOW_SPEED_MODE,
            .channel = LEDC_CHANNEL_2,
            .intr_type = LEDC_INTR_DISABLE,
            .timer_sel = LEDC_TIMER_2,
            .duty = 0,
            .hpoint = 0};
        ret = ledc_channel_config(&servoLedcChannel);
        if (ret != ESP_OK)
        {
            ESP_LOGE(TAG, "Failed to configure LEDC channel: %s", esp_err_to_name(ret));
            return;
        }
        ESP_LOGI(TAG, "LEDC channel configured successfully");

        ESP_LOGI(TAG, "Servo initialized successfully");

        SetServoAngle(90); // 设置初始角度为 90 度
    }

    void InitializeRGB() {
        ESP_LOGI(TAG, "Initializing RGB with RMT...");
    
        // Create RMT TX channel configuration
        rmt_tx_channel_config_t tx_config = {
            .gpio_num = GPIO_NUM_11,          // WS2812B data pin
            .clk_src = RMT_CLK_SRC_DEFAULT, // Default clock source
            .resolution_hz = 10000000,    // 10 MHz resolution (1 tick = 0.1us)
            .mem_block_symbols = 64,     // Memory block size (must be valid for your hardware)
            .trans_queue_depth = 4,      // Transmission queue depth
            .flags = {
                .invert_out = false,     // Do not invert output
                .with_dma = false,       // Do not use DMA
            },
        };
    
        if (!tx_channel_) {
            ESP_ERROR_CHECK(rmt_new_tx_channel(&tx_config, &tx_channel_));
            ESP_ERROR_CHECK(rmt_enable(tx_channel_));
    
            // Create the encoder
            rmt_copy_encoder_config_t encoder_config = {};
            ESP_ERROR_CHECK(rmt_new_copy_encoder(&encoder_config, &encoder_));
        }

        SendRGBWithRMT(0, 0, 0); // Initialize with black color
    
        ESP_LOGI(TAG, "RMT initialized successfully");
    }

    void SendRGBWithRMT(uint8_t red, uint8_t green, uint8_t blue) {
        const rmt_symbol_word_t bit0 = {
            .duration0 = 3, .level0 = 1,  // T0H: 0.3us
            .duration1 = 7, .level1 = 0   // T0L: 0.7us
        };
        const rmt_symbol_word_t bit1 = {
            .duration0 = 7, .level0 = 1,  // T1H: 0.7us
            .duration1 = 3, .level1 = 0   // T1L: 0.35us
        };
    
        rmt_symbol_word_t symbols[24 * NUM_LEDS];  // Each LED requires 24 bits (GRB format)
    
        int idx = 0;
        for (int i = 0; i < NUM_LEDS; i++) {
            uint8_t colors[3] = {green, red, blue};  // WS2812B uses GRB format
            for (int c = 0; c < 3; c++) {
                for (int bit = 7; bit >= 0; bit--) {
                    symbols[idx++] = (colors[c] & (1 << bit)) ? bit1 : bit0;
                }
            }
        }
    
        // Transmit the RMT symbols using the encoder
        rmt_transmit_config_t tx_config = {
            .loop_count = 0,  // No looping
        };
        ESP_ERROR_CHECK(rmt_transmit(tx_channel_, encoder_, symbols, sizeof(symbols), &tx_config));
    }

public:
    CompactWifiBoard() :
        boot_button_(BOOT_BUTTON_GPIO),
        touch_button_(TOUCH_BUTTON_GPIO),
        volume_up_button_(VOLUME_UP_BUTTON_GPIO),
        volume_down_button_(VOLUME_DOWN_BUTTON_GPIO) {
        InitializeDisplayI2c();
        InitializeSsd1306Display();
        InitializeButtons();
        InitializeIot();
        InitializeBleUart();
        InitializeServo();
        InitializeRGB();
    }

    virtual Led* GetLed() override {
        static SingleLed led(BUILTIN_LED_GPIO);
        return &led;
    }

    virtual AudioCodec* GetAudioCodec() override {
#ifdef AUDIO_I2S_METHOD_SIMPLEX
        static NoAudioCodecSimplex audio_codec(AUDIO_INPUT_SAMPLE_RATE, AUDIO_OUTPUT_SAMPLE_RATE,
            AUDIO_I2S_SPK_GPIO_BCLK, AUDIO_I2S_SPK_GPIO_LRCK, AUDIO_I2S_SPK_GPIO_DOUT, AUDIO_I2S_MIC_GPIO_SCK, AUDIO_I2S_MIC_GPIO_WS, AUDIO_I2S_MIC_GPIO_DIN);
#else
        static NoAudioCodecDuplex audio_codec(AUDIO_INPUT_SAMPLE_RATE, AUDIO_OUTPUT_SAMPLE_RATE,
            AUDIO_I2S_GPIO_BCLK, AUDIO_I2S_GPIO_WS, AUDIO_I2S_GPIO_DOUT, AUDIO_I2S_GPIO_DIN);
#endif
        return &audio_codec;
    }

    virtual Display* GetDisplay() override {
        return display_;
    }

    // 获取当前舵机角度值
    virtual uint8_t GetServoAngle() override {
        return servoAngle_;
    }

    // 设置当前舵机角度值
    virtual void SetServoAngle(uint8_t angle) override
    {
        if (!iot::userDev1Enable) return;

        if (angle > 180)
        {
            angle = 180;
        }
        servoAngle_ = angle;
        ESP_LOGI(TAG, "Set servo angle to %d", servoAngle_);

        // 计算占空比
        uint32_t min_duty = 410;  // 对应 0.5ms
        uint32_t max_duty = 2048; // 对应 2.5ms
        uint32_t duty = min_duty + (angle * (max_duty - min_duty) / 180);

        // 更新占空比
        ESP_ERROR_CHECK(ledc_set_duty(servoLedcChannel.speed_mode, servoLedcChannel.channel, duty));
        ESP_ERROR_CHECK(ledc_update_duty(servoLedcChannel.speed_mode, servoLedcChannel.channel));
    }

    // 获取当前 RGB 颜色值
    virtual int GetRGBColor(void) override
    {
        return rgbColor_;
    }

    // 设置当前 RGB 颜色值
    virtual void SetRGBColor(int color) override
    {
        if (!iot::userDev2Enable) return;

        rgbColor_ = color;

        // Extract RGB components from color value
        uint8_t red = (color >> 16) & 0xFF;
        uint8_t green = (color >> 8) & 0xFF;
        uint8_t blue = (color >> 0) & 0xFF;
        ESP_LOGI(TAG, "Set RGB color to R:%d G:%d B:%d", red, green, blue);

        SendRGBWithRMT(red, green, blue);
    }
};

DECLARE_BOARD(CompactWifiBoard);
