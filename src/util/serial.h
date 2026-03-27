#ifndef ER_UTIL_SERIAL_H
#define ER_UTIL_SERIAL_H

#include <inttypes.h>

static inline uint16_t er_read_u16(uint8_t const *buf) {
    return ((buf[0] & 0xFF) << 8) | (buf[1] & 0xFF);
}

static inline void er_write_u16(uint8_t *buf, uint16_t u16) {
    buf[0] = (u16 >> 8) & 0xFF;
    buf[1] = u16 & 0xFF;
}

#endif
