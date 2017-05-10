#include "main.h"
#include "function_list.h"
#include "stm32f4xx.h"
#include "global_variable.h"
#include "external_control.h"
#include "chassis_control.h"
#include "gimbal_control.h"

void TIM7_IRQHandler(void) {
    static uint32_t tick = 0;

    TIM_ClearITPendingBit(TIM7, TIM_IT_Update);
    TIM_ClearFlag(TIM7, TIM_FLAG_Update);

    ++tick;
    if (tick == 1000) {
        tick = 0;
    }

    if (tick % 500 == 0) {
        LED_blink(LED1);
    }

    if (tick % 20 == 0)
    {
        CanCheckConnection_for_Chassis();
        CanCheckConnection_for_Gimbal();
        Chassis_Connected = can_chassis_connected[0] & can_chassis_connected[1] & can_chassis_connected[2] & can_chassis_connected[3];
        Gimbal_Connected = can_gimbal_connected[0] & can_gimbal_connected[1];
        DBUS_Connected = DBUS_CheckConnection();
        GUN_ENCODER_Connected = ENCODER_CheckConnection();
        GUN_EncoderUpdate();

        if (!GUN_ENCODER_Connected || clearing_ammo)
        {
            GUN_SetFree();
        }
        else GUN_PokeControl();
    }
    current_angle = output_angle;
    external_control();
}