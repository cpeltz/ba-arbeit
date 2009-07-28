#include "flags.h"

/**
 * Static storage for the global flags used throughout the program.
 * @todo Remove need for flags or migrate more settings to flags
 */
static uint32_t flags_global_storage = 0;

/**
 * Sets one of the global flags.
 *
 * @param[in] flag Specifies, which flag should be set.
 */
void flag_set(uint8_t flag) {
	flags_global_storage |= ((uint32_t) 1 << flag);
}

/**
 * Clears one of the global flags.
 *
 * @param[in] flag Specifies, which flag should be cleared.
 */
void flag_clear(uint8_t flag) {
	flags_global_storage &= ~((uint32_t) 1 << flag);
}

/**
 * Reads one of the global flags.
 *
 * @param[in] flag The flag which should be read.
 * @return <em>uint8_t</em> Returns 1 if set, 0 otherwise.
 */
uint8_t flag_read(uint8_t flag) {
	return ((flags_global_storage & ((uint32_t) 1 << flag)) >> flag);
}

/**
 * Reads one immediatly after that clears a global flag.
 *
 * @param[in] flag The flag which should be operated upon.
 * @return <em>uint8_t</em> Returns 1 if set, 0 otherwise.
 */
uint8_t flag_read_and_clear(uint8_t flag) {
	if ((flags_global_storage & ((uint32_t) 1 << flag))) {
		flag_clear(flag);
		return 1;
	}
	return 0;
}

/**
 * Sets one flag in a local storage.
 *
 * @param[in] storage The local storage byte, in which to set the flag.
 * @param[in] flag Specifies, which flag should be set.
 */
void flag_local_set(uint8_t * storage, const uint8_t flag) {
	*storage |= (1 << flag);
}

/**
 * Clears one flag in a local storage.
 *
 * @param[in] storage The local storage byte, in which to clear the flag.
 * @param[in] flag Specifies, which flag should be cleared.
 */
void flag_local_clear(uint8_t * storage, const uint8_t flag) {
	*storage &= ~(1 << flag);
}

/**
 * Reads one flag in a local storage.
 *
 * @param[in] storage The local storage byte, in which to read the flag.
 * @param[in] flag Specifies, which flag should be read.
 * @return <em>uint8_t</em> Returns 1 if set, 0 otherwise.
 */
uint8_t flag_local_read(const uint8_t * storage, const uint8_t flag) {
	return ((*storage & (1 << flag)) >> flag);
}

/**
 * Reads one flag in a local storage and clears it after that.
 *
 * @param[in] storage The local storage byte, in which to read and clear the flag.
 * @param[in] flag Specifies, which flag should be read and cleared.
 * @return <em>uint8_t</em> Returns 1 if set, 0 otherwise.
 */
uint8_t flag_local_read_and_clear(uint8_t * storage, const uint8_t flag) {
	if ((*storage & (1 << flag))) {
		flag_local_clear(storage, flag);
		return 1;
	}
	return 0;
}
