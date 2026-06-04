# Filter — Dual-Kawase

Pyramid blur whose effective radius grows with pyramid depth at a fixed
per-pass tap count, independent of the radius. For wide, soft blur it is
cheaper than a separable Gaussian (whose cost scales with radius).

Source: [`filter/kawase.slang`](../../filter/kawase.slang)

Reference: Marius Bjørge, "Bandwidth-Efficient Rendering", SIGGRAPH 2015.

---

## How to use

Run a downsample chain followed by an upsample chain over a mip pyramid. The
downsample writes each level from the level above; the upsample REPLACES each
level from the level below. After both chains, level 0 holds the blurred image.
Blur strength is set by how many levels the chains span — deeper = softer. This
differs from the Karis bloom pyramid (`karis.slang`), whose upsample
*accumulates* for energy-preserving glow.

Pair with `RWTextureView.Store(uv, value, lod)` / `TextureView.SampleLinear(uv, lod)`
exactly as the Karis down/up passes do; the host binds the pyramid as both
`outgoing` (read) and `dst` (write).

---

## Functions

### `KawaseDownsample`

```cpp
float4 KawaseDownsample(TextureView src, float2 uv, uint lod)
```

5-tap downsample: centre tap (×4) plus four diagonal corner taps at a
half-source-texel, normalised by 8. Bilinear oversampling folds each corner
into a 2×2 box, so the 5 fetches cover a 4×4 footprint. `lod` is the source
level; write the result to `lod + 1`.

### `KawaseUpsample`

```cpp
float4 KawaseUpsample(TextureView src, float2 uv, uint lod, float radius)
```

8-tap tent upsample: four edge taps (×1) and four corner taps (×2) on a
one-source-texel ring scaled by `radius`, normalised by 12. `lod` is the source
level (the smaller one); write the result to `lod - 1`. `radius` 1.0 is the
canonical kernel; larger softens, smaller tightens.

---

## Gaussian vs Dual-Kawase

| | Separable Gaussian (`Gaussian1d`) | Dual-Kawase |
|---|---|---|
| Cost vs radius | linear (O(radius)) | ~constant (radius set by depth) |
| Passes | 2 (H + V) | 2·(levels − 1) (down + up) |
| Taps / pass | radius-dependent | 5 (down) / 8 (up) |
| Best for | small, exact-radius blur | wide, soft blur (fringe, glow base) |
| Buffers | one scratch | a mip pyramid |
