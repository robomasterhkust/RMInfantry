#ifndef EXTERNAL_CONTROL_H
#define EXTERNAL_CONTROL_H

void external_control();
void remote_control();
void computer_control();
void remote_buff_adjust();
void process_mouse_data();
void process_keyboard_data();

void dancing_mode();

void DBUS_disconnect_handler();
void gimbal_disconnect_handler();
void chassis_disconnect_handler();

#endif
