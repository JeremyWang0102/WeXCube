#include "wexcube.h" // 引入 WeXCube SDK 头文件

// 以下控件 ID 为示例，实际使用时请根据小程序端设备控制页面控件 ID 进行修改
#define BUTTON_ID        1  // 设备控制页面按钮的 ID
#define SWITCH_ID        2  // 设备控制页面开关的 ID
#define TEXT_ID          3  // 设备控制页面文本框的 ID
#define SLIDER_ID        4  // 设备控制页面滑动条 ID
#define INPUT_ID         5  // 设备控制页面输入框 ID

void main (void)
{
    wex_init();     // 初始化 WeXCube
    wex_start();    // 启动 WeXCube

    while (1)
    {
        const t_sWexCmd *pWexCmd = wex_process();   // 处理 WeXCube 收发数据，返回小程序端发来的指令
        switch (pWexCmd->eCmdType)  // 根据指令类型执行不同的操作
        {
        case eWexCmd_Connect:       // WeXCube 连接指令
            // 连接成功后可以发送数据给小程序端
            // 可以初始化小程序设备控制页面控件内容
            break;

        case eWexCmd_Disconnect:    // WeXCube 断开连接指令，当小程序端退出设备控制页面时发送
            // 断开连接后小程序端将不处理后续指令
            break;

        case eWexCmd_Handshake:     // WeXCube 握手指令，wex_askHandshake() 的回复
            // 如果需要判断小程序是否异常退出或者蓝牙异常断开，可以持续发送 wex_askHandshake()，然后检查是否有回复
            // 超过一定数量未收到回复则可以认为小程序端异常退出或者蓝牙异常断开
            break;

        case eWexCmd_Err:           // 错误码指令，wex_askErr() 的回复，当小程序端执行指令错误时也会发送
            pWexCmd->ucCtrlId;      // 发生错误的控件 ID，为 0 时表示 WeXCube 系统错误
            pWexCmd->ucErrCode;     // 错误码，具体错误码解释请参考 t_eWexErrCode
            break;

        case eWexCmd_Date:          // 日期指令，wex_askDate() 的回复
            pWexCmd->sDate.ucYear;  // 年
            pWexCmd->sDate.ucMonth; // 月
            pWexCmd->sDate.ucDay;   // 日
            break;
        
        case eWexCmd_Time:          // 时间指令，wex_askTime() 的回复
            pWexCmd->sTime.ucHour;  // 时
            pWexCmd->sTime.ucMinute;// 分
            pWexCmd->sTime.ucSecond;// 秒
            break;

        case eWexCmd_Event:         // 控件事件触发，小程序端按键、开关、滑动条的值改变时触发，小程序端主动发送无需请求
            pWexCmd->ucCtrlId;      // 触发事件的控件 ID
            pWexCmd->ucValue;       // 控件值，按键：0-释放、1-按下、2-长按（需开启），开关：0-关、1-开，滑动条：0~100
            break;
        
        case eWexCmd_Value:         // 控件值，wex_askValue() 的回复
            pWexCmd->ucCtrlId;      // 控件 ID
            pWexCmd->ucValue;       // 控件值，按键：0-释放、1-按下、2-长按（需开启），开关：0-关、1-开，滑动条：0~100
            break;

        case eWexCmd_Text:          // 控件文本内容，wex_askText() 的回复或小程序端输入框控件输入内容发生变化时发送
            pWexCmd->ucCtrlId;      // 控件 ID
            pWexCmd->pcText;        // 控件文本内容，字符串，以 '\0' 结尾，中文编码格式为 UTF-8
            break;

        case eWexCmd_BackRGB:       // 控件背景颜色，wex_askBackRGB() 的回复
        case eWexCmd_TextRGB:       // 控件文本颜色，wex_askTextRGB() 的回复
            pWexCmd->ucCtrlId;      // 控件 ID
            pWexCmd->sColor.ucR;    // 红色
            pWexCmd->sColor.ucG;    // 绿色
            pWexCmd->sColor.ucB;    // 蓝色
            break;
        
        case eWexCmd_FontSize:      // 控件文本字体大小，wex_askFontSize() 的回复
            pWexCmd->ucCtrlId;      // 控件 ID
            pWexCmd->ucFontSize;    // 字体大小
            break;
        
        default:
            break;
        }

        if (GPIO_Read(GPIOA, GPIO_Pin_0) == 0)
            wex_askHandshake();         // 请求握手，该指令可以用于判断小程序是否异常退出或蓝牙异常断开

        if (GPIO_Read(GPIOC, GPIO_Pin_1) == 0)
            wex_sendDisconnect();       // 断开 WeXCube 连接（并不会断开蓝牙连接），断开后将不能接收到小程序发来数据且不能申请重新连接，需要重新请在小程序端操作

        if (GPIO_Read(GPIOA, GPIO_Pin_1) == 0)
            wex_askErr();               // 请求错误码，返回的结果为上一次操作的执行结果，请求后小程序端会清空当前错误码

        if (GPIO_Read(GPIOA, GPIO_Pin_2) == 0)
            wex_askDate();              // 请求手机当前日期

        if (GPIO_Read(GPIOA, GPIO_Pin_3) == 0)
            wex_askTime();              // 请求手机当前时间

        if (GPIO_Read(GPIOA, GPIO_Pin_4) == 0)
            wex_askValue(SLIDER_ID);    // 请求控件值，可以被请求的控件类型：按键、开关、滑动条

        if (GPIO_Read(GPIOA, GPIO_Pin_5) == 0)
            wex_askText(INPUT_ID);      // 请求控件文本内容，可以被请求的控件类型：按键、文本、输入框

        if (GPIO_Read(GPIOA, GPIO_Pin_6) == 0)
            wex_askBackRGB(BUTTON_ID);  // 请求控件背景颜色，可以被请求的控件类型：按键、文本、输入框

        if (GPIO_Read(GPIOA, GPIO_Pin_7) == 0)
            wex_askTextRGB(TEXT_ID);    // 请求控件文本字体颜色，可以被请求的控件类型：按键、文本、输入框

        if (GPIO_Read(GPIOB, GPIO_Pin_0) == 0)
            wex_askFontSize(TEXT_ID);   // 请求控件文本字体大小，可以被请求的控件类型：按键、文本、输入框

        if (GPIO_Read(GPIOB, GPIO_Pin_1) == 0)
            wex_setValue(SWITCH_ID, 1); // 设置控件值，可以被设置的控件类型：开关、滑动条

        if (GPIO_Read(GPIOB, GPIO_Pin_2) == 0)
        {
            wex_setText(TEXT_ID, wex_uintToStr(1000));      // 设置控件文本内容，可以被设置的控件类型：按键、文本、输入框
            wex_setText(TEXT_ID, wex_floatToStr(9.28, 2));
            wex_setText(BUTTON_ID, "按键");                 // 内容为中文时需要为 UTF-8 编码，最多支持 30 个字母或 10 个汉字
        }

        if (GPIO_Read(GPIOB, GPIO_Pin_3) == 0)
        {
            wex_setBackColor(BUTTON_ID, eWexColor_Skyblue); // 设置控件背景颜色种类，可以被设置的控件类型：按键、文本、输入框
            wex_setBackRGB(BUTTON_ID, 135, 206, 250);       // 设置控件背景颜色 RGB 值，可以被设置的控件类型：按键、文本、输入框
        }

        if (GPIO_Read(GPIOB, GPIO_Pin_4) == 0)
        {
            wex_setTextColor(TEXT_ID, eWexColor_Skyblue);   // 设置控件文本颜色种类，可以被设置的控件类型：按键、文本、输入框
            wex_setTextRGB(TEXT_ID, 135, 206, 250);         // 设置控件文本颜色 RGB 值，可以被设置的控件类型：按键、文本、输入框
        }

        if (GPIO_Read(GPIOB, GPIO_Pin_5) == 0)
            wex_setFontSize(TEXT_ID, 100);                  // 设置控件文本字体大小，字体范围 10~255，可以被设置的控件类型：按键、文本、输入框

        delay_ms(20); // 延时不是必须，如果发送指令较多建议加上，防止数据发送过快产生丢包，建议为 10~50ms
    }
}