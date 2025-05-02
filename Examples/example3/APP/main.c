/******************************************************************************/
/* 头文件包含 */
#include "CONFIG.h"
#include "HAL.h"
#include "gattprofile.h"
#include "peripheral.h"

#include "wexcube.h"

#include "CH57x_common.h"
#include "usb_hid.h"

// 控件ID定义
#define KEY_UP_ID           10  // 向上键
#define KEY_LEFT_ID         11  // 向左键
#define KEY_DOWN_ID         12  // 向下键
#define KEY_RIGHT_ID        13  // 向右键
#define KEY_TAB_ID          14  // Tab键
#define KEY_ENTER_ID        15  // Enter键
#define KEY_BACK_ID         16  // Backspace键
#define KEY_SHIFT_ID        17  // Shift键

#define MOUSE_LEFT_ID       30  // 鼠标左键
#define MOUSE_MID_UP_ID     31  // 鼠标中键向上滚动
#define MOUSE_MID_DOWN_ID   32  // 鼠标中键向下滚动
#define MOUSE_RIGHT_ID      33  // 鼠标右键
#define MOUSE_UP_MOVE_ID    34  // 鼠标向上移动
#define MOUSE_LEFT_MOVE_ID  35  // 鼠标向左移动
#define MOUSE_DOWN_MOVE_ID  36  // 鼠标向下移动
#define MOUSE_RIGHT_MOVE_ID 37  // 鼠标向右移动

/*********************************************************************
 * GLOBAL TYPEDEFS
 */
__attribute__((aligned(4))) u32 MEM_BUF[BLE_MEMHEAP_SIZE / 4];

#if(defined(BLE_MAC)) && (BLE_MAC == TRUE)
u8C MacAddr[6] = {0x84, 0xC2, 0xE4, 0x03, 0x02, 0x02};
#endif

extern void app_drv_process(void);
extern void app_drv_init(void);

static uint32_t timeCnt = 0;

/*******************************************************************************
 * Function Name  : Main_Circulation
 * Description    : 主循环
 * Input          : None
 * Output         : None
 * Return         : None
 *******************************************************************************/
__attribute__((section(".highcode")))
void Main_Circulation()
{
    uint32_t lastTimeCnt = 0;
    const t_sWexCmd *psWexCmd;
    uint8_t mouseMidUp      = 0;    // 滚轮向上键是否按下
    uint8_t mouseMidDown    = 0;    // 滚轮向下键是否按下
    uint8_t mouseUp         = 0;    // 鼠标向上键是否按下
    uint8_t mouseLeft       = 0;    // 鼠标向左键是否按下
    uint8_t mouseDown       = 0;    // 鼠标向下键是否按下
    uint8_t mouseRight      = 0;    // 鼠标向右键是否按下
    uint8_t mouseMoveType   = 0;    // 鼠标移动类型：0 - 无移动，1 - 滚轮向上，2 - 滚轮向下，3 - 鼠标向上，4 - 鼠标向左，5 - 鼠标向下，6 - 鼠标向右

    wex_init();
    wex_start();

    while(1)
    {
        TMOS_SystemProcess();
        app_drv_process();

        psWexCmd = wex_process();
        switch (psWexCmd->eCmdType)
        {
            case eWexCmd_Connect:
            break;

            case eWexCmd_Event:
            {
                switch (psWexCmd->ucCtrlId)
                {
                    case KEY_UP_ID:         // 向上键
                    {
                        if (psWexCmd->ucValue)  DevHIDKeyReport(HID_KEY_ARROW_UP);
                        else DevHIDKeyReport(HID_KEY_NONE);
                    }
                    break;

                    case KEY_LEFT_ID:       // 向左键
                    {
                        if (psWexCmd->ucValue)  DevHIDKeyReport(HID_KEY_ARROW_LEFT);
                        else DevHIDKeyReport(HID_KEY_NONE);
                    }
                    break;

                    case KEY_DOWN_ID:       // 向下键
                    {
                        if (psWexCmd->ucValue)  DevHIDKeyReport(HID_KEY_ARROW_DOWN);
                        else DevHIDKeyReport(HID_KEY_NONE);
                    }
                    break;

                    case KEY_RIGHT_ID:      // 向右键
                    {
                        if (psWexCmd->ucValue)  DevHIDKeyReport(HID_KEY_ARROW_RIGHT);
                        else DevHIDKeyReport(HID_KEY_NONE);
                    }
                    break;

                    case KEY_TAB_ID:        // Tab键
                    {
                        if (psWexCmd->ucValue)  DevHIDKeyReport(HID_KEY_TAB);
                        else DevHIDKeyReport(HID_KEY_NONE);
                    }
                    break;

                    case KEY_ENTER_ID:      // Enter键
                    {
                        if (psWexCmd->ucValue)  DevHIDKeyReport(HID_KEY_ENTER);
                        else DevHIDKeyReport(HID_KEY_NONE);
                    }
                    break;

                    case KEY_BACK_ID:       // Backspace键
                    {
                        if (psWexCmd->ucValue)  DevHIDKeyReport(HID_KEY_BACKSPACE);
                        else DevHIDKeyReport(HID_KEY_NONE);
                    }
                    break;

                    case KEY_SHIFT_ID:      // Shift键
                    {
                        if (psWexCmd->ucValue)  DevHIDKeyShiftReport(1);
                        else DevHIDKeyShiftReport(0);
                    }
                    break;

                    case MOUSE_LEFT_ID:     // 鼠标左键
                    {
                        if (psWexCmd->ucValue)  DevHIDMouseReport(HID_MOUSE_LEFT);
                        else DevHIDMouseReport(HID_MOUSE_NONE);
                    }
                    break;

                    case MOUSE_MID_UP_ID:   // 鼠标中键向上滚动
                    {
                        mouseMidUp = psWexCmd->ucValue;
                    }
                    break;

                    case MOUSE_MID_DOWN_ID: // 鼠标中键向下滚动
                    {
                        mouseMidDown = psWexCmd->ucValue;
                    }
                    break;

                    case MOUSE_RIGHT_ID:    // 鼠标右键
                    {
                        if (psWexCmd->ucValue)  DevHIDMouseReport(HID_MOUSE_RIGHT);
                        else DevHIDMouseReport(HID_MOUSE_NONE);
                    }
                    break;

                    case MOUSE_UP_MOVE_ID:       // 鼠标向上移动
                    {
                        mouseUp = psWexCmd->ucValue;
                    }
                    break;

                    case MOUSE_LEFT_MOVE_ID:     // 鼠标向左移动
                    {
                        mouseLeft = psWexCmd->ucValue;
                    }
                    break;

                    case MOUSE_DOWN_MOVE_ID:     // 鼠标向下移动
                    {
                        mouseDown = psWexCmd->ucValue;
                    }
                    break;

                    case MOUSE_RIGHT_MOVE_ID:    // 鼠标向右移动
                    {
                        mouseRight = psWexCmd->ucValue;
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

        // 根据优先级判断移动类型
        if (mouseMidUp)
            mouseMoveType = 1;
        else if (mouseMidDown)
            mouseMoveType = 2;
        else if (mouseUp)
            mouseMoveType = 3;
        else if (mouseLeft)
            mouseMoveType = 4;
        else if (mouseDown)
            mouseMoveType = 5;
        else if (mouseRight)
            mouseMoveType = 6;
        else
            mouseMoveType = 0;

        // 不能直接用 mDelaymS()，它会阻塞程序执行影响 USB HID发送
        if (timeCnt != lastTimeCnt)
        {
            lastTimeCnt = timeCnt;
            switch (mouseMoveType)
            {
                case 1:
                    DevHIDMouseMidReport(HID_MOUSE_MID_UP);
                break;

                case 2:
                    DevHIDMouseMidReport(HID_MOUSE_MID_DOWN);
                break;

                case 3:
                    DevHIDMouseMoveUpReport();
                break;

                case 4:
                    DevHIDMouseMoveLeftReport();
                break;

                case 5:
                    DevHIDMouseMoveDownReport();
                break;

                case 6:
                    DevHIDMouseMoveRightReport();
                break;

                default:
                break;
            }
        }
    }
}

/*******************************************************************************
 * Function Name  : main
 * Description    : 主函数
 * Input          : None
 * Output         : None
 * Return         : None
 *******************************************************************************/
int main(void)
{
    SetSysClock(CLK_SOURCE_PLL_60MHz);
#ifdef DEBUG
    GPIOA_SetBits(bTXD1);
    GPIOA_ModeCfg(bTXD1, GPIO_ModeOut_PP_5mA);
    UART1_DefInit();
#endif
    PRINT("%s\n", VER_LIB);
    CH57X_BLEInit();
    HAL_Init();
    GAPRole_PeripheralInit();
    Peripheral_Init();
    app_drv_init();

    USB_HidInit();

    TMR0_TimerInit(FREQ_SYS / 10);         // 设置定时时间 100ms
    TMR0_ITCfg(ENABLE, TMR0_3_IT_CYC_END); // 开启中断
    PFIC_EnableIRQ(TMR0_IRQn);

    Main_Circulation();
}

/*********************************************************************
 * @fn      TMR0_IRQHandler
 *
 * @brief   TMR0中断函数
 *
 * @return  none
 */
__attribute__((interrupt("WCH-Interrupt-fast")))
__attribute__((section(".highcode")))
void TMR0_IRQHandler(void) // TMR0 定时中断
{
    if(TMR0_GetITFlag(TMR0_3_IT_CYC_END))
    {
        TMR0_ClearITFlag(TMR0_3_IT_CYC_END); // 清除中断标志
        timeCnt++;
    }
}

/******************************** endfile @ main ******************************/

