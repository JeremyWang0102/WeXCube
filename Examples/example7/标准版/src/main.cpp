#include <Arduino.h>
#include <ESP32Servo.h>   // 包含 ESP32Servo 库
#include <Adafruit_NeoPixel.h> // 包含 Adafruit_NeoPixel 库

#include "wexcube_sdk/wexcube.h"
#include "wexcube_sdk/wexcube_port.h"

// BLE 名称前缀
#define BLE_NAME_PRE        "WEX-RobotDog-"

// 小程序设备控制页面控件ID
#define BUTTON_EYES_COLOR1_ID   1   // 眼睛颜色1
#define BUTTON_EYES_COLOR2_ID   2   // 眼睛颜色2
#define BUTTON_EYES_COLOR3_ID   3   // 眼睛颜色3
#define BUTTON_EYES_COLOR4_ID   4   // 眼睛颜色4
#define BUTTON_EYES_COLOR5_ID   5   // 眼睛颜色5
#define BUTTON_EYES_COLOR6_ID   6   // 眼睛颜色6
#define SLIDER_EYES_LEVEL_ID    7   // 眼睛亮度
#define BUTTON_EYES_MODE1_ID    8   // 眼睛模式1
#define BUTTON_EYES_MODE2_ID    9   // 眼睛模式2
#define BUTTON_EYES_MODE3_ID    10  // 眼睛模式3 

#define SLIDER_LEFTHAND_ID      11  // 左手舵机
#define SLIDER_RIGHTHAND_ID     12  // 右手舵机
#define SLIDER_LEFTLEG_ID       13  // 左脚舵机
#define SLIDER_RIGHTLEG_ID      14  // 右脚舵机  

#define BUTTON_FORWARD_ID       15  // 前进
#define BUTTON_LEFT_ID          16  // 左转
#define BUTTON_RIGHT_ID         17  // 右转
#define BUTTON_BACKWARD_ID      18  // 后退  

#define BUTTON_STANDUP_ID       19  // 站立
#define BUTTON_SITDOWN_ID       20  // 趴下
#define BUTTON_LEFTRIGHT_ID     21  // 左右摇晃
#define BUTTON_FRONTBACK_ID     22  // 前后摇晃

#define SLIDER_MOVE_SPEED_ID    23  // 动作速度


// 引脚定义
#define WEX_LED_PIN             8   // WeXCube 指示灯引脚
#define EYES_CTRL_PIN           6   // 眼睛控制引脚
#define LEFTHAND_CTRL_PIN       0   // 左手舵机控制引脚
#define RIGHTHAND_CTRL_PIN      1   // 右手舵机控制引脚
#define LEFTLEG_CTRL_PIN        10  // 左脚舵机控制引脚
#define RIGHTLEG_CTRL_PIN       7   // 右脚舵机控制引脚

#define EYES_NUM                2   // 眼睛灯带数量

#define THERSHOLD_ANGLE         60  // 舵机角度变化阈值


// 灯光模式枚举
enum lightModel_t
{
  LIGHT_MODEL_NORMAL = 0,   // 正常模式
  LIGHT_MODEL_BREATHING,    // 呼吸模式
  LIGHT_MODEL_COLORFUL      // 彩灯模式
};

// 动作模式枚举
enum actionModel_t
{
  ACTION_MODEL_FORWARD = 1,   // 向前
  ACTION_MODEL_LEFT,          // 向左
  ACTION_MODEL_RIGHT,         // 向右
  ACTION_MODEL_BACKWARD,      // 后退
  ACTION_MODEL_STANDUP,       // 站立
  ACTION_MODEL_SITDOWN,       // 趴下
  ACTION_MODEL_LEFTRIGHT,     // 左右摇晃
  ACTION_MODEL_FRONTBACK      // 前后摇晃
};


Servo leftHandServo;                // 左手舵机对象
Servo rightHandServo;               // 右手舵机对象
Servo leftLegServo;                 // 左脚舵机对象
Servo rightLegServo;                // 右脚舵机对象
int leftHandAngle = 0;              // 左手舵机角度
int rightHandAngle = 180;           // 右手舵机角度
int leftLegAngle = 0;               // 左脚舵机角度
int rightLegAngle = 180;            // 右脚舵机角度
wex_u8_t actionModel = 0;           // 动作模式（enum actionModel_t）
wex_u8_t moveSpeed = 1;             // 动作速度（1~5）
wex_u8_t stepFlag = 0;

Adafruit_NeoPixel strip(EYES_NUM, EYES_CTRL_PIN, NEO_GRB + NEO_KHZ800);

wex_u8_t lightLevel = 0;
wex_u8_t lightColor = 0;
wex_u8_t lightModel = 0; // 0 - 正常，1 - 呼吸，2 - 彩灯
wex_u8_t ledRGB[3] = {255, 255, 255}; // 眼睛灯光颜色（RGB）

wex_u8_t askHandshakeCnt = 0;       // 心跳包计数
bool wexConnectFlag = false;        // WeXCube 连接标志

hw_timer_t *timer = NULL;           // 定时器对象
volatile bool timerFlag = false;    // 共享变量（标志位）

// 定时器中断函数（必须使用 IRAM_ATTR）
void IRAM_ATTR onTimer() {
  timerFlag = true;
}

void wex_ble_loop();
void updateLED(wex_u8_t ucRed, wex_u8_t ucGreen, wex_u8_t ucBlue, wex_u8_t ucLevel);
void updateServo(wex_u8_t ucLeftHand, wex_u8_t ucRightHand, wex_u8_t ucLeftLeg, wex_u8_t ucRightLeg);
void selectEyesColor(wex_u8_t id); // 选择眼睛颜色
void selectEyesModel(wex_u8_t id); // 选择眼睛模式


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
  wex_ble_config(BLE_NAME_PRE + String(macStr).substring(6));
  wex_init();
  wex_start();
  Serial.println("WeXCube start");

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
  
  // 设置 PWM 输出频率为 50Hz（标准舵机控制频率，周期为20ms）
  leftHandServo.setPeriodHertz(50);
  rightHandServo.setPeriodHertz(50);
  leftLegServo.setPeriodHertz(50);
  rightLegServo.setPeriodHertz(50);
  
  // 附加舵机到指定引脚，并设置脉宽范围（500us ~ 2500us）
  // 500us ~ 2500us 对应舵机角度范围 0° ~ 180°（具体范围根据舵机型号和实际情况调整）
  leftHandServo.attach(LEFTHAND_CTRL_PIN, 500, 2500);
  rightHandServo.attach(RIGHTHAND_CTRL_PIN, 500, 2500);
  leftLegServo.attach(LEFTLEG_CTRL_PIN, 500, 2500);
  rightLegServo.attach(RIGHTLEG_CTRL_PIN, 500, 2500);

  // 设置舵机初始角度
  updateServo(leftHandAngle, rightHandAngle, leftLegAngle, rightLegAngle);

  strip.begin();
  strip.setBrightness(lightLevel); // 亮度初始值
  strip.show();           // 清空灯带
}

void loop() {
  wex_ble_loop();

  // 延时 10ms，避免蓝牙通信阻塞
  delay(10);

  // 眼睛灯光控制
  if (lightLevel)
  {
    if (lightModel == LIGHT_MODEL_NORMAL) // 正常模式
    {
      static wex_u32_t cnt = 0;
      cnt++;
      // 10 * 10 = 100ms 更新一次值
      if (cnt >= 10)
      {
        cnt = 0;
        updateLED(ledRGB[0], ledRGB[1], ledRGB[2], lightLevel);
      }
    }
    else if (lightModel == LIGHT_MODEL_BREATHING) // 呼吸模式
    {
      static wex_u8_t level = 0;
      static wex_u8_t dir = 0;
      if (dir)
      {
        if (level >= lightLevel)
          dir = 0;
        else
          level++;
      }
      else
      {
        if (level == 0)
          dir = 1;
        else
          level--;
      }
      updateLED(ledRGB[0], ledRGB[1], ledRGB[2], level);
    }
    else if (lightModel == LIGHT_MODEL_COLORFUL) // 彩灯模式
    {
      static long firstPixelHue = 0;

      // 彩虹渐变效果
      firstPixelHue += 256;
      if (firstPixelHue >= 65536)
        firstPixelHue = 0;
      for (int i = 0; i < strip.numPixels(); i++)
      {
        int pixelHue = firstPixelHue + (i * 65536L / strip.numPixels());
        strip.setPixelColor(i, strip.gamma32(strip.ColorHSV(pixelHue)));
      }
      strip.show();
    }
  }

  // 舵机动作控制
  switch (actionModel)
  {
    case ACTION_MODEL_FORWARD: // 前进
    {
      if (stepFlag == 0)
      {
        leftHandAngle -= moveSpeed;
        rightLegAngle += moveSpeed;
        
        if (leftHandAngle <= THERSHOLD_ANGLE) stepFlag = 1;
      }
      else if (stepFlag == 1)
      {
        rightHandAngle -= moveSpeed;
        leftLegAngle += moveSpeed;
        
        if (rightHandAngle <= THERSHOLD_ANGLE) stepFlag = 2;
      }
      else if (stepFlag == 2)
      {
        leftHandAngle += moveSpeed;
        rightLegAngle -= moveSpeed;
        
        if (leftHandAngle >= 90) stepFlag = 3;
      }
      else if (stepFlag == 3)
      {
        rightHandAngle += moveSpeed;
        leftLegAngle -= moveSpeed;
        
        if (rightHandAngle >= (180 - THERSHOLD_ANGLE)) stepFlag = 4;
      }
      else if (stepFlag == 4)
      {
        leftHandAngle += moveSpeed;
        rightLegAngle -= moveSpeed;
        
        if (leftHandAngle >= (180 - THERSHOLD_ANGLE)) stepFlag = 5;
      }
      else if (stepFlag == 5)
      {
        rightHandAngle -= moveSpeed;
        leftLegAngle += moveSpeed;
        
        if (rightHandAngle <= 90) stepFlag = 0;
      }
      updateServo(leftHandAngle, rightHandAngle, leftLegAngle, rightLegAngle);  // 更新舵机角度
    }
    break;

    case ACTION_MODEL_LEFT: // 左转
    {
      if (stepFlag == 0)
      {
        rightHandAngle += moveSpeed;
        
        if (rightHandAngle >= (180 - THERSHOLD_ANGLE)) stepFlag = 1;
      }
      else if (stepFlag == 1)
      {
        rightLegAngle -= moveSpeed;
        
        if (rightLegAngle <= THERSHOLD_ANGLE) stepFlag = 2;
      }
      else if (stepFlag == 2)
      {
        rightHandAngle -= moveSpeed;
        
        if (rightHandAngle <= 90) stepFlag = 3;
      }
      else if (stepFlag == 3)
      {
        rightLegAngle += moveSpeed;
        
        if (rightLegAngle >= 90) stepFlag = 0;
      }
      updateServo(leftHandAngle, rightHandAngle, leftLegAngle, rightLegAngle);  // 更新舵机角度
    }
    break;

    case ACTION_MODEL_RIGHT: // 右转
    {
      if (stepFlag == 0)
      {
        leftHandAngle -= moveSpeed;
        
        if (leftHandAngle <= THERSHOLD_ANGLE) stepFlag = 1;
      }
      else if (stepFlag == 1)
      {
        leftLegAngle += moveSpeed;
        
        if (leftLegAngle >= (180 - THERSHOLD_ANGLE)) stepFlag = 2;
      }
      else if (stepFlag == 2)
      {
        leftHandAngle += moveSpeed;
        
        if (leftHandAngle >= 90) stepFlag = 3;
      }
      else if (stepFlag == 3)
      {
        leftLegAngle -= moveSpeed;
        
        if (leftLegAngle <= 90) stepFlag = 0;
      }
      updateServo(leftHandAngle, rightHandAngle, leftLegAngle, rightLegAngle);  // 更新舵机角度
    }
    break;

    case ACTION_MODEL_BACKWARD: // 后退
    {
      if (stepFlag == 0)
      {
        leftHandAngle += moveSpeed;
        rightLegAngle -= moveSpeed;
        
        if (leftHandAngle >= (180 - THERSHOLD_ANGLE)) stepFlag = 1;
      }
      else if (stepFlag == 1)
      {
        rightHandAngle += moveSpeed;
        leftLegAngle -= moveSpeed;
        
        if (rightHandAngle >= (180 - THERSHOLD_ANGLE)) stepFlag = 2;
      }
      else if (stepFlag == 2)
      {
        leftHandAngle -= moveSpeed;
        rightLegAngle += moveSpeed;
        
        if (leftHandAngle <= 90) stepFlag = 3;
      }
      else if (stepFlag == 3)
      {
        rightHandAngle -= moveSpeed;
        leftLegAngle += moveSpeed;
        
        if (rightHandAngle <= THERSHOLD_ANGLE) stepFlag = 4;
      }
      else if (stepFlag == 4)
      {
        leftHandAngle -= moveSpeed;
        rightLegAngle += moveSpeed;
        
        if (leftHandAngle <= THERSHOLD_ANGLE) stepFlag = 5;
      }
      else if (stepFlag == 5)
      {
        rightHandAngle += moveSpeed;
        leftLegAngle -= moveSpeed;
        
        if (rightHandAngle >= 90) stepFlag = 0;
      }
      updateServo(leftHandAngle, rightHandAngle, leftLegAngle, rightLegAngle);  // 更新舵机角度
    }
    break;

    case ACTION_MODEL_STANDUP: // 站立
    {
      leftLegAngle = 90;
      rightLegAngle = 90;
      updateServo(leftHandAngle, rightHandAngle, leftLegAngle, rightLegAngle);  // 更新舵机角度

      delay(200);

      leftHandAngle = 90;
      rightHandAngle = 90;
      updateServo(leftHandAngle, rightHandAngle, leftLegAngle, rightLegAngle);  // 更新舵机角度
    }
    break;

    case ACTION_MODEL_SITDOWN: // 趴下
    {
      leftLegAngle = 0;
      rightLegAngle = 180;
      updateServo(leftHandAngle, rightHandAngle, leftLegAngle, rightLegAngle);  // 更新舵机角度

      delay(200);
      
      leftHandAngle = 0;
      rightHandAngle = 180;
      updateServo(leftHandAngle, rightHandAngle, leftLegAngle, rightLegAngle);  // 更新舵机角度
    }
    break;

    case ACTION_MODEL_LEFTRIGHT: // 左右摇晃
    {
      if (stepFlag == 0)
      {
        leftHandAngle -= moveSpeed;
        rightHandAngle -= moveSpeed;
        leftLegAngle -= moveSpeed;
        rightLegAngle -= moveSpeed;
        
        if (leftHandAngle <= 10) stepFlag = 1;
      }
      else
      {
        leftHandAngle += moveSpeed;
        rightHandAngle += moveSpeed;
        leftLegAngle += moveSpeed;
        rightLegAngle += moveSpeed;
        
        if (leftHandAngle >= 90) stepFlag = 0;
      }
      updateServo(leftHandAngle, rightHandAngle, leftLegAngle, rightLegAngle);  // 更新舵机角度
    }
    break;

    case ACTION_MODEL_FRONTBACK: // 前后摇晃
    {
      if (stepFlag == 0)
      {
        leftHandAngle -= moveSpeed;
        rightHandAngle += moveSpeed;
        leftLegAngle -= moveSpeed;
        rightLegAngle += moveSpeed;
        
        if (leftHandAngle <= 10) stepFlag = 1;
      }
      else
      {
        leftHandAngle += moveSpeed;
        rightHandAngle -= moveSpeed;
        leftLegAngle += moveSpeed;
        rightLegAngle -= moveSpeed;
        
        if (leftHandAngle >= 90) stepFlag = 0;
      }
      updateServo(leftHandAngle, rightHandAngle, leftLegAngle, rightLegAngle);  // 更新舵机角度
    }
    break;
  
    default:
    stepFlag = 0;
    break;
  }

  // 定时器中断处理
  if (timerFlag) {
    timerFlag = false;
    
    // 如果 WeXCube 连接成功则执行心跳包
    if (wexConnectFlag) {
      wex_askHandshake();

      // 把当前舵机角度值发送给 WeXCube
      wex_setValue(SLIDER_LEFTHAND_ID, leftHandAngle);  // 左手舵机角度
      wex_setValue(SLIDER_RIGHTHAND_ID, rightHandAngle);  // 右手舵机角度
      wex_setValue(SLIDER_LEFTLEG_ID, leftLegAngle);  // 左脚舵机角度
      wex_setValue(SLIDER_RIGHTLEG_ID, rightLegAngle);  // 右脚舵机角度

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
      selectEyesColor(BUTTON_EYES_COLOR1_ID + lightColor); // 眼睛颜色
      wex_setValue(SLIDER_EYES_LEVEL_ID, map(lightLevel, 0, 255, 0, 100)); // 眼睛亮度值
      selectEyesModel(BUTTON_EYES_MODE1_ID + lightModel); // 眼睛模式

      wex_setValue(SLIDER_LEFTHAND_ID, leftHandAngle);  // 左手舵机角度
      wex_setValue(SLIDER_RIGHTHAND_ID, rightHandAngle);  // 右手舵机角度
      wex_setValue(SLIDER_LEFTLEG_ID, leftLegAngle);  // 左脚舵机角度
      wex_setValue(SLIDER_RIGHTLEG_ID, rightLegAngle);  // 右脚舵机角度

      wex_setValue(SLIDER_MOVE_SPEED_ID, moveSpeed); // 动作速度
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
        case BUTTON_EYES_COLOR1_ID: // 眼睛颜色1
        case BUTTON_EYES_COLOR2_ID: // 眼睛颜色2
        case BUTTON_EYES_COLOR3_ID: // 眼睛颜色3
        case BUTTON_EYES_COLOR4_ID: // 眼睛颜色4
        case BUTTON_EYES_COLOR5_ID: // 眼睛颜色5
        case BUTTON_EYES_COLOR6_ID: // 眼睛颜色6
        {
          selectEyesColor(psWexCmd->ucCtrlId); // 选择眼睛颜色
        }
        break;

        case SLIDER_EYES_LEVEL_ID: // 眼睛亮度调节
        {
          lightLevel = map(psWexCmd->ucValue, 0, 100, 0, 255); // 将滑动条值映射到 0-255 范围
          Serial.printf("Eyes light level: %d\n", lightLevel);
          updateLED(ledRGB[0], ledRGB[1], ledRGB[2], lightLevel);
        }
        break;

        case BUTTON_EYES_MODE1_ID: // 眼睛模式1
        case BUTTON_EYES_MODE2_ID: // 眼睛模式2
        case BUTTON_EYES_MODE3_ID: // 眼睛模式3
        {
          selectEyesModel(psWexCmd->ucCtrlId); // 选择眼睛模式
        }
        break;

        case SLIDER_LEFTHAND_ID:        // 左手舵机控制
        {
          leftHandAngle = psWexCmd->ucValue;
          Serial.printf("Left hand servo angle: %d\n", leftHandAngle);
          leftHandServo.write(leftHandAngle);  // 设置舵机角度
        }
        break;

        case SLIDER_RIGHTHAND_ID:      // 右手舵机控制
        {
          rightHandAngle = psWexCmd->ucValue;
          Serial.printf("Right hand servo angle: %d\n", rightHandAngle);
          rightHandServo.write(rightHandAngle);  // 设置舵机角度
        }
        break;

        case SLIDER_LEFTLEG_ID:       // 左脚舵机控制
        {
          leftLegAngle = psWexCmd->ucValue;
          Serial.printf("Left leg servo angle: %d\n", leftLegAngle);
          leftLegServo.write(leftLegAngle);  // 设置舵机角度
        }
        break;

        case SLIDER_RIGHTLEG_ID:      // 右脚舵机控制
        {
          rightLegAngle = psWexCmd->ucValue;
          Serial.printf("Right leg servo angle: %d\n", rightLegAngle);
          rightLegServo.write(rightLegAngle);  // 设置舵机角度
        }
        break;

        case BUTTON_FORWARD_ID:       // 前进
        {
          if (psWexCmd->ucValue)
          {
            actionModel = ACTION_MODEL_FORWARD;
            leftLegAngle = 90;
            rightLegAngle = 90;
            updateServo(leftHandAngle, rightHandAngle, leftLegAngle, rightLegAngle);  // 更新舵机角度
            delay(200);
            leftHandAngle = 90;
            rightHandAngle = 90;
            updateServo(leftHandAngle, rightHandAngle, leftLegAngle, rightLegAngle);
          }
          else actionModel = 0; // 停止
        }
        break;

        case BUTTON_LEFT_ID:          // 左转
        {
          if (psWexCmd->ucValue)
          {
            actionModel = ACTION_MODEL_LEFT;
            leftLegAngle = 90;
            rightLegAngle = 90;
            updateServo(leftHandAngle, rightHandAngle, leftLegAngle, rightLegAngle);  // 更新舵机角度
            delay(200);
            leftHandAngle = 90;
            rightHandAngle = 90;
            updateServo(leftHandAngle, rightHandAngle, leftLegAngle, rightLegAngle);
          }
          else actionModel = 0; // 停止
        }
        break;

        case BUTTON_RIGHT_ID:         // 右转
        {
          if (psWexCmd->ucValue)
          {
            actionModel = ACTION_MODEL_RIGHT;
            leftLegAngle = 90;
            rightLegAngle = 90;
            updateServo(leftHandAngle, rightHandAngle, leftLegAngle, rightLegAngle);  // 更新舵机角度
            delay(200);
            leftHandAngle = 90;
            rightHandAngle = 90;
            updateServo(leftHandAngle, rightHandAngle, leftLegAngle, rightLegAngle);
          }
          else actionModel = 0; // 停止
        }
        break;

        case BUTTON_BACKWARD_ID:      // 后退
        {
          if (psWexCmd->ucValue)
          {
            actionModel = ACTION_MODEL_BACKWARD;
            leftLegAngle = 90;
            rightLegAngle = 90;
            updateServo(leftHandAngle, rightHandAngle, leftLegAngle, rightLegAngle);  // 更新舵机角度
            delay(200);
            leftHandAngle = 90;
            rightHandAngle = 90;
            updateServo(leftHandAngle, rightHandAngle, leftLegAngle, rightLegAngle);
          }
          else actionModel = 0; // 停止
        }
        break;

        case BUTTON_STANDUP_ID:       // 站立
        {
          if (psWexCmd->ucValue) actionModel = ACTION_MODEL_STANDUP;
          else actionModel = 0; // 停止
        }
        break;

        case BUTTON_SITDOWN_ID:       // 趴下
        {
          if (psWexCmd->ucValue) actionModel = ACTION_MODEL_SITDOWN;
          else actionModel = 0; // 停止
        }
        break;

        case BUTTON_LEFTRIGHT_ID:     // 左右摇晃
        {
          if (psWexCmd->ucValue)
          {
            actionModel = ACTION_MODEL_LEFTRIGHT;
            leftLegAngle = 90;
            rightLegAngle = 170;
            updateServo(leftHandAngle, rightHandAngle, leftLegAngle, rightLegAngle);
            delay(200);
            leftHandAngle = 90;
            rightHandAngle = 170;
            updateServo(leftHandAngle, rightHandAngle, leftLegAngle, rightLegAngle);
          }
          else actionModel = 0; // 停止
        }
        break;

        case BUTTON_FRONTBACK_ID:        // 前后摇晃
        {
          if (psWexCmd->ucValue)
          {
            actionModel = ACTION_MODEL_FRONTBACK;
            leftLegAngle = 170;
            rightLegAngle = 10;
            updateServo(leftHandAngle, rightHandAngle, leftLegAngle, rightLegAngle);
            delay(200);
            leftHandAngle = 90;
            rightHandAngle = 90;
            updateServo(leftHandAngle, rightHandAngle, leftLegAngle, rightLegAngle); 
          }
          else actionModel = 0; // 停止
        } 
        break;

        case SLIDER_MOVE_SPEED_ID: // 动作速度调节
        {
          if (psWexCmd->ucValue == 0) 
          {
            moveSpeed = 1; // 最小速度
            wex_setValue(SLIDER_MOVE_SPEED_ID, 1); // 设置滑动条值为 1
          }
          else 
          {
            moveSpeed = psWexCmd->ucValue;
          }
          Serial.printf("Move speed: %d\n", moveSpeed);
        }
        break;

        default:
        break;
      }
    }
    break;

    case eWexCmd_BackRGB:
    {
      switch (psWexCmd->ucCtrlId)
      {
        case BUTTON_EYES_COLOR1_ID: // 眼睛颜色1
        case BUTTON_EYES_COLOR2_ID: // 眼睛颜色2
        case BUTTON_EYES_COLOR3_ID: // 眼睛颜色3
        case BUTTON_EYES_COLOR4_ID: // 眼睛颜色4
        case BUTTON_EYES_COLOR5_ID: // 眼睛颜色5
        case BUTTON_EYES_COLOR6_ID: // 眼睛颜色6
        {
          // WeXCube 颜色值返回
          ledRGB[0] = psWexCmd->sColor.ucR; // 红色分量
          ledRGB[1] = psWexCmd->sColor.ucG; // 绿色分量
          ledRGB[2] = psWexCmd->sColor.ucB; // 蓝色分量
          Serial.printf("Eyes light color: %d, %d, %d\n", ledRGB[0], ledRGB[1], ledRGB[2]);
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


/**
 * @brief LED 颜色、亮度更新
 * @param ucRed 红色分量
 * @param ucGreen 绿色分量
 * @param ucBlue 蓝色分量
 * @param ucLevel 亮度
 * @retval None
 */
void updateLED(wex_u8_t ucRed, wex_u8_t ucGreen, wex_u8_t ucBlue, wex_u8_t ucLevel)
{
  for (int i = 0; i < EYES_NUM; i++)
  {
    strip.setPixelColor(i, strip.Color(ucRed, ucGreen, ucBlue));
    strip.setBrightness(ucLevel);
    strip.show();
  }
}


/**
 * @brief 舵机更新
 * @param ucLeftHand 左手舵机角度
 * @param ucRightHand 右手舵机角度
 * @param ucLeftLeg 左脚舵机角度
 * @param ucRightLeg 右脚舵机角度
 * @retval None
 */
void updateServo(wex_u8_t ucLeftHand, wex_u8_t ucRightHand, wex_u8_t ucLeftLeg, wex_u8_t ucRightLeg)
{
  leftHandServo.write(ucLeftHand);  // 设置左手舵机角度
  rightHandServo.write(ucRightHand);  // 设置右手舵机角度
  leftLegServo.write(ucLeftLeg);  // 设置左脚舵机角度
  rightLegServo.write(ucRightLeg);  // 设置右脚舵机角度
}


// 选择眼睛颜色
void selectEyesColor(wex_u8_t id)
{
  wex_askBackRGB(id);
  lightColor = id - BUTTON_EYES_COLOR1_ID;
  wex_setText(BUTTON_EYES_COLOR1_ID, ""); // 清空按钮文本
  wex_setText(BUTTON_EYES_COLOR2_ID, "");
  wex_setText(BUTTON_EYES_COLOR3_ID, "");
  wex_setText(BUTTON_EYES_COLOR4_ID, "");
  wex_setText(BUTTON_EYES_COLOR5_ID, "");
  wex_setText(BUTTON_EYES_COLOR6_ID, "");
  wex_setText(id, "✓"); // 设置选中颜色的按钮文本为 "✓"
}


// 选择眼睛模式
void selectEyesModel(wex_u8_t id)
{
  lightModel = id - BUTTON_EYES_MODE1_ID; // 0 - 正常，1 - 呼吸，2 - 彩灯
  wex_setText(BUTTON_EYES_MODE1_ID, ""); // 清空按钮文本
  wex_setText(BUTTON_EYES_MODE2_ID, "");
  wex_setText(BUTTON_EYES_MODE3_ID, "");
  wex_setText(id, "✓"); // 设置选中模式的按钮文本为 "✓"
}