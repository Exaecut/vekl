# Sampling — Bicubic

Catmull-Rom bicubic reconstruction on a `TextureView`.

Source: [`sampling/bicubic.slang`](../../../sampling/bicubic.slang)

---

## Public Functions

### `SampleBicubic`

```cpp
float4 SampleBicubic(TextureView tex, float2 uv)
```

Samples `tex` at `uv` with Catmull-Rom bicubic reconstruction, using 4
bilinear taps via the classic weight-fold trick (same result as 16-tap
bicubic, ~4× cheaper). Prefer over `SampleLinear` when upsampling or
reconstructing a soft signal that would otherwise look stair-stepped.

| Parameter | Description |
|-----------|-------------|
| `tex` | Source view (level 0 only) |
| `uv` | Normalized `[0, 1]` texture coordinate |
| **Returns** | Bicubic-reconstructed RGBA |

---

## Internal Functions

| Function | Description |
|----------|-------------|
| `CubicWeights(v)` | Catmull-Rom cubic B-spline weights for a single axis |
