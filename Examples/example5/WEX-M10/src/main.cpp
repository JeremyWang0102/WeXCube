#include <Arduino.h>

#include "wexcube_sdk/wexcube.h"
#include "wexcube_sdk/wexcube_port.h"

// BLE 名称前缀
#define BLE_NAME_PRE        "WEX-M10-"
// 服务 UUID
#define SERVICE_UUID        "0000FF10-0000-1000-8000-00805F9B34FB"
// 通知 UUID
#define NOTIFY_CHARA_UUID   "0000FF11-0000-1000-8000-00805F9B34FB"
// 写入 UUID
#define WRITE_CHARA_UUID    "0000FF12-0000-1000-8000-00805F9B34FB"

// 小程序设备控制页面控件ID，该ID与GPIO引脚对应
#define SWITCH1_ID          1     // 1号引脚控件ID
#define SWITCH2_ID          2     // 2号引脚控件ID
#define SWITCH3_ID          3     // 3号引脚控件ID
#define SWITCH4_ID          4     // 4号引脚控件ID
#define SWITCH5_ID          5     // 5号引脚控件ID
#define SWITCH6_ID          6     // 6号引脚控件ID
#define SWITCH7_ID          7     // 7号引脚控件ID
#define SWITCH10_ID         10    // 10号引脚控件ID
#define SWITCH20_ID         20    // 20号引脚控件ID
#define SWITCH21_ID         21    // 21号引脚控件ID
#define TEXT_ID             9     // 该文本控件用于显示0脚引脚电平状态


#define OUTPUT_CTRL_PIN     0     // 上电输出引脚默认输出电平控制引脚
#define WEX_LED_PIN         8     // WeXCube 指示灯引脚

wex_u8_t askHandshakeCnt = 0;     // 心跳包计数
bool wexConnectFlag = false;      // WeXCube 连接标志

hw_timer_t *timer = NULL;         // 定时器对象
volatile bool timerFlag = false;  // 共享变量（标志位）

bool defaultPinLevel = HIGH;     // 默认引脚电平

// 定时器中断函数（必须使用 IRAM_ATTR）
void IRAM_ATTR onTimer() {
  timerFlag = true;
}


void wex_ble_loop();


void setup() {
  // 降低到 80MHz
  setCpuFrequencyMhz(80);

  // 延时一段时间等待稳定
  delay(500);

  // 启动 BLE 设备
  Serial.begin(115200);

  // 启动 BLE 服务
  uint64_t chipid = ESP.getEfuseMac();
  char macStr[20];  
  sprintf(macStr, "%02x%02x%02x%02x%02x%02x", 
                  (uint8_t)(chipid >> 0),
                  (uint8_t)(chipid >> 8),
                  (uint8_t)(chipid >> 16),
                  (uint8_t)(chipid >> 24),
                  (uint8_t)(chipid >> 32),
                  (uint8_t)(chipid >> 40));
  wex_ble_config(BLE_NAME_PRE + String(macStr).substring(6), SERVICE_UUID, NOTIFY_CHARA_UUID, WRITE_CHARA_UUID);
  wex_init();
  wex_start();
  Serial.println("WeXCube start");

  // 初始化 GPIO 为输出
  pinMode(SWITCH1_ID, OUTPUT);
  pinMode(SWITCH2_ID, OUTPUT);
  pinMode(SWITCH3_ID, OUTPUT);
  pinMode(SWITCH4_ID, OUTPUT);
  pinMode(SWITCH5_ID, OUTPUT);
  pinMode(SWITCH6_ID, OUTPUT);
  pinMode(SWITCH7_ID, OUTPUT);
  pinMode(SWITCH10_ID, OUTPUT);
  pinMode(SWITCH20_ID, OUTPUT);
  pinMode(SWITCH21_ID, OUTPUT);

  // 初始化 GPIO 为输入
  pinMode(OUTPUT_CTRL_PIN, INPUT_PULLDOWN);

  // GPIO0 为上电输出引脚默认输出电平
  if (digitalRead(OUTPUT_CTRL_PIN) == HIGH) {
    digitalWrite(SWITCH1_ID, HIGH);
    digitalWrite(SWITCH2_ID, HIGH);
    digitalWrite(SWITCH3_ID, HIGH);
    digitalWrite(SWITCH4_ID, HIGH);
    digitalWrite(SWITCH5_ID, HIGH);
    digitalWrite(SWITCH6_ID, HIGH);
    digitalWrite(SWITCH7_ID, HIGH);
    digitalWrite(SWITCH10_ID, HIGH);
    digitalWrite(SWITCH20_ID, HIGH);
    digitalWrite(SWITCH21_ID, HIGH);

    defaultPinLevel = HIGH;
  } else {
    digitalWrite(SWITCH1_ID, LOW);
    digitalWrite(SWITCH2_ID, LOW);
    digitalWrite(SWITCH3_ID, LOW);
    digitalWrite(SWITCH4_ID, LOW);
    digitalWrite(SWITCH5_ID, LOW);
    digitalWrite(SWITCH6_ID, LOW);
    digitalWrite(SWITCH7_ID, LOW);
    digitalWrite(SWITCH10_ID, LOW);
    digitalWrite(SWITCH20_ID, LOW);
    digitalWrite(SWITCH21_ID, LOW);

    defaultPinLevel = LOW;
  }

  // WeXCube 指示灯
  pinMode(WEX_LED_PIN, OUTPUT);
  digitalWrite(WEX_LED_PIN, HIGH);

  // 初始化定时器 (定时器编号 0, 分频系数 80, 递增模式)
  timer = timerBegin(0, 80, true);  // 80MHz / 80 = 1MHz（1us）
  // 绑定定时器中断回调
  timerAttachInterrupt(timer, &onTimer, true);
  // 设置定时器（1000000us = 1秒，重复触发）
  timerAlarmWrite(timer, 1000000, true);
  // 启动定时器
  timerAlarmEnable(timer);
}

void loop() {
  wex_ble_loop();

  // 定时器中断处理
  if (timerFlag) {
    timerFlag = false;
    
    // 如果 WeXCube 连接成功则执行心跳包
    if (wexConnectFlag) {
      wex_askHandshake();

      // 心跳包计数超过3次则认为 WeXCube 断开连接
      askHandshakeCnt++;
      if (askHandshakeCnt > 3) {
        askHandshakeCnt = 0;

        Serial.println("WeXCube disconnect");
        digitalWrite(WEX_LED_PIN, HIGH);

        wexConnectFlag = false;
      }
    }
  }
}

// WeXCube 处理
void wex_ble_loop() {
  const t_sWexCmd *psWexCmd = wex_process();
  switch (psWexCmd->eCmdType)
  {
    case eWexCmd_Connect:       		// WeXCube 连接指令
    {
      Serial.println("WeXCube connect");
      digitalWrite(WEX_LED_PIN, LOW);

      wexConnectFlag = true;

      // WeXCube连接成功则初始化设备页面
      wex_setText(TEXT_ID, defaultPinLevel ? "高电平" : "低电平");
      wex_setValue(SWITCH1_ID, digitalRead(SWITCH1_ID) == defaultPinLevel ? 0 : 1);
      wex_setValue(SWITCH2_ID, digitalRead(SWITCH2_ID) == defaultPinLevel ? 0 : 1);
      wex_setValue(SWITCH3_ID, digitalRead(SWITCH3_ID) == defaultPinLevel ? 0 : 1);
      wex_setValue(SWITCH4_ID, digitalRead(SWITCH4_ID) == defaultPinLevel ? 0 : 1);
      wex_setValue(SWITCH5_ID, digitalRead(SWITCH5_ID) == defaultPinLevel ? 0 : 1);
      wex_setValue(SWITCH6_ID, digitalRead(SWITCH6_ID) == defaultPinLevel ? 0 : 1);
      wex_setValue(SWITCH7_ID, digitalRead(SWITCH7_ID) == defaultPinLevel ? 0 : 1);
      wex_setValue(SWITCH10_ID, digitalRead(SWITCH10_ID) == defaultPinLevel ? 0 : 1);
      wex_setValue(SWITCH20_ID, digitalRead(SWITCH20_ID) == defaultPinLevel ? 0 : 1);
      wex_setValue(SWITCH21_ID, digitalRead(SWITCH21_ID) == defaultPinLevel ? 0 : 1);
    }
    break;

    case eWexCmd_Disconnect:        // WeXCube 断开连接指令
    {
      Serial.println("WeXCube disconnect");
      digitalWrite(WEX_LED_PIN, HIGH);

      askHandshakeCnt = 0;
      wexConnectFlag = false;
    }
    break;

    case eWexCmd_Handshake:         // WeXCube 握手指令回应
    {
      askHandshakeCnt = 0;
    }
    break;

    case eWexCmd_Event:             // 控件事件触发
    {
      Serial.printf("WeXCube event, Ctrl ID is %d\n", psWexCmd->ucCtrlId);
      switch (psWexCmd->ucCtrlId)
      {
        case SWITCH1_ID:
        case SWITCH2_ID:
        case SWITCH3_ID:
        case SWITCH4_ID:
        case SWITCH5_ID:
        case SWITCH6_ID:
        case SWITCH7_ID:
        case SWITCH10_ID:
        case SWITCH20_ID:
        case SWITCH21_ID:
        {
          // 控件事件触发，设置对应引脚电平
          if (psWexCmd->ucValue) {
            digitalWrite(psWexCmd->ucCtrlId, defaultPinLevel ? LOW : HIGH);
          } else {
            digitalWrite(psWexCmd->ucCtrlId, defaultPinLevel ? HIGH : LOW);
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
}
