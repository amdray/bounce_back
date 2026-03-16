#ifndef ENDIAN_UTILS_H
#define ENDIAN_UTILS_H

#include <stdint.h>

static inline uint32_t endian_read_be32_u32(const uint8_t* p) {
    return ((uint32_t)p[0] << 24) |
           ((uint32_t)p[1] << 16) |
           ((uint32_t)p[2] << 8) |
           (uint32_t)p[3];
}

static inline int32_t endian_read_be32_i32(const uint8_t* p) {
    return (int32_t)endian_read_be32_u32(p);
}

static inline uint32_t endian_read_be32_advance(const uint8_t** p) {
    uint32_t v = endian_read_be32_u32(*p);
    *p += 4;
    return v;
}

#endif // ENDIAN_UTILS_H
