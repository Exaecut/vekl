# Sampling — Coordinate

UV coordinate and pixel coordinate conversion utilities.

Source: [`sampling/coordinate.slang`](../../sampling/coordinate.slang)

---

## Functions

### `TexCoord`

```cpp
float2 TexCoord(uint2 gid, uint2 sizePx)
```

Convert integer pixel coordinates to normalized UV `[0, 1]`.
Uses half-pixel offset: `(gid + 0.5) / sizePx`.

### `PixelCoord`

```cpp
float2 PixelCoord(float2 uv, uint2 sizePx)
```

Convert normalized UV back to sub-pixel coordinates.
Inverse of `TexCoord`: `uv * sizePx - 0.5`.

### `ScaleUV`

```cpp
float2 ScaleUV(float2 uv, float2 scale)
float2 ScaleUV(float2 uv, float scale)
```

Scale UV coordinates around center `(0.5, 0.5)`. Values less than `1.0`
zoom in, values greater than `1.0` zoom out. Uniform scale variant available.

### `RotateUV`

```cpp
float2 RotateUV(float2 uv, float angle, float aspect)
```

Rotate UV coordinates around center `(0.5, 0.5)` with aspect ratio
correction. `aspect` (width / height) prevents elliptical distortion.

### `UniformAspectRatio`

```cpp
float2 UniformAspectRatio(float2 uv, uint2 sizePx)
```

Scale UV coordinates to maintain uniform aspect ratio. Stretches X by
the aspect ratio so that a circle in pixel space maps to a circle in UV space.
