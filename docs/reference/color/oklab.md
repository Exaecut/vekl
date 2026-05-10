# Color — Oklab

Perceptually uniform color space (Björn Ottosson, 2020). Splits luma `L`
from a green↔red axis `a` and a blue↔yellow axis `b`. Hue is the polar
angle of `(a, b)`; chroma is its magnitude.

Source: [`color/oklab.slang`](../../../color/oklab.slang)

---

## Public Functions

### `RGBToOklab`

```cpp
float3 RGBToOklab(float3 rgb)
```

Encodes linear RGB into Oklab. Returns `(L, a, b)`. `L` ≈ apparent
lightness in `[0, 1]` for normal sRGB inputs; `a` and `b` are signed
chroma axes (typical magnitudes `< 0.4`).

### `OklabToRGB`

```cpp
float3 OklabToRGB(float3 lab)
```

Inverse of [`RGBToOklab`]. After a hue rotation or chroma boost the
result may overshoot `[0, 1]`; clamp before storing into an unsigned-norm
target.

### `HueShiftOklab`

```cpp
float3 HueShiftOklab(float3 rgb, float shift)
```

Rotates hue by `shift` **radians** in the Oklab `(a, b)` plane. Luma and
chroma magnitude are preserved, so equal `shift` steps look like equal
hue steps to the eye — unlike a naive RGB-around-the-grey-axis rotation,
which compresses the green/blue arc and stretches the red/yellow one.

| Parameter | Description |
|-----------|-------------|
| `rgb`   | Linear RGB color |
| `shift` | Hue rotation in radians (positive = red→yellow→green→cyan…) |
| **Returns** | Linear RGB after the perceptual hue rotation |

## When to use

* **Hue shift / cycle effects** — perceptually uniform, no luma drift.
* **Color-grading masks** that target a hue range — work in Oklab `(a, b)`
  polar coordinates, not HSV.
* **Glow / bloom chroma preservation** — separate `L` from `(a, b)` so a
  Gaussian on `L` only blurs luma without bleaching the chroma.

For non-perceptual conversions (analog-TV chroma manipulation, NTSC-style
color shifts) use [YIQ](yiq.md) instead.

## Gamma note

Inputs are treated as **linear RGB**. Adobe hosts in 32-bit float modes
already deliver linear pixels; in 8-bit / 16-bit integer modes the values
are sRGB gamma-encoded. The math runs either way and the perceptual
benefit largely survives, but for a strict rotation in *display-linear*
space, decode sRGB → linear before calling and encode back afterwards.
