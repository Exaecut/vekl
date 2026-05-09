# Color — YIQ

Rec.601 (NTSC) luma/chroma color-space conversion.

Source: [`color/yiq.slang`](../../../color/yiq.slang)

---

## Public Functions

### `RGBToYIQ`

```cpp
float3 RGBToYIQ(float3 rgb)
```

Encodes sRGB into Rec.601 YIQ. `.x` is luminance, `.yz` are the in-phase /
quadrature chroma axes. Useful for analog-TV-style effects that need to
manipulate luma and chroma independently (chroma subsampling, bleaching).

### `YIQToRGB`

```cpp
float3 YIQToRGB(float3 yiq)
```

Inverse of [`RGBToYIQ`]. May produce values outside `[0, 1]` after heavy
chroma manipulation — callers should clamp if writing to an unsigned-norm
target.
