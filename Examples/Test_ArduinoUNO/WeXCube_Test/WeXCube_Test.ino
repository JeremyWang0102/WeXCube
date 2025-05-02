#include "src/wexcube_sdk/wexcube.h"

// 小程序设备控制页面控件ID
#define TEXT1_ID    1         // 文本控件1 用于测试显示

wex_u32_t baudrate = 9600;    // BLE 模块串口波特率，请根据模块串口值填写，常见值为 9600、115200

wex_u32_t count = 0;
wex_u32_t seconds = 0;

/***
 * 注意：串口已经被 WeXCube 占用，不能在其他函数中再使用 Serial.begin 等串口函数了。
***/
void setup() {
  /*** 用户代码 BEGIN ***



  *** 用户代码 END ***/

  wex_init();
  wex_start();

  sei();  // 开启全局中断
}

void loop() {
  const t_sWexCmd *psWexCmd = wex_process();
  switch (psWexCmd->eCmdType)
  {
    case eWexCmd_Connect:       		// WeXCube 连接指令
    {
      wex_setText(TEXT1_ID, wex_uintToStr(seconds));
    }
    break;

    case eWexCmd_Disconnect:        // WeXCube 断开连接指令
    {
    }
    break;

    case eWexCmd_Event:             // 控件事件触发
    {
    }
    break;

    case eWexCmd_Value:             // 控件值
    {
    }
    break;

    case eWexCmd_Text:              // 控件文本
    {
    }
    break;

    default:
    break;
  }

  delay(10);
  count++;
  if (count >= 100)
  {
    count = 0;
    seconds++;
    wex_setText(TEXT1_ID, wex_uintToStr(seconds));
  }
}
