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

#define WEX_DEBUG_ENABLE            0           // wexcube 打印调试开关

#define WEX_REC_BUF_SIZE            256         // wexcube 接收缓存大小，最小 64，最大 4096
#define WEX_TRS_BUF_SIZE            512         // wexcube 发送缓存大小，最小 128，最大 4096
#define WEX_TRS_MAX_ONCE            20          // wexcube 一次发送最大字节数，最小 1，最大 256，如果不使用模块请根据 BLE 的 MTU(默认20) 设置

#ifdef __cplusplus
}
#endif

#endif /* __WEXCUBE_CONFIG_H__ */
