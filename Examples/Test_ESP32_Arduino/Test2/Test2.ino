#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLE2902.h>

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

BLEServer *pServer;
BLEService *pService;
BLECharacteristic *pNotifyChara;
BLECharacteristic *pWriteChara;
BLEAdvertising *pAdvertising;

int cnt = 0;
int second = 0;


// 继承 BLEServerCallbacks 类来监听连接和断开事件
class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      // 连接时停止广播
      pAdvertising->stop();
    }

    void onDisconnect(BLEServer* pServer) {
      // 断开时开始广播
      pAdvertising->start();
    }
};


// 监听写入数据的回调类
class MyCharacteristicCallbacks : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pCharacteristic) {
    // 获取接收到的数据
    std::string receivedData = pCharacteristic->getValue();
    wex_push((wex_u8_t*)receivedData.c_str(), receivedData.length());
  }
};

// BLE 初始化函数
void ble_init(void)
{
  // 设置 BLE 设备名称
  BLEDevice::init(BLE_NAME);

  // 创建 BLE 服务器
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // 创建自定义服务
  pService = pServer->createService(SERVICE_UUID);

  // 创建通知特征
  pNotifyChara = pService->createCharacteristic(
                       NOTIFY_CHARA_UUID,
                       BLECharacteristic::PROPERTY_NOTIFY
                     );
  // 为通知特征添加描述符
  pNotifyChara->addDescriptor(new BLE2902());

  // 创建写入特征
  pWriteChara = pService->createCharacteristic(
                       WRITE_CHARA_UUID,
                       BLECharacteristic::PROPERTY_WRITE
                     );
  // 设置回调来监听数据
  pWriteChara->setCallbacks(new MyCharacteristicCallbacks());

  // 创建广播
  pAdvertising = pServer->getAdvertising();
  pAdvertising->setAdvertisementType(ADV_TYPE_IND);
  pAdvertising->setScanResponse(false);
  pAdvertising->setMinPreferred(0x10);   // 16 * 0.625 = 10ms
  pAdvertising->setMaxPreferred(0x80);   // 128 * 0.625 = 80ms

  // 启动服务
  pService->start();
  // 开始广播
  pAdvertising->start();
}

void setup() {
  // 启动 BLE 设备
  Serial.begin(115200);
 
  ble_init();
  wex_ble_config(pNotifyChara);
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
