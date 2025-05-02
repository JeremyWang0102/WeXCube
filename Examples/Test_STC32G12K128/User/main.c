#include "stc32g.h"
#include "intrins.h"

#include "wexcube.h"

// 设备控制页面控件ID
#define BUTTON_ID   	1   // 按钮控件ID
#define SWITCH_ID     	2   // 开关控件ID
#define TEXT_ID     	3   // 文本框控件ID
#define INPUT_ID     	4   // 输入框控件ID

#define MAIN_Fosc       33177600L   //定义主时钟

wex_u16_t seconds = 0;

//========================================================================
// 函数: void delay_ms(u8 ms)
// 描述: 延时函数。
// 参数: ms,要延时的ms数, 这里只支持1~255ms. 自动适应主时钟.
// 返回: none.
// 版本: VER1.0
// 日期: 2022-6-3
// 备注: 
//========================================================================
void delay_ms(wex_u8_t ms)
{
     wex_u16_t i;
     do{
          i = MAIN_Fosc / 6000;
          while(--i);   //6T per loop
     }while(--ms);
}

void timer0_init(void)
{
	//20毫秒@33.1776MHz
	AUXR &= 0x7F;			//定时器时钟12T模式
	TMOD &= 0xF0;			//设置定时器模式
	TL0 = 0x00;				//设置定时初始值
	TH0 = 0x28;				//设置定时初始值
	TF0 = 0;				//清除TF0标志
	TR0 = 1;				//定时器0开始计时
	ET0 = 1;				//使能定时器0中断
}

void main(void)
{
	P1M0 = 0x00;
    P1M1 = 0x00;
	P2M0 = 0x00;
    P2M1 = 0x00;
	
	wex_init();
	wex_start();
	
	timer0_init();

    EA = 1;
	
	while (1)
	{
		const t_sWexCmd *psWexCmd = wex_process();
		switch (psWexCmd->eCmdType)		// 接收到的指令
		{
			case eWexCmd_Connect:		// WeXCube 连接指令
			{
				P17 = 0;
				
				// 把当前状态发送给页面
				wex_setText(TEXT_ID, wex_uintToStr(seconds));
				wex_setValue(SWITCH_ID, P15);
			}
			break;
			
			case eWexCmd_Disconnect:	// WeXCube 断开指令
			{
				P17 = 1;
			}
			break;

			case eWexCmd_Event:         // 控件事件触发
			{
				switch (psWexCmd->ucCtrlId)
				{
					case BUTTON_ID:
					{
						P14 = psWexCmd->ucValue;
					}
					break;
					
					case SWITCH_ID:
					{
						P15 = psWexCmd->ucValue;
					}
					break;
					
					default:
					break;
				}
			}
			break;
			
			case eWexCmd_Text:		// 控件文本
			{
				switch (psWexCmd->ucCtrlId)
				{
					case INPUT_ID:
					{
						wex_u8_t pin = wex_strToUint(psWexCmd->pcText);
						if (psWexCmd->pcText[0] == '\0') pin = -1;
						
						switch (pin)
						{
							case 0:
								P20 = 0;
							break;
							
							case 1:
								P21 = 0;
							break;
							
							case 2:
								P22 = 0;
							break;
							
							case 3:
								P23 = 0;
							break;
							
							case 4:
								P24 = 0;
							break;
							
							case 5:
								P25 = 0;
							break;
							
							case 6:
								P26 = 0;
							break;
							
							case 7:
								P27 = 0;
							break;
							
							default:
								P2 = 0xff;
							break;
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
		
		// WeXCube 连接好就向页面传输计数值
		if (wex_getConnectState())
		{
			static wex_u16_t lastSeconds = 0;
			if (lastSeconds != seconds)
			{
				wex_setText(TEXT_ID, wex_uintToStr(seconds));
				lastSeconds = seconds;
			}
		}
		
		// 向 WeXCube 发送数据时不能太频繁容易造成蓝牙发送阻塞
		delay_ms(10);
	}
}


// 定时器0中断
void Timer0_Isr(void) interrupt 1
{
	static wex_u8_t n = 0;
	
	if (n++ >= 50)
	{
		n = 0;
		seconds++;
	}
}