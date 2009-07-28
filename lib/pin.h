#ifndef PIN_H
#define PIN_H

void pin_init(void);
void pin_set_A(uint8_t pin);
void pin_set_C(uint8_t pin);
void pin_clear_A(uint8_t pin);
void pin_clear_C(uint8_t pin);
void pin_toggle_A(uint8_t pin);
void pin_toggle_C(uint8_t pin);

#endif
