#ifndef __USB_HID_H__
#define __USB_HID_H__

#include <stdbool.h>
#include <stdint.h>

#include "CH57x_common.h"

// ÉùÒô¿ØÖÆ´úÂë
#define NONE_CODE           0x00
#define PLAY_CODE           0x01
#define NEXT_CODE           0x02
#define PREV_CODE           0x04
#define VOLUME_MUTE_CODE    0x08
#define VOLUME_UP_CODE      0x10
#define VOLUME_DOWN_CODE    0x20
#define FORWARD_CODE        0x40
#define BACKWARD_CODE       0x80

void USB_HidInit(void);
void DevHIDVolumeCodeReport(uint8_t VolumeCode);

#endif // __USB_HID_H__
