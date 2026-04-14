#pragma once

typedef void pixel;

struct layout_rgba {
    inline float4 to_rgba(float4 c) const { return c; }
    inline float4 from_rgba(float4 c) const { return c; }
};

struct layout_bgra {
    inline float4 to_rgba(float4 c) const { return float4(c.z, c.y, c.x, c.w); }
    inline float4 from_rgba(float4 c) const { return float4(c.z, c.y, c.x, c.w); }
};

struct layout_vuya {

    inline float4 to_rgba(float4 c) const {
        // For 32f VUYA (__cpu_format==16), chroma is already centered at 0 (range [-0.5, 0.5]).
        // For 8u/16u VUYA, chroma is in [0,1] after normalization and needs 0.5 offset.
        float chroma_offset = (__cpu_format == 16) ? 0.0f : 0.5f;
        float v = c.x - chroma_offset;
        float u = c.y - chroma_offset;
        float y = c.z;
        return float4(
            y + 1.402f * v,
            y - 0.344136f * u - 0.714136f * v,
            y + 1.772f * u,
            c.w
        );
    }

    inline float4 from_rgba(float4 c) const {
        float chroma_offset = (__cpu_format == 16) ? 0.0f : 0.5f;
        float y = 0.299f * c.x + 0.587f * c.y + 0.114f * c.z;
        return float4(
            (c.x - y) / 1.402f + chroma_offset,
            (c.z - y) / 1.772f + chroma_offset,
            y,
            c.w
        );
    }
};

struct layout_vuya709 {

    inline float4 to_rgba(float4 c) const {
        float chroma_offset = (__cpu_format == 16) ? 0.0f : 0.5f;
        float v = c.x - chroma_offset;
        float u = c.y - chroma_offset;
        float y = c.z;
        return float4(
            y + 1.5748f * v,
            y - 0.1873f * u - 0.4681f * v,
            y + 1.8556f * u,
            c.w
        );
    }

    inline float4 from_rgba(float4 c) const {
        float chroma_offset = (__cpu_format == 16) ? 0.0f : 0.5f;
        float y = 0.2126f * c.x + 0.7152f * c.y + 0.0722f * c.z;
        return float4(
            (c.x - y) / 1.5748f + chroma_offset,
            (c.z - y) / 1.8556f + chroma_offset,
            y,
            c.w
        );
    }
};

struct layout_auto {
    uint layout_type;

    inline float4 to_rgba(float4 c) const {
        switch (layout_type) {
            case 2: return layout_vuya().to_rgba(c);
            case 3: return layout_vuya709().to_rgba(c);
            case 1: return layout_bgra().to_rgba(c);
            default: return c;
        }
    }

    inline float4 from_rgba(float4 c) const {
        switch (layout_type) {
            case 2: return layout_vuya().from_rgba(c);
            case 3: return layout_vuya709().from_rgba(c);
            case 1: return layout_bgra().from_rgba(c);
            default: return c;
        }
    }
};

inline float4 pixel_load(device const pixel *data, uint pitch_px, uint2 xy) {
    uint idx = xy.y * pitch_px + xy.x;
    switch (__cpu_format) {
        case 4: {
            const uint8_t *p = reinterpret_cast<const uint8_t *>(data) + idx * 4;
            return float4(p[0] / 255.0f, p[1] / 255.0f, p[2] / 255.0f, p[3] / 255.0f);
        }
        case 8: {
            const uint16_t *p = reinterpret_cast<const uint16_t *>(reinterpret_cast<const uint8_t *>(data) + idx * 8);
            return float4(p[0] / 65535.0f, p[1] / 65535.0f, p[2] / 65535.0f, p[3] / 65535.0f);
        }
        case 16: {
            const float *p = reinterpret_cast<const float *>(data) + idx * 4;
            return float4(p[0], p[1], p[2], p[3]);
        }
        default: return float4(0.0f);
    }
}

inline void pixel_store(device pixel *data, uint pitch_px, uint2 xy, float4 c) {
    uint idx = xy.y * pitch_px + xy.x;
    switch (__cpu_format) {
        case 4: {
            uint8_t *p = reinterpret_cast<uint8_t *>(data) + idx * 4;
            p[0] = (uint8_t)clamp(c.x * 255.0f, 0.0f, 255.0f);
            p[1] = (uint8_t)clamp(c.y * 255.0f, 0.0f, 255.0f);
            p[2] = (uint8_t)clamp(c.z * 255.0f, 0.0f, 255.0f);
            p[3] = (uint8_t)clamp(c.w * 255.0f, 0.0f, 255.0f);
            break;
        }
        case 8: {
            uint16_t *p = reinterpret_cast<uint16_t *>(reinterpret_cast<uint8_t *>(data) + idx * 8);
            p[0] = (uint16_t)clamp(c.x * 65535.0f, 0.0f, 65535.0f);
            p[1] = (uint16_t)clamp(c.y * 65535.0f, 0.0f, 65535.0f);
            p[2] = (uint16_t)clamp(c.z * 65535.0f, 0.0f, 65535.0f);
            p[3] = (uint16_t)clamp(c.w * 65535.0f, 0.0f, 65535.0f);
            break;
        }
        case 16: {
            float *p = reinterpret_cast<float *>(data) + idx * 4;
            p[0] = c.x;
            p[1] = c.y;
            p[2] = c.z;
            p[3] = c.w;
            break;
        }
    }
}
