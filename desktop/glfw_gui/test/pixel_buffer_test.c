#include "../c/pixel_buffer.h"
#include <assert.h>
#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static uint32_t state = 7193;
static uint32_t next(void) { state = state * UINT32_C(1664525) + UINT32_C(1013904223); return state; }
static uint32_t reference(uint32_t s, uint32_t d) {
    double a = (s >> 24) / 255.0, b = (d >> 24) / 255.0, out = a + b * (1 - a);
    if (!a) return d;
    uint32_t result = (uint32_t)lround(out * 255) << 24;
    for (int shift = 0; shift <= 16; shift += 8)
        result |= (uint32_t)lround((((s >> shift) & 255) * a + ((d >> shift) & 255) * b * (1 - a)) / out) << shift;
    return result;
}
int main(void) {
    assert(pixel_source_over(0x80ff0000, 0xff0000ff) == 0xff80007f);
    assert(pixel_source_over(0x8040a0f0, 0) == 0x8040a0f0);
    for (int trial = 0; trial < 1000; trial++) {
        uint32_t source[64], actual[64], expected[64];
        uint8_t bytes[256];
        for (int i = 0; i < 64; i++) { source[i] = next(); actual[i] = expected[i] = next(); }
        assert(pixel_argb_bytes(source, 64, bytes, 256, 64, 1));
        uint32_t roundtrip[64];
        assert(pixel_argb_bytes(roundtrip, 64, bytes, 256, 64, 0));
        assert(!memcmp(source, roundtrip, sizeof source));
        assert(bytes[0] == ((source[0] >> 16) & 255) && bytes[3] == (source[0] >> 24));
        int width = 1 + (next() >> 16) % 8, height = 1 + (next() >> 16) % 8;
        int reverse = (next() >> 16) & 1, stride = reverse ? -8 : 8, offset = reverse ? (height - 1) * 8 : 0;
        int alpha = (next() >> 16) & 1;
        for (int y = 0; y < height; y++) for (int x = 0; x < width; x++) {
            uint32_t s = source[offset + y * stride + x];
            expected[y * 8 + x] = alpha ? reference(s, expected[y * 8 + x]) : s | UINT32_C(0xff000000);
        }
        assert(pixel_argb_blit(source, 64, offset, stride, actual, 64, 0, 8, width, height, alpha));
        for (int i = 0; i < 64; i++) for (int shift = 0; shift <= 24; shift += 8)
            assert(abs((int)((actual[i] >> shift) & 255) - (int)((expected[i] >> shift) & 255)) <= 1);
        memcpy(expected, actual, sizeof actual);
        assert(!pixel_argb_blit(source, 64, INT_MAX, INT_MIN, actual, 64, 0, 8, 8, INT_MAX, 1));
        assert(!pixel_argb_blit(source, 64, 0, -8, actual, 64, 0, 8, 8, 8, 1));
        assert(!pixel_argb_blit(actual, 64, 0, 8, actual, 64, 0, 8, 8, 8, 1));
        assert(!pixel_argb_bytes(actual, 64, bytes, 255, 64, 0));
        assert(!memcmp(actual, expected, sizeof actual));
    }
    puts("Native pixels: 1000 alpha, stride, conversion and bounds cases passed.");
}
