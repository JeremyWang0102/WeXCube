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
#define PREV_ID             10  // 上一首
#define PLAY_ID             11  // 播放暂停
#define NEXT_ID             12  // 下一首
#define VOLUME_UP_ID        13  // 声音增加
#define VOLUME_DOWN_ID      14  // 声音减小
#define VOLUME_MUTE_ID      15  // 静音


/*********************************************************************
 * GLOBAL TYPEDEFS
 */
__attribute__((aligned(4))) u32 MEM_BUF[BLE_MEMHEAP_SIZE / 4];

#if(defined(BLE_MAC)) && (BLE_MAC == TRUE)
u8C MacAddr[6] = {0x84, 0xC2, 0xE4, 0x03, 0x02, 0x02};
#endif

extern void app_drv_process(void);
extern void app_drv_init(void);

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
    const t_sWexCmd *psWexCmd;

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
                    case PREV_ID:           // 上一首
                    {
                        if (psWexCmd->ucValue)  DevHIDVolumeCodeReport(PREV_CODE);
                        else DevHIDVolumeCodeReport(NONE_CODE);
                    }
                    break;

                    case PLAY_ID:           // 播放暂停
                    {
                        if (psWexCmd->ucValue)  DevHIDVolumeCodeReport(PLAY_CODE);
                        else DevHIDVolumeCodeReport(NONE_CODE);
                    }
                    break;

                    case NEXT_ID:           // 下一首
                    {
                        if (psWexCmd->ucValue)  DevHIDVolumeCodeReport(NEXT_CODE);
                        else DevHIDVolumeCodeReport(NONE_CODE);
                    }
                    break;

                    case VOLUME_UP_ID:      // 声音增加
                    {
                        if (psWexCmd->ucValue)  DevHIDVolumeCodeReport(VOLUME_UP_CODE);
                        else DevHIDVolumeCodeReport(NONE_CODE);
                    }
                    break;

                    case VOLUME_DOWN_ID:    // 声音减小
                    {
                        if (psWexCmd->ucValue)  DevHIDVolumeCodeReport(VOLUME_DOWN_CODE);
                        else DevHIDVolumeCodeReport(NONE_CODE);
                    }
                    break;

                    case VOLUME_MUTE_ID:    // 静音
                    {
                        if (psWexCmd->ucValue)  DevHIDVolumeCodeReport(VOLUME_MUTE_CODE);
                        else DevHIDVolumeCodeReport(NONE_CODE);
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

    Main_Circulation();
}

/******************************** endfile @ main ******************************/

