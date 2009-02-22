#ifndef FLAGS_H
#define FLAGS_H

// Funktionen für globale Flags
void flag_Set(uint8_t flag);
void flag_Clear(uint8_t flag);
uint8_t flag_Read(uint8_t flag);
uint8_t flag_ReadAndClear(uint8_t flag);

// und für lokale Flags
void flagLocal_Set(uint8_t * storage, const uint8_t flag);
void flagLocal_Clear(uint8_t * storage, const uint8_t flag);
uint8_t flagLocal_Read(const uint8_t * storage, const uint8_t flag);
uint8_t flagLocal_ReadAndClear(uint8_t * storage, const uint8_t flag);

#endif
