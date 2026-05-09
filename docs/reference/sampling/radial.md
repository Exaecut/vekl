# Sampling — Radial / Angular Sweeps

Stateless sampling primitives for multi-tap effects that sweep around a
center point: radial blur, motion blur, lens streaks, glow trails, etc.

Source: [`sampling/radial.slang`](../../sampling/radial.slang)

---

## Why this module exists

A "sweep" is a multi-tap blur that moves the sampling UV along a curve
(an arc for spin, a radial line for zoom). Three failure modes show up
when you implement one ad-hoc:

1. **Sample placement** — uniform `t` produces banding; clustering near
   the anchor pixel hides it. `SampleParam` + `Redistribute` solve this.
2. **Trail-length aliasing** — a long trail with too few taps skips
   source pixels and leaks high-frequency detail through. `PickSweepLod`
   picks the matching mip level (paired with `SampleLinearTrilinear` for
   smooth transitions).
3. **Chroma washout / alpha-edge darkening** — naive `sum/N` averaging
   desaturates and dims edges. `SweepAccumulator` (in
   [`color/accumulate.md`](../color/accumulate.md)) handles both.

A complete sweep loop combines all three:

```cpp
import vekl;

uint  maxLod = max(frame.outDesc.mipLevelCount, 1u) - 1u;
float lodF   = PickSweepLod(trailPx, opts.sampleCount, maxLod);

SweepAccumulator acc = SweepAccumulator.Create();
for (uint i = 0u; i < opts.sampleCount; ++i)
{
    SweepSample s = AngularSample(uv, origin, angle, i, opts);
    acc.Add(src.SampleLinearTrilinear(saturate(s.uv), lodF), s.weight);
}
float4 result = acc.Resolve(/* restoreSaturation */ 1.0);
```

---

## Types

### `SampleDistribution`

Sample-positioning curve along the trail. Denser sampling near the
anchor (`t = 0`) hides banding around the pixel of interest.

| Variant | Value | Description |
|---------|-------|-------------|
| `Linear` | `0` | Uniform spacing. Cheapest, simplest, and best when the trail is short. |
| `Exponential` | `1` | `\|t'\| = \|t\|²` — strong cluster near the anchor. |
| `Gaussian` | `2` | `\|t'\| = 1 - cos(π/2 · \|t\|)` — smoother shoulder than Exponential, single cosine. |

### `SweepOptions`

Per-instance tuning shared by `AngularSample` and `RadialSample`.

| Field | Type | Description |
|-------|------|-------------|
| `sampleCount` | `uint` | Tap count, `>= 2`. Drives the kernel loop bound (must be uniform across the dispatch). |
| `direction` | `float` | Trail bias in `[-1, +1]`. `0` is symmetric; `+1` reaches forward only; `-1` reaches backward only. |
| `distribution` | `SampleDistribution` | Sample-positioning curve. |
| `jitter` | `float` | `[0..1]` random `t` perturbation to break low-sample-count banding. Deterministic per index. |
| `anisotropy` | `float2` | Per-axis scale of the offset around `center`. `(1, 1)` keeps the sweep circular; combine with `PixelAspectAnisotropy` for non-square sources. |
| `weightExponent` | `float` | `weight = NaturalWeight(t)^weightExponent`. `1.0` = natural; `> 1` emphasizes the anchor; `0` = uniform. |

`SweepOptions.Default(sampleCount)` returns a deterministic, linear,
symmetric, isotropic configuration — fine for most spin / zoom effects.

### `SweepSample`

Result of one sample query.

| Field | Type | Description |
|-------|------|-------------|
| `uv` | `float2` | UV to read from the source. May fall outside `[0,1]` — wrap with `saturate` or your own address mode. |
| `weight` | `float` | Pre-normalized weight to multiply this sample by. Caller divides by the running weight sum (or hands it to `SweepAccumulator.Add`). |

---

## Public functions

### `AngularSample`

```cpp
SweepSample AngularSample(
    float2 uv,
    float2 center,
    float  angle,        // full sweep arc in radians
    uint   index,        // 0 .. opts.sampleCount-1
    SweepOptions opts);
```

Rotates the offset `(uv - center)` around `center` across the arc
`[-angle/2, +angle/2]` (modulated by `direction`). Produces a "spin"
blur. Anchor `t = 0` always sits at `uv` itself.

### `RadialSample`

```cpp
SweepSample RadialSample(
    float2 uv,
    float2 center,
    float  distance,     // signed peak displacement, fraction of offset
    uint   index,
    SweepOptions opts);
```

Scales the offset `(uv - center)` along the line `uv` ↔ `center` by
`1 + t·distance`. Produces a "zoom" blur. Negative `distance` reverses
the trail direction.

### `PixelAspectAnisotropy`

```cpp
float2 PixelAspectAnisotropy(uint2 sizePx);
```

Returns an anisotropy vector that compensates a non-square texture so
the sweep traces a perceptual circle in screen space. Multiply
`opts.anisotropy` by the result if your source isn't 1:1.

### `PickSweepLod`

```cpp
float PickSweepLod(float trailPx, uint sampleCount, uint maxLod);
float PickSweepLod(float trailPx, uint sampleCount, uint maxLod, float oversample);
```

Returns the lowest fractional mip level the kernel can sample without
aliasing, given the per-pixel trail length and the available sample
budget. Pair with
[`TextureView.SampleLinearTrilinear(uv, lodF)`](../texture/view.md#sampleLinearTrilinear)
to get a hard-cut-free transition between mip levels.

Formula:

```
lodF = max(0, log2(oversample · trailPx / sampleCount))
```

clamped to `[0, maxLod]`. The 3-arg overload is `oversample = 1.0`
(Nyquist).

#### `oversample` — perf vs detail trade-off

The chosen lod sits at `log2(oversample)` levels above Nyquist, which
both reduces the per-tap memory cost (deeper mip = smaller cache
footprint) and the per-pixel tap count (`local_n` shrinks proportionally
because `trail_at_lod = sampleCount / oversample`).

| `oversample` | Behaviour | When to use |
|--------------|-----------|-------------|
| `1.0` | Nyquist — 1 tap per source pixel of trail. Sharpest, most expensive on long trails. | Image-quality-first paths, offline renders. |
| `4.0` | 4× oversampled — lod ramps up 2 levels earlier; ~25 % of Nyquist's tap count. Recommended default for real-time sweep effects. | Real-time radial / motion / lens-streak blur. |
| `0.5` | Sub-Nyquist — lod stays sharp at the cost of mild aliasing. | Known-very-large sample budget where detail dominates. |

For a typical sweep at 1080p with `sampleCount = 200` and a 200 px trail,
`oversample = 1.0` runs at lod 0 with ~200 taps per pixel; `oversample = 4.0`
runs at lod ~2 with ~50 taps per pixel — a perceptually similar result
(thanks to trilinear blending) at a fraction of the cost.

#### Inputs

| Parameter | Description |
|-----------|-------------|
| `trailPx` | Worst-case trail length covered by the sweep at this output pixel, in source pixels (lod-0 units). For an angular sweep, `trailPx = 2 · length(d_px) · \|sin(angle/2)\|`. For a radial sweep, `trailPx = length(d_px) · \|distance\|`. |
| `sampleCount` | Number of taps the kernel will loop over for this pixel. Pass `opts.sampleCount` (the frame-wide budget). Values `<= 1` short-circuit to `0`. |
| `maxLod` | Highest available lod (`max(desc.mipLevelCount, 1u) - 1u`). `0` means "no mip chain" — always `0`. |
| `oversample` | Taps per source pixel of trail at the chosen lod. Defaults to `1.0` (Nyquist) in the 3-arg overload. |

#### Recipe (radial / spin / zoom blur)

```cpp
import vekl;

// 1. Worst-case trail length at this pixel, in source pixels.
float2 d_px    = (uv - origin) * float2(size) / max(opts.anisotropy, 1e-6);
float  trailPx = 2.0 * length(d_px) * abs(sin(angle * 0.5));   // angular
//      trailPx = length(d_px) * abs(distance);                 // radial

// 2. Pick the matching lod. 4× oversample is the recommended real-time
//    default — much cheaper than Nyquist with negligible perceptual loss.
uint  maxLod = max(frame.outDesc.mipLevelCount, 1u) - 1u;
float lodF   = PickSweepLod(trailPx, opts.sampleCount, maxLod, 4.0);

// 3. Sample with trilinear so two adjacent output pixels picking
//    slightly different lods cross-fade.
SweepAccumulator acc = SweepAccumulator.Create();
for (uint i = 0u; i < opts.sampleCount; ++i) {
    SweepSample s = AngularSample(uv, origin, angle, i, opts);
    acc.Add(src.SampleLinearTrilinear(saturate(s.uv), lodF), s.weight);
}
float4 result = acc.Resolve(1.0);
```

The host fills mip levels 1..N-1 via [`prgpu::kernels::mip::generate_mips`](../../../../prgpu/docs/mip_chain.md);
the shader never sees the chain layout — it just consumes `lodF`.

---

## Internal helpers

| Function | Description |
|----------|-------------|
| `Hash11(x)` | 1D deterministic hash, `[0..1]`. Drives jitter so two pixels with the same index see different perturbations. |
| `SampleParam(i, n, direction, jitter, salt)` | Maps `i` in `[0..n)` to a parametric `t` in `[-1, +1]`, biased by `direction`, optionally jittered. |
| `Redistribute(t, dist)` | Maps linear `t` through the chosen `SampleDistribution` curve, preserving sign. |
| `NaturalWeight(t, dist)` | Per-distribution baseline weight at `t`. The caller raises it to `weightExponent` and normalizes. |
