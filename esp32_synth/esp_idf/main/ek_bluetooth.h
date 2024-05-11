
#ifndef EK_BLUETOOTH_H
#define EK_BLUETOOTH_H

void ek_bluetooth_start(void (*callback)(uint16_t length, uint8_t *data));

#endif