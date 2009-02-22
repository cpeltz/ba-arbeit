#include <inttypes.h>
#include "flags.h"

static uint32_t flags_global_storage = 0;

// globale Flagge setzen
void flag_set(uint8_t flag) {
	flags_global_storage |= ((uint32_t) 1 << flag);
}

// globale Flagge löschen
void flag_clear(uint8_t flag) {
	flags_global_storage &= ~((uint32_t) 1 << flag);
}

// globale Flagge lesen
uint8_t flag_read(uint8_t flag) {
	return ((flags_global_storage & ((uint32_t) 1 << flag)) >> flag);
}

// globale Flagge lesen und löschen
uint8_t flag_read_and_clear(uint8_t flag) {
	if ((flags_global_storage & ((uint32_t) 1 << flag))) {
		flag_clear(flag);
		return 1;
	}
	return 0;
}

// lokale Flagge setzen
void flag_local_set(uint8_t * storage, const uint8_t flag) {
	*storage |= (1 << flag);
}

// lokale Flagge löschen
void flag_local_clear(uint8_t * storage, const uint8_t flag) {
	*storage &= ~(1 << flag);
}

// lokale Flagge lesen
uint8_t flag_local_read(const uint8_t * storage, const uint8_t flag) {
	return ((*storage & (1 << flag)) >> flag);
}

// lokale Flagge lesen und löschen
uint8_t flag_local_read_and_clear(uint8_t * storage, const uint8_t flag) {
	if ((*storage & (1 << flag))) {
		flag_local_clear(storage, flag);
		return 1;
	}
	return 0;
}
