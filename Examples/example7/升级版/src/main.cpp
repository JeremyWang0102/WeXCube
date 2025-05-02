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
#define JOYSTICK_X_PIN          2   // 摇杆X轴引脚
#define JOYSTICK_Y_PIN          3   // 摇杆Y轴引脚
#define JOYSTICK_SW_PIN         4   // 摇杆按键引脚

#define EYES_NUM                2   // 眼睛灯带数量

#define THERSHOLD_ANGLE         60  // 舵机角度变化阈值

#define THERSHOLD_JOYSTICK_FORNT 120// 摇杆前进阈值
#define THERSHOLD_JOYSTICK_BACK  250// 摇杆后退阈值
#define THERSHOLD_JOYSTICK_LEFT  160// 摇杆左转阈值
#define THERSHOLD_JOYSTICK_RIGHT 220// 摇杆右转阈值


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
  ACTION_MODEL_NONE = 0,      // 无动作
  ACTION_MODEL_FORWARD,       // 向前
  ACTION_MODEL_LEFT,          // 向左
  ACTION_MODEL_RIGHT,         // 向右
  ACTION_MODEL_BACKWARD,      // 后退
  ACTION_MODEL_STANDUP,       // 站立
  ACTION_MODEL_SITDOWN,       // 趴下
  ACTION_MODEL_CROUCH,        // 蹲下
  ACTION_MODEL_LEFTRIGHT,     // 左右摇晃
  ACTION_MODEL_FRONTBACK      // 前后摇晃
};

// 状态枚举
enum stateModel_t
{
  STATE_MODEL_IDLE = 0,       // 空闲状态
  STATE_MODEL_MOVING,         // 移动状态
  STATE_MODEL_STANDUP,        // 站立状态
  STATE_MODEL_SITDOWN,        // 趴下状态
  STATE_MODEL_CROUCH,         // 蹲下状态
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
wex_u8_t stateModel = 0;            // 状态模式（enum stateModel_t）
wex_u8_t moveSpeed = 1;             // 动作速度（1~5）
wex_u8_t stepFlag = 0;

Adafruit_NeoPixel strip(EYES_NUM, EYES_CTRL_PIN, NEO_GRB + NEO_KHZ400);

wex_u8_t lightLevel = 0;
wex_u8_t lightColor = 0;
wex_u8_t lightModel = 0; // 0 - 正常，1 - 呼吸，2 - 彩灯
wex_u8_t ledRGB[3] = {255, 255, 255}; // 眼睛灯光颜色（RGB）

wex_u8_t askHandshakeCnt = 0;       // 心跳包计数
bool wexConnectFlag = false;        // WeXCube 连接标志
bool wexcubeCtrlFlag = false;       // WeXCube 控制标志

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
void standUp(); // 站立
void sitDown(); // 趴下
void crouch(); // 蹲下


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
  actionModel = ACTION_MODEL_SITDOWN; // 初始动作模式为趴下

  strip.begin();
  strip.setBrightness(lightLevel); // 亮度初始值
  strip.show();           // 清空灯带

  // 摇杆控制引脚初始化
  analogReadResolution(8); // 设置ADC分辨率为8位（0-255）
  analogSetAttenuation(ADC_11db); // 设置衰减为11dB，量程约0-3.1V
  pinMode(JOYSTICK_X_PIN, ANALOG);
  pinMode(JOYSTICK_Y_PIN, ANALOG);
  pinMode(JOYSTICK_SW_PIN, INPUT_PULLUP); // 摇杆按键引脚设置为上拉输入
}

void loop() {
  wex_ble_loop();

  // 延时 10ms，避免蓝牙通信阻塞
  delay(10);

  if (wexcubeCtrlFlag == false)
  {
    // 摇杆事件处理：
    // 1. 单击，眼睛灯光开关
    // 2. 双击，站立/趴下
    // 3. 向前拉，前进
    // 4. 向左拉，左转
    // 5. 向右拉，右转
    // 6. 向后拉，蹲下
    int joystickX = analogRead(JOYSTICK_X_PIN); // 读取摇杆X轴值
    int joystickY = analogRead(JOYSTICK_Y_PIN); // 读取摇杆Y轴值
    int joystickSW = digitalRead(JOYSTICK_SW_PIN); // 读取摇杆按键值

    if (joystickSW == LOW) // 摇杆按键按下
    {
      wex_u32_t i = 0;
      delay(30); // 防抖动延时

      // 等待摇杆按键松开
      while (digitalRead(JOYSTICK_SW_PIN) == LOW)
      {
        i++;
        delay(10); 
      }
      if (i < 60)
      {
        while (i < 60)
        {
          delay(10);
          if (digitalRead(JOYSTICK_SW_PIN) == LOW) // 摇杆按键再次按下
            break;
          i++;
        }
      }
      if (i < 60)
      {
        if (stateModel == STATE_MODEL_STANDUP)
          actionModel = ACTION_MODEL_SITDOWN;
        else
          actionModel = ACTION_MODEL_STANDUP;
        delay(500); // 等待手拿开
      }
      else
      {
        lightLevel = lightLevel == 0 ? 128 : 0; // 切换眼睛灯光开关
        if (lightLevel == 0)
        {
          updateLED(0, 0, 0, 0); // 关闭灯光
          wex_setValue(SLIDER_EYES_LEVEL_ID, 0); // 更新小程序滑块值
        }
        else
        {
          updateLED(ledRGB[0], ledRGB[1], ledRGB[2], lightLevel); // 打开灯光
          wex_setValue(SLIDER_EYES_LEVEL_ID, lightLevel); // 更新小程序滑块值
        }
      }
    }
    else // 摇杆方向处理
    {
      static wex_u8_t index = 0;
      static wex_u8_t joystickXArr[5] = {0}; // 摇杆X轴值数组
      static wex_u8_t joystickYArr[5] = {0}; // 摇杆Y轴值数组

      if (index < 5)
      {
        joystickXArr[index] = joystickX; // 存储摇杆X轴值
        joystickYArr[index] = joystickY; // 存储摇杆Y轴值
        index++;
      }
      else
      {
        static wex_u8_t lastActionModel = ACTION_MODEL_NONE; // 上一个动作模式
        static wex_u32_t lastTime = 0; // 上次动作时间

        index = 0;
        joystickX = (joystickXArr[0] + joystickXArr[1] + joystickXArr[2] + joystickXArr[3] + joystickXArr[4]) / 5; // 平均值
        joystickY = (joystickYArr[0] + joystickYArr[1] + joystickYArr[2] + joystickYArr[3] + joystickYArr[4]) / 5; // 平均值

        if (joystickY <= THERSHOLD_JOYSTICK_FORNT)
        {
          actionModel = ACTION_MODEL_FORWARD; // 前进
          lastTime = millis(); // 更新动作时间
        }
        else if (joystickY >= THERSHOLD_JOYSTICK_BACK)
        {
          actionModel = ACTION_MODEL_CROUCH; // 后退
          lastTime = millis(); // 更新动作时间
        }
        else if (joystickX <= THERSHOLD_JOYSTICK_LEFT)
        {
          actionModel = ACTION_MODEL_LEFT; // 左转
          lastTime = millis(); // 更新动作时间
        }
        else if (joystickX >= THERSHOLD_JOYSTICK_RIGHT)
        {
          actionModel = ACTION_MODEL_RIGHT; // 右转
          lastTime = millis(); // 更新动作时间
        }
        else
        {
          if ((stepFlag == 0) && (millis() - lastTime > 1000)) // 停止状态，且超过1秒未操作
            actionModel = ACTION_MODEL_NONE; // 无动作
        }

        if (actionModel != lastActionModel) // 动作模式发生变化
        {
          if ((actionModel != ACTION_MODEL_NONE) && (actionModel != ACTION_MODEL_CROUCH))
          {
            standUp(); // 站立
          }
          lastActionModel = actionModel; // 更新上一个动作模式
        }
      }
    }
  }

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
        
        if (rightHandAngle <= 90) stepFlag = 6;
      }
      else if (stepFlag == 6)
      {
        leftHandAngle -= moveSpeed;
        rightLegAngle += moveSpeed;
        
        if (leftHandAngle <= 90) stepFlag = 0;
      }
      updateServo(leftHandAngle, rightHandAngle, leftLegAngle, rightLegAngle);  // 更新舵机角度

      stateModel = STATE_MODEL_MOVING; // 设置状态为移动
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

      stateModel = STATE_MODEL_MOVING; // 设置状态为移动
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

      stateModel = STATE_MODEL_MOVING; // 设置状态为移动
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
        
        if (rightHandAngle >= 90) stepFlag = 6;
      }
      else if (stepFlag == 6)
      {
        leftHandAngle += moveSpeed;
        rightLegAngle -= moveSpeed;
        
        if (leftHandAngle >= 90) stepFlag = 0;
      }
      updateServo(leftHandAngle, rightHandAngle, leftLegAngle, rightLegAngle);  // 更新舵机角度

      stateModel = STATE_MODEL_MOVING; // 设置状态为移动
    }
    break;

    case ACTION_MODEL_STANDUP: // 站立
    {
      standUp(); // 调用站立函数

      actionModel = ACTION_MODEL_NONE;
      stateModel = STATE_MODEL_STANDUP; // 设置状态为站立
    }
    break;

    case ACTION_MODEL_SITDOWN: // 趴下
    {
      sitDown(); // 调用趴下函数

      actionModel = ACTION_MODEL_NONE;
      stateModel = STATE_MODEL_SITDOWN; // 设置状态为趴下
    }
    break;

    case ACTION_MODEL_CROUCH: // 蹲下
    {
      crouch(); // 调用蹲下函数

      actionModel = ACTION_MODEL_NONE;
      stateModel = STATE_MODEL_CROUCH; // 设置状态为趴下
    }
    break;

    case ACTION_MODEL_LEFTRIGHT: // 左右摇晃
    {
      if (stepFlag == 0)
      {
        while ((abs(rightHandAngle - 160) > moveSpeed) || (abs(rightLegAngle - 160) > moveSpeed))
        {
          if (rightHandAngle > 160) rightHandAngle -= moveSpeed;
          else if (rightHandAngle < 160) rightHandAngle += moveSpeed;

          if (rightLegAngle > 160) rightLegAngle -= moveSpeed;
          else if (rightLegAngle < 160) rightLegAngle += moveSpeed;

          delay(10); // 延时，避免过快

          updateServo(leftHandAngle, rightHandAngle, leftLegAngle, rightLegAngle);  // 更新舵机角度
        }
  
        stepFlag = 1;
      }
      else if (stepFlag == 1)
      {
        leftHandAngle -= moveSpeed;
        rightHandAngle -= moveSpeed;
        leftLegAngle -= moveSpeed;
        rightLegAngle -= moveSpeed;
        
        if (leftHandAngle <= 20) stepFlag = 2;
      }
      else if (stepFlag == 2)
      {
        leftHandAngle += moveSpeed;
        rightHandAngle += moveSpeed;
        leftLegAngle += moveSpeed;
        rightLegAngle += moveSpeed;
        
        if (leftHandAngle >= 90) stepFlag = 0;
      }
      updateServo(leftHandAngle, rightHandAngle, leftLegAngle, rightLegAngle);  // 更新舵机角度

      stateModel = STATE_MODEL_MOVING; // 设置状态为移动
    }
    break;

    case ACTION_MODEL_FRONTBACK: // 前后摇晃
    {
      if (stepFlag == 0)
      {
        while ((abs(leftLegAngle - 160) > moveSpeed) || (abs(rightLegAngle - 20) > moveSpeed))
        {
          if (leftLegAngle > 160) leftLegAngle -= moveSpeed;
          else if (leftLegAngle < 160) leftLegAngle += moveSpeed;

          if (rightLegAngle > 20) rightLegAngle -= moveSpeed;
          else if (rightLegAngle < 20) rightLegAngle += moveSpeed;

          delay(10); // 延时，避免过快

          updateServo(leftHandAngle, rightHandAngle, leftLegAngle, rightLegAngle);  // 更新舵机角度
        }
  
        stepFlag = 1;
      }
      else if (stepFlag == 1)
      {
        leftHandAngle -= moveSpeed;
        rightHandAngle += moveSpeed;
        leftLegAngle -= moveSpeed;
        rightLegAngle += moveSpeed;
        
        if (leftHandAngle <= 20) stepFlag = 2;
      }
      else if (stepFlag == 2)
      {
        leftHandAngle += moveSpeed;
        rightHandAngle -= moveSpeed;
        leftLegAngle += moveSpeed;
        rightLegAngle -= moveSpeed;
        
        if (leftHandAngle >= 90) stepFlag = 0;
      }
      updateServo(leftHandAngle, rightHandAngle, leftLegAngle, rightLegAngle);  // 更新舵机角度

      stateModel = STATE_MODEL_MOVING; // 设置状态为移动
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
            wexcubeCtrlFlag = true; // WeXCube 控制标志
            actionModel = ACTION_MODEL_FORWARD;
            standUp(); // 站立
          }
          else
          {
            wexcubeCtrlFlag = false;
            actionModel = ACTION_MODEL_NONE; // 停止
          }
        }
        break;

        case BUTTON_LEFT_ID:          // 左转
        {
          if (psWexCmd->ucValue)
          {
            wexcubeCtrlFlag = true; // WeXCube 控制标志
            actionModel = ACTION_MODEL_LEFT;
            standUp(); // 站立
          }
          else
          {
            wexcubeCtrlFlag = false;
            actionModel = ACTION_MODEL_NONE; // 停止
          }
        }
        break;

        case BUTTON_RIGHT_ID:         // 右转
        {
          if (psWexCmd->ucValue)
          {
            wexcubeCtrlFlag = true; // WeXCube 控制标志
            actionModel = ACTION_MODEL_RIGHT;
            standUp(); // 站立
          }
          else
          {
            wexcubeCtrlFlag = false;
            actionModel = ACTION_MODEL_NONE; // 停止
          }
        }
        break;

        case BUTTON_BACKWARD_ID:      // 后退
        {
          if (psWexCmd->ucValue)
          {
            wexcubeCtrlFlag = true; // WeXCube 控制标志
            actionModel = ACTION_MODEL_BACKWARD;
            standUp(); // 站立
          }
          else
          {
            wexcubeCtrlFlag = false;
            actionModel = ACTION_MODEL_NONE; // 停止
          }
        }
        break;

        case BUTTON_STANDUP_ID:       // 站立
        {
          if (psWexCmd->ucValue) actionModel = ACTION_MODEL_STANDUP;
        }
        break;

        case BUTTON_SITDOWN_ID:       // 趴下
        {
          if (psWexCmd->ucValue) actionModel = ACTION_MODEL_SITDOWN;
        }
        break;

        case BUTTON_LEFTRIGHT_ID:     // 左右摇晃
        {
          if (psWexCmd->ucValue)
          {
            wexcubeCtrlFlag = true; // WeXCube 控制标志
            actionModel = ACTION_MODEL_LEFTRIGHT;
            standUp(); // 站立
          }
          else
          {
            wexcubeCtrlFlag = false;
            actionModel = ACTION_MODEL_NONE; // 停止
          }
        }
        break;

        case BUTTON_FRONTBACK_ID:        // 前后摇晃
        {
          if (psWexCmd->ucValue)
          {
            wexcubeCtrlFlag = true; // WeXCube 控制标志
            actionModel = ACTION_MODEL_FRONTBACK;
            standUp(); // 站立
          }
          else
          {
            wexcubeCtrlFlag = false;
            actionModel = ACTION_MODEL_NONE; // 停止
          }
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
  }
  strip.setBrightness(ucLevel);
  strip.show();
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


// 站立
void standUp()
{
  while ((abs(leftLegAngle - 90) > 5) || (abs(rightLegAngle - 90) > 5))
  {
    if (leftLegAngle > 90) leftLegAngle -= 5;
    else if (leftLegAngle < 90) leftLegAngle += 5;

    if (rightLegAngle > 90) rightLegAngle -= 5;
    else if (rightLegAngle < 90) rightLegAngle += 5;

    delay(25); // 延时，避免过快

    updateServo(leftHandAngle, rightHandAngle, leftLegAngle, rightLegAngle);  // 更新舵机角度
  }
  
  while ((abs(leftHandAngle - 90) > 5) || (abs(rightHandAngle - 90) > 5))
  {
    if (leftHandAngle > 90) leftHandAngle -= 5;
    else if (leftHandAngle < 90) leftHandAngle += 5;

    if (rightHandAngle > 90) rightHandAngle -= 5;
    else if (rightHandAngle < 90) rightHandAngle += 5;

    delay(20); // 延时，避免过快
    
    updateServo(leftHandAngle, rightHandAngle, leftLegAngle, rightLegAngle);  // 更新舵机角度
  }
}

// 趴下
void sitDown()
{
  while ((abs(leftLegAngle - 10) > 5) || (abs(rightLegAngle - 170) > 5))
  {
    if (leftLegAngle > 10) leftLegAngle -= 5;
    else if (leftLegAngle < 10) leftLegAngle += 5;

    if (rightLegAngle > 170) rightLegAngle -= 5;
    else if (rightLegAngle < 170) rightLegAngle += 5;

    delay(20); // 延时，避免过快

    updateServo(leftHandAngle, rightHandAngle, leftLegAngle, rightLegAngle);  // 更新舵机角度
  }
  
  while ((abs(leftHandAngle - 10) > 5) || (abs(rightHandAngle - 170) > 5))
  {
    if (leftHandAngle > 10) leftHandAngle -= 5;
    else if (leftHandAngle < 10) leftHandAngle += 5;

    if (rightHandAngle > 170) rightHandAngle -= 5;
    else if (rightHandAngle < 170) rightHandAngle += 5;

    delay(20); // 延时，避免过快
    
    updateServo(leftHandAngle, rightHandAngle, leftLegAngle, rightLegAngle);  // 更新舵机角度
  }
}


// 蹲下
void crouch()
{
  while ((abs(leftLegAngle - 30) > 5) || (abs(rightLegAngle - 150) > 5))
  {
    if (leftLegAngle > 30) leftLegAngle -= 5;
    else if (leftLegAngle < 30) leftLegAngle += 5;

    if (rightLegAngle > 150) rightLegAngle -= 5;
    else if (rightLegAngle < 150) rightLegAngle += 5;

    delay(20); // 延时，避免过快

    updateServo(leftHandAngle, rightHandAngle, leftLegAngle, rightLegAngle);  // 更新舵机角度
  }
  
  while ((abs(leftHandAngle - 90) > 5) || (abs(rightHandAngle - 90) > 5))
  {
    if (leftHandAngle > 90) leftHandAngle -= 5;
    else if (leftHandAngle < 90) leftHandAngle += 5;

    if (rightHandAngle > 90) rightHandAngle -= 5;
    else if (rightHandAngle < 90) rightHandAngle += 5;

    delay(20); // 延时，避免过快
    
    updateServo(leftHandAngle, rightHandAngle, leftLegAngle, rightLegAngle);  // 更新舵机角度
  }
}