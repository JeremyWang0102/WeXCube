/**
 * @file wexcube_port.c
 * @author JeremyWang (jeremywang0102@gmail.com / gin0101@126.com)
 * @brief WeXCube 接口文件
 * @version 
 * @date 2025-01-19
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#include "wexcube_port.h"
#include "wexcube.h"

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLE2902.h>


// BLE 名称 （默认值）
#define BLE_NAME_DEFAULT            "ESP32_Arduino_WEX"

// 服务 UUID （默认值）
#define SERVICE_UUID_DEFAULT        "0000FFE0-0000-1000-8000-00805F9B34FB"

// 通知 UUID （默认值）
#define NOTIFY_CHARA_UUID_DEFAULT   "0000FFE1-0000-1000-8000-00805F9B34FB"

// 写入 UUID （默认值）
#define WRITE_CHARA_UUID_DEFAULT    "0000FFE2-0000-1000-8000-00805F9B34FB"

String _bleName = "";
String _serviceUUID = "";
String _notifyUUID = "";
String _writeUUID = "";

BLEServer *_pServer;
BLEService *_pService;
BLECharacteristic *_pNotifyChara;
BLECharacteristic *_pWriteChara;
BLEAdvertising *_pAdvertising;


// 继承 BLEServerCallbacks 类来监听连接和断开事件
class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      // 连接时停止广播
      _pAdvertising->stop();
    }

    void onDisconnect(BLEServer* pServer) {
      // 断开时开始广播
      _pAdvertising->start();

      wex_reset();
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


/**
 * @brief BLE 参数配置
 * 
 * @param bleName 蓝牙名称
 * @param serviceUUID 服务 UUID
 * @param notifyUUID 通知特征值
 * @param writeUUID 写入特征值
 */
void wex_ble_config(String bleName, String serviceUUID, String notifyUUID, String writeUUID)
{
  _bleName      = bleName;
  _serviceUUID  = serviceUUID;
  _notifyUUID   = notifyUUID;
  _writeUUID    = writeUUID;
}


/**
 * @brief 端口初始化函数
 * 
 */
void wex_port_init(void)
{
  // 如果 BLE 配置为空，则使用默认值
  if (_bleName.isEmpty()) _bleName = BLE_NAME_DEFAULT;
  if (_serviceUUID.isEmpty()) _serviceUUID = SERVICE_UUID_DEFAULT;
  if (_notifyUUID.isEmpty()) _notifyUUID = NOTIFY_CHARA_UUID_DEFAULT;
  if (_writeUUID.isEmpty()) _writeUUID = WRITE_CHARA_UUID_DEFAULT;

  // 设置 BLE 设备名称
  BLEDevice::init(_bleName.c_str());

  // 创建 BLE 服务器
  _pServer = BLEDevice::createServer();
  _pServer->setCallbacks(new MyServerCallbacks());

  // 创建自定义服务
  _pService = _pServer->createService(_serviceUUID.c_str());

  // 创建通知特征
  _pNotifyChara = _pService->createCharacteristic(
                       _notifyUUID.c_str(),
                       BLECharacteristic::PROPERTY_NOTIFY
                     );
  // 为通知特征添加描述符
  _pNotifyChara->addDescriptor(new BLE2902());

  // 创建写入特征
  _pWriteChara = _pService->createCharacteristic(
                       _writeUUID.c_str(),
                       BLECharacteristic::PROPERTY_WRITE
                     );
  // 设置回调来监听数据
  _pWriteChara->setCallbacks(new MyCharacteristicCallbacks());

  // 创建广播
  _pAdvertising = _pServer->getAdvertising();
  _pAdvertising->setAdvertisementType(ADV_TYPE_IND);
  _pAdvertising->setScanResponse(false);
  _pAdvertising->setMinPreferred(0x190);    // 最小间隔 (单位 0.625ms)
  _pAdvertising->setMaxPreferred(0x640);    // 最大间隔 (单位 0.625ms)

  // 启动服务
  _pService->start();
  // 开始广播
  _pAdvertising->start();
}


/**
 * @brief 发送数据
 * 
 * @param pucData 数据指针
 * @param ucLen 数据长度
 */
void wex_port_send(wex_u8_t *pucData, wex_u8_t ucLen)
{
  // 设置通知特征的值
  _pNotifyChara->setValue(pucData, ucLen);
  // 发送通知
  _pNotifyChara->notify();
}


/**
  * @brief  This function handles UART interrupt request.
  *
  */
void UART_IRQHandler(void)
{
  // 在 class MyCharacteristicCallbacks 中实现
}
