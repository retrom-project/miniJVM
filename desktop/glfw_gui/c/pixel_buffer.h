#ifndef MINI_PIXEL_BUFFER_H
#define MINI_PIXEL_BUFFER_H

#include <stdint.h>

/* All bounds are checked before touching either buffer, including negative strides. */
static inline int pixel_rect_valid(int length, int offset, int stride, int width, int height) {
    if (length < 0 || width <= 0 || height <= 0) return 0;
    int64_t first = offset, last = first + (int64_t)(height - 1) * stride;
    int64_t low = first < last ? first : last;
    int64_t high = first > last ? first : last;
    return low >= 0 && high + width <= length;
}

static inline uint32_t pixel_source_over(uint32_t source, uint32_t destination) {
    uint32_t alpha = source >> 24, destination_alpha = destination >> 24;
    if (!alpha) return destination;
    if (alpha == 255 || !destination_alpha) return source;
    uint32_t sw = alpha * 255, dw = destination_alpha * (255 - alpha), weight = sw + dw;
    uint32_t result = ((weight + 127) / 255) << 24;
    for (int shift = 0; shift <= 16; shift += 8) {
        uint32_t channel = (((source >> shift) & 255) * sw + ((destination >> shift) & 255) * dw + weight / 2) / weight;
        result |= channel << shift;
    }
    return result;
}

static inline int pixel_argb_blit(const uint32_t *source, int source_length, int source_offset, int source_stride,
                                  uint32_t *destination, int destination_length, int destination_offset, int destination_stride,
                                  int width, int height, int process_alpha) {
    if (!source || !destination || source == destination ||
        !pixel_rect_valid(source_length, source_offset, source_stride, width, height) ||
        !pixel_rect_valid(destination_length, destination_offset, destination_stride, width, height)) return 0;
    for (int row = 0; row < height; row++) {
        const uint32_t *src = source + (int64_t)source_offset + (int64_t)row * source_stride;
        uint32_t *dst = destination + (int64_t)destination_offset + (int64_t)row * destination_stride;
        if (!process_alpha) {
            for (int col = 0; col < width; col++) dst[col] = src[col] | UINT32_C(0xff000000);
        } else {
            for (int col = 0; col < width; col++) dst[col] = pixel_source_over(src[col], dst[col]);
        }
    }
    return 1;
}

/* Packed Java straight ARGB <-> byte-ordered straight RGBA; independent of native endianness. */
static inline int pixel_argb_bytes(uint32_t *argb, int argb_length, uint8_t *rgba, int rgba_length, int count, int to_rgba) {
    if (!argb || !rgba || count < 0 || count > argb_length || (int64_t)count * 4 > rgba_length) return 0;
    for (int i = 0; i < count; i++) {
        uint8_t *p = rgba + (int64_t)i * 4;
        if (to_rgba) {
            uint32_t value = argb[i];
            p[0] = value >> 16; p[1] = value >> 8; p[2] = value; p[3] = value >> 24;
        } else {
            argb[i] = ((uint32_t)p[3] << 24) | ((uint32_t)p[0] << 16) | ((uint32_t)p[1] << 8) | p[2];
        }
    }
    return 1;
}
#endif
