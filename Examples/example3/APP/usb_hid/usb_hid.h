#ifndef __USB_HID_H__
#define __USB_HID_H__

#include <stdbool.h>
#include <stdint.h>

#include "CH57x_common.h"

// 按键控制值
#define HID_KEY_NONE            0x00
// 字母键
#define HID_KEY_A              	0x04 // A
#define HID_KEY_B              	0x05 // B
#define HID_KEY_C              	0x06 // C
#define HID_KEY_D              	0x07 // D
#define HID_KEY_E              	0x08 // E
#define HID_KEY_F              	0x09 // F
#define HID_KEY_G              	0x0A // G
#define HID_KEY_H              	0x0B // H
#define HID_KEY_I              	0x0C // I
#define HID_KEY_J              	0x0D // J
#define HID_KEY_K              	0x0E // K
#define HID_KEY_L              	0x0F // L
#define HID_KEY_M              	0x10 // M
#define HID_KEY_N              	0x11 // N
#define HID_KEY_O              	0x12 // O
#define HID_KEY_P              	0x13 // P
#define HID_KEY_Q              	0x14 // Q
#define HID_KEY_R              	0x15 // R
#define HID_KEY_S              	0x16 // S
#define HID_KEY_T              	0x17 // T
#define HID_KEY_U              	0x18 // U
#define HID_KEY_V              	0x19 // V
#define HID_KEY_W              	0x1A // W
#define HID_KEY_X              	0x1B // X
#define HID_KEY_Y              	0x1C // Y
#define HID_KEY_Z              	0x1D // Z
	
// 数字键	
#define HID_KEY_1              	0x1E // 1
#define HID_KEY_2              	0x1F // 2
#define HID_KEY_3              	0x20 // 3
#define HID_KEY_4              	0x21 // 4
#define HID_KEY_5              	0x22 // 5
#define HID_KEY_6              	0x23 // 6
#define HID_KEY_7              	0x24 // 7
#define HID_KEY_8              	0x25 // 8
#define HID_KEY_9              	0x26 // 9
#define HID_KEY_0              	0x27 // 0
	
// 功能键	
#define HID_KEY_ENTER          	0x28 // Enter
#define HID_KEY_ESC            	0x29 // Escape
#define HID_KEY_BACKSPACE      	0x2A // Backspace
#define HID_KEY_TAB            	0x2B // Tab
#define HID_KEY_SPACE          	0x2C // Spacebar
#define HID_KEY_MINUS          	0x2D // -
#define HID_KEY_EQUALS         	0x2E // =
#define HID_KEY_LEFT_BRACKET   	0x2F // [
#define HID_KEY_RIGHT_BRACKET  	0x30 // ]
#define HID_KEY_BACKSLASH      	0x31 // \
#define HID_KEY_SEMICOLON      	0x33 // ;
#define HID_KEY_APOSTROPHE     	0x34 // '
#define HID_KEY_GRAVE          	0x35 // `
#define HID_KEY_COMMA          	0x36 // ,
#define HID_KEY_PERIOD         	0x37 // .
#define HID_KEY_SLASH          	0x38 // /
#define HID_KEY_CAPS_LOCK      	0x39 // Caps Lock
	
// 功能键 F1-F12	
#define HID_KEY_F1             	0x3A // F1
#define HID_KEY_F2             	0x3B // F2
#define HID_KEY_F3             	0x3C // F3
#define HID_KEY_F4             	0x3D // F4
#define HID_KEY_F5             	0x3E // F5
#define HID_KEY_F6             	0x3F // F6
#define HID_KEY_F7             	0x40 // F7
#define HID_KEY_F8             	0x41 // F8
#define HID_KEY_F9             	0x42 // F9
#define HID_KEY_F10            	0x43 // F10
#define HID_KEY_F11            	0x44 // F11
#define HID_KEY_F12            	0x45 // F12
	
// 控制键	
#define HID_KEY_PRINT_SCREEN   	0x46 // Print Screen
#define HID_KEY_SCROLL_LOCK    	0x47 // Scroll Lock
#define HID_KEY_PAUSE          	0x48 // Pause
#define HID_KEY_INSERT         	0x49 // Insert
#define HID_KEY_HOME           	0x4A // Home
#define HID_KEY_PAGE_UP        	0x4B // Page Up
#define HID_KEY_DELETE         	0x4C // Delete
#define HID_KEY_END            	0x4D // End
#define HID_KEY_PAGE_DOWN      	0x4E // Page Down
#define HID_KEY_ARROW_RIGHT    	0x4F // →
#define HID_KEY_ARROW_LEFT     	0x50 // ←
#define HID_KEY_ARROW_DOWN     	0x51 // ↓
#define HID_KEY_ARROW_UP       	0x52 // ↑

// 鼠标控制值
#define HID_MOUSE_NONE        	0x00
#define HID_MOUSE_LEFT        	0x01
#define HID_MOUSE_RIGHT       	0x02
#define HID_MOUSE_MID         	0x04

// 鼠标中键滚动控制值
#define HID_MOUSE_MID_UP      	1
#define HID_MOUSE_MID_DOWN    	-1

void USB_HidInit(void);
void DevHIDKeyReport(uint8_t key);
void DevHIDKeyShiftReport(uint8_t enable);
void DevHIDMouseReport(uint8_t mouse);
void DevHIDMouseMidReport(uint8_t mid);
void DevHIDMouseMoveUpReport(void);
void DevHIDMouseMoveLeftReport(void);
void DevHIDMouseMoveDownReport(void);
void DevHIDMouseMoveRightReport(void);

#endif // __USB_HID_H__
