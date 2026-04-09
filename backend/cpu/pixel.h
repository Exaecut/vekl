#pragma once

typedef void pixel;

struct layout_rgba {
    inline float4 to_rgba(float4 c) const { return c; }
    inline float4 from_rgba(float4 c) const { return c; }
};

struct layout_bgra {
    inline float4 to_rgba(float4 c) const { return c; }
    inline float4 from_rgba(float4 c) const { return c; }
};

inline float4 pixel_load(device const pixel *data, uint pitch_px, uint2 xy) {
    uint idx = xy.y * pitch_px + xy.x;
    switch (__cpu_format) {
        case 4: {
            const uint8_t *p = reinterpret_cast<const uint8_t *>(data) + idx * 4;
            return float4(p[2] / 255.0f, p[1] / 255.0f, p[0] / 255.0f, p[3] / 255.0f);
        }
        case 8: {
            const uint16_t *p = reinterpret_cast<const uint16_t *>(reinterpret_cast<const uint8_t *>(data) + idx * 8);
            return float4(p[2] / 65535.0f, p[1] / 65535.0f, p[0] / 65535.0f, p[3] / 65535.0f);
        }
        case 16: {
            return reinterpret_cast<const float4 *>(data)[idx];
        }
        default: return float4(0.0f);
    }
}

inline void pixel_store(device pixel *data, uint pitch_px, uint2 xy, float4 c) {
    uint idx = xy.y * pitch_px + xy.x;
    switch (__cpu_format) {
        case 4: {
            uint8_t *p = reinterpret_cast<uint8_t *>(data) + idx * 4;
            p[0] = (uint8_t)clamp(c.z * 255.0f, 0.0f, 255.0f);
            p[1] = (uint8_t)clamp(c.y * 255.0f, 0.0f, 255.0f);
            p[2] = (uint8_t)clamp(c.x * 255.0f, 0.0f, 255.0f);
            p[3] = (uint8_t)clamp(c.w * 255.0f, 0.0f, 255.0f);
            break;
        }
        case 8: {
            uint16_t *p = reinterpret_cast<uint16_t *>(reinterpret_cast<uint8_t *>(data) + idx * 8);
            p[0] = (uint16_t)clamp(c.z * 65535.0f, 0.0f, 65535.0f);
            p[1] = (uint16_t)clamp(c.y * 65535.0f, 0.0f, 65535.0f);
            p[2] = (uint16_t)clamp(c.x * 65535.0f, 0.0f, 65535.0f);
            p[3] = (uint16_t)clamp(c.w * 65535.0f, 0.0f, 65535.0f);
            break;
        }
        case 16: {
            reinterpret_cast<float4 *>(data)[idx] = c;
            break;
        }
    }
}
