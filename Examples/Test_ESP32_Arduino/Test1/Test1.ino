#include "src/wexcube_sdk/wexcube.h"
#include "src/wexcube_sdk/wexcube_port.h"

// BLE 名称
#define BLE_NAME            "ESP32_Arduino_WEX"

// 服务 UUID
#define SERVICE_UUID        "0000FFE0-0000-1000-8000-00805F9B34FB"

// 通知 UUID
#define NOTIFY_CHARA_UUID   "0000FFE1-0000-1000-8000-00805F9B34FB"

// 写入 UUID
#define WRITE_CHARA_UUID    "0000FFE2-0000-1000-8000-00805F9B34FB"

// 小程序设备控制页面控件ID
#define TEXT1_ID    1   // 文本控件1 用于显示芯片内部温度值
#define TEXT2_ID    2   // 文本控件2 用于显示芯片内部霍尔效应传感器值
#define TEXT3_ID    3   // 文本控件3 用于显示计数值
#define BUTTON1_ID  4   // 按钮控件1 用于重置计数值

int cnt = 0;
int second = 0;

void setup() {
  // 启动 BLE 设备
  Serial.begin(115200);
  
  wex_ble_config(BLE_NAME, SERVICE_UUID, NOTIFY_CHARA_UUID, WRITE_CHARA_UUID);
  wex_init();
  wex_start();

  Serial.println("WeXCube start");
}

void loop() {
  const t_sWexCmd *pWexCmd = wex_process();
  switch (pWexCmd->eCmdType)
  {
    case eWexCmd_Connect:       		// WeXCube 连接指令
    {
      Serial.println("WeXCube connect");

      // 连接成功则初始化设备页面
      wex_setBackColor(TEXT1_ID, eWexColor_Pink);
      wex_setBackRGB(TEXT2_ID, 200, 100, 200);
      wex_setTextColor(TEXT3_ID, eWexColor_Green);
      wex_setText(BUTTON1_ID, "重置");
    }
    break;

    case eWexCmd_Disconnect:        // WeXCube 断开连接指令
    {
      Serial.println("WeXCube disconnect");
    }
    break;

    case eWexCmd_Event:             // 控件事件触发
    {
      Serial.printf("WeXCube event, Ctrl ID is %d\n", pWexCmd->ucCtrlId);
      switch (pWexCmd->ucCtrlId)
      {
        case BUTTON1_ID:
        {
          // 按键按下时
          if (pWexCmd->ucValue == 1)
          {
            second = 0;
            wex_setText(TEXT3_ID, wex_intToStr(second));
          }
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

  // 上传温度值，一秒更新一次
  cnt++;
  if (wex_getConnectState() && (cnt % 100 == 0))
  {
    float temperature = temperatureRead();  // 如果温度值为 53.3 则可能是芯片温度传感器已经被损坏
    int hallValue = hallRead();             // 靠近磁场时值会变化
    wex_setText(TEXT1_ID, wex_floatToStr(temperature, 1));
    wex_setText(TEXT2_ID, wex_intToStr(hallValue));
    Serial.printf("Chip temperature is %.1f\n", temperature);
    Serial.printf("Chip halValue is %d\n", hallValue);

    second = ++second >= 10000 ? 0 : second;
    wex_setText(TEXT3_ID, wex_intToStr(second));
  }

  // 建议添加不小于 2ms 的延时
  delay(10);
}
