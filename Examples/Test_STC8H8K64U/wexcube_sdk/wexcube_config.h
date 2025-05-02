/**
 * @file wexcube_config.h
 * @author JeremyWang (jeremywang0102@gmail.com / gin0101@126.com)
 * @brief WeXCube 配置头文件
 * @version 
 * @date 2024-11-25
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#ifndef __WEXCUBE_CONFIG_H__
#define __WEXCUBE_CONFIG_H__

#ifdef __cplusplus
extern "C" {
#endif

#define WEX_REC_BUF_SIZE            64         	// wexcube 接收缓存大小，最小 64，最大 4095
#define WEX_TRS_BUF_SIZE            128         // wexcube 发送缓存大小，最小 128，最大 4095
#define WEX_TRS_MAX_ONCE            20         	// wexcube 一次发送最大字节数，最小 1，最大 255


// wexcube.c 中函数条件编译开关，不需要编译的直接屏蔽，可以减小 ROM 消耗
//#define		WEX_ASKHANDSHAKE
//#define		WEX_SENDDISCONNECT
//#define		WEX_ASKERR
//#define		WEX_ASKDATE
//#define		WEX_ASKTIME

//#define		WEX_ASKVALUE
//#define		WEX_ASKTEXT
//#define		WEX_ASKBACKRGB
//#define		WEX_ASKTEXTRGB
//#define		WEX_ASKFONTSIZE
	
#define		WEX_SETVALUE
#define		WEX_SETTEXT
//#define		WEX_SETBACKCOLOR
//#define		WEX_SETTEXTCOLOR
//#define		WEX_SETBACKRGB
//#define		WEX_SETTEXTRGB
//#define		WEX_SETFONTSIZE

//#define		WEX_STRTOINT
#define		WEX_STRTOUINT
//#define		WEX_INTTOSTR
#define		WEX_UINTTOSTR
//#define		WEX_STRTOFLOAT
//#define		WEX_FLOATTOSTR


#ifdef __cplusplus
}
#endif

#endif /* __WEXCUBE_CONFIG_H__ */
