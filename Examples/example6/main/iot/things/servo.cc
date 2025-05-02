#include "iot/thing.h"
#include "board.h"
#include "audio_codec.h"

#include <esp_log.h>
#include <driver/ledc.h>

#define TAG "Servo"

namespace iot
{
    // 这里仅定义 Servo 的属性和方法，不包含具体的实现
    class Servo : public Thing
    {
    public:
        Servo() : Thing("Servo", userDev1Name)
        {
            // 定义设备的属性
            properties_.AddNumberProperty("angle", "当前角度值", [this]() -> int {
                return Board::GetInstance().GetServoAngle();
            });

            // 定义设备可以被远程执行的指令
            methods_.AddMethod("SetAngle", "设置角度", ParameterList({
                Parameter("angle", "0到180之间的整数", kValueTypeNumber, true)
            }), [this](const ParameterList& parameters) {
                Board::GetInstance().SetServoAngle(static_cast<uint8_t>(parameters["angle"].number()));
            });
        }
    };

} // namespace iot

DECLARE_THING(Servo);
