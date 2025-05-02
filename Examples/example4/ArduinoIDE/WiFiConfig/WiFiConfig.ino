#include <Arduino.h>
#include <WiFi.h>
#include <Preferences.h>

#include "src/wexcube_sdk/wexcube.h"
#include "src/wexcube_sdk/wexcube_port.h"

// BLE 名称
// #define BLE_NAME            "ESP32_WiFi_Config"

// 服务 UUID
#define SERVICE_UUID        "0000FFE0-0000-1000-8000-00805F9B34FB"

// 通知 UUID
#define NOTIFY_CHARA_UUID   "0000FFE1-0000-1000-8000-00805F9B34FB"

// 写入 UUID
#define WRITE_CHARA_UUID    "0000FFE2-0000-1000-8000-00805F9B34FB"

// 小程序设备控制页面控件ID
#define DEVICE_NAME_TEXT_ID         1     // 设备连接名称文本ID
#define DEVICE_MAC_TEXT_ID          2     // 设备MAC地址文本ID
#define WIFI_CON_STATUS_TEXT_ID     3     // WiFi连接状态文本ID
#define WIFI_CON_NAME_TEXT_ID       4     // WiFi连接名称文本ID
#define DEVICE_IP_TEXT_ID           5     // 设备IP文本ID
#define DISCONNECT_BUTTON_ID        10    // 断开WiFi连接按钮ID
#define RECONNECT_BUTTON_ID         11    // 重新WiFi连接按钮ID
#define CONFIG_STATUS_TEXT_ID       20    // 配置状态文本ID
#define WIFI_NAME_TEXT_ID           21    // WiFi名称文本ID
#define WIFI_PASSWORD_TEXT_ID       22    // WiFi密码文本ID
#define UPDATE_WIFI_BUTTON_ID       30    // 更新WiFi配置按钮ID

// WiFi configuration
const char* defaultSsid = "Test_WiFi";
const char* defaultPassword = "123456";

String ssid = "";
String password = "";;
String ssidInput = "";;
String passwordInput = "";


wl_status_t WiFiLastStatus = WL_IDLE_STATUS;
const char* WiFiStatusStr[] = {
  "连接中...",
  "未找到WiFi",
  "扫描完成",
  "已连接",
  "连接失败",
  "连接丢失",
  "已断开"
};

Preferences preferences;


uint8_t configStatus = 0; // 0: 等待配置, 1: WiFi名称为空, 2: WiFi密码为空, 3: 配置中, 4: 配置成功, 5: 配置失败
const char* configStatusStr[] = {
  "等待配置",
  "WiFi名称不能为空",
  "WiFi密码不能为空",
  "配置中...",
  "配置成功",
  "配置失败"
};

void wex_ble_loop();


void setup() {
  // 启动 BLE 设备
  Serial.begin(115200);

  // 初始化 Preferences flash 存储
  preferences.begin("wifi-config", false);
  if (!preferences.isKey("ssid") || !preferences.isKey("password")) {
    preferences.putString("ssid", defaultSsid);
    preferences.putString("password", defaultPassword);
    Serial.println("Default SSID and password saved to flash.");
  }

  // 从 flash 中读取 SSID 和 password
  ssid = preferences.getString("ssid", defaultSsid);
  password = preferences.getString("password", defaultPassword);

  // 启动 WiFi 连接
  WiFi.begin(ssid, password);
  Serial.println("Connecting to WiFi");

  // 启动 BLE 服务
#ifdef BLE_NAME
  wex_ble_config(BLE_NAME, SERVICE_UUID, NOTIFY_CHARA_UUID, WRITE_CHARA_UUID);
#else
  wex_ble_config(String(WiFi.getHostname()), SERVICE_UUID, NOTIFY_CHARA_UUID, WRITE_CHARA_UUID);
#endif
  wex_init();
  wex_start();
  Serial.println("WeXCube start");
}

void loop() {
  wex_ble_loop();
}

// WeXCube 处理
void wex_ble_loop() {
  const t_sWexCmd *psWexCmd = wex_process();
  switch (psWexCmd->eCmdType)
  {
    case eWexCmd_Connect:       		// WeXCube 连接指令
    {
      Serial.println("WeXCube connect");

      // WeXCube连接成功则初始化设备页面
      wex_setText(DEVICE_NAME_TEXT_ID, WiFi.getHostname());
      wex_setText(DEVICE_MAC_TEXT_ID, WiFi.macAddress().c_str());
      wex_setText(WIFI_CON_NAME_TEXT_ID, ssid.c_str());

      WiFiLastStatus = WiFi.status();
      wex_setText(WIFI_CON_STATUS_TEXT_ID, WiFiStatusStr[WiFiLastStatus]);
      if (WiFiLastStatus == WL_CONNECTED)
      {

        wex_setTextColor(WIFI_CON_STATUS_TEXT_ID, eWexColor_Green);
        wex_setText(DEVICE_IP_TEXT_ID, WiFi.localIP().toString().c_str());
      }
      else
      {
        if (WiFiLastStatus == WL_IDLE_STATUS)
        {
          wex_setTextColor(WIFI_CON_STATUS_TEXT_ID, eWexColor_Gray1);
        }
        else
        {
          wex_setTextColor(WIFI_CON_STATUS_TEXT_ID, eWexColor_Red);
        }
        wex_setText(DEVICE_IP_TEXT_ID, "0.0.0.0");
      }


      passwordInput = "";
      configStatus = 0;
      wex_setText(CONFIG_STATUS_TEXT_ID,configStatusStr[configStatus]);
      wex_setText(WIFI_NAME_TEXT_ID, ssidInput.c_str());
      wex_setText(WIFI_PASSWORD_TEXT_ID, passwordInput.c_str());
    }
    break;

    case eWexCmd_Disconnect:        // WeXCube 断开连接指令
    {
      Serial.println("WeXCube disconnect");
    }
    break;

    case eWexCmd_Event:             // 控件事件触发
    {
      Serial.printf("WeXCube event, Ctrl ID is %d\n", psWexCmd->ucCtrlId);
      switch (psWexCmd->ucCtrlId)
      {
        case DISCONNECT_BUTTON_ID:  // 断开WiFi连接按钮
        {
          if (psWexCmd->ucValue) WiFi.disconnect();
        }
        break;

        case RECONNECT_BUTTON_ID:   // 重新WiFi连接按钮
        {
          if (psWexCmd->ucValue) WiFi.reconnect();
        }
        break;

        case UPDATE_WIFI_BUTTON_ID: // 更新WiFi配置按钮
        {
          if (psWexCmd->ucValue)
          {
            if (ssidInput.length() == 0)
            {
              configStatus = 1;
            }
            else if (passwordInput.length() == 0)
            {
              configStatus = 2;
            }
            else
            {
              configStatus = 3;
              wex_setText(CONFIG_STATUS_TEXT_ID, configStatusStr[configStatus]);
              WiFi.disconnect();
              delay(1000);

              WiFi.begin(ssidInput.c_str(), passwordInput.c_str());
              configStatus = 4;
              preferences.putString("ssid", ssidInput);
              preferences.putString("password", passwordInput);
              ssid = ssidInput;
              password = passwordInput;
              wex_setText(WIFI_CON_NAME_TEXT_ID, ssid.c_str());
              wex_setText(WIFI_NAME_TEXT_ID, ssidInput.c_str());
              wex_setText(WIFI_PASSWORD_TEXT_ID, passwordInput.c_str());
            }
          }
          wex_setText(CONFIG_STATUS_TEXT_ID, configStatusStr[configStatus]);
        }
        break;

        default:
        break;
      }
    }
    break;

    case eWexCmd_Text:              // 控件文本
    {
      Serial.printf("WeXCube text, Ctrl ID is %d\n", psWexCmd->ucCtrlId);
      switch (psWexCmd->ucCtrlId)
      {
        case WIFI_NAME_TEXT_ID:     // WiFi名称文本框
        {
          ssidInput = String(psWexCmd->pcText);
        }
        break;

        case WIFI_PASSWORD_TEXT_ID: // WiFi密码文本框
        {
          passwordInput = String(psWexCmd->pcText);
        }
        break;

        default:
        break;
      }
    }
    break;

    default:
    break;
  }

  // 实时监测WiFi连接状态
  if (WiFiLastStatus != WiFi.status())
  {
    WiFiLastStatus = WiFi.status();
    wex_setText(WIFI_CON_STATUS_TEXT_ID, WiFiStatusStr[WiFiLastStatus]);
    if (WiFiLastStatus == WL_CONNECTED)
    {
      wex_setTextColor(WIFI_CON_STATUS_TEXT_ID, eWexColor_Green);
      wex_setText(DEVICE_IP_TEXT_ID, WiFi.localIP().toString().c_str());
    }
    else
    {
      if (WiFiLastStatus == WL_IDLE_STATUS)
      {
        wex_setTextColor(WIFI_CON_STATUS_TEXT_ID, eWexColor_Gray1);
      }
      else
      {
        wex_setTextColor(WIFI_CON_STATUS_TEXT_ID, eWexColor_Red);
      }
      wex_setText(DEVICE_IP_TEXT_ID, "0.0.0.0");
    }
  }
}
