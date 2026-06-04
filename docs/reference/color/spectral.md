# Color - Spectral

Wavelength-band weights for spectral dispersion / prism effects. Maps a sweep
coordinate `s ∈ [0, 1]` to per-channel response that ramps Red → Green → Blue,
so accumulating source taps along a line with these weights integrates a
continuous prism smear instead of a 2-channel split.

Source: [`color/spectral.slang`](../../color/spectral.slang)

---

## How to use

Sweep `N` taps along the dispersion axis from `-offset` to `+offset` and weight
each tap by `SpectralWeights(s)`. Normalize each output channel by its own
summed band weight so a flat field reproduces unchanged (the three bands sum to
1 at every `s`, so there is no tint and no energy loss). Red lands at `-offset`,
blue at `+offset`, green in the middle.

For alpha-correct edges, accumulate premultiplied (`rgb * a`) per channel and
divide by the per-channel weight sum.

---

## Functions

### `SpectralWeights`

```cpp
float3 SpectralWeights(float s)
```

Returns `(wR, wG, wB)` triangular bands at `s ∈ [0, 1]`: `wR` peaks at `s = 0`,
`wG` at `s = 0.5`, `wB` at `s = 1`. `wR + wG + wB == 1` for all `s`.

```cpp
float3 num = float3(0), den = float3(0);
for (uint i = 0; i < n; ++i) {
    float s = float(i) / float(n - 1);
    float3 w = SpectralWeights(s);
    float3 c = src.SampleLinear(uv0 + offset * (s * 2.0 - 1.0)).rgb;
    num += c * w; den += w;
}
float3 prism = num / max(den, float3(1e-6));
```
