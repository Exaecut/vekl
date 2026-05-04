# Math — Mask

Mathematical utility functions for effects.

Source: [`math/mask.slang`](../../math/mask.slang)

---

## Functions

### `DistanceMask`

```cpp
float DistanceMask(float2 uv, float2 center, float mn, float mx)
```

Radial distance mask centered at `center`. Returns `0.0` near the center
and `1.0` far away, with a smooth transition between `mn` and `mx`.

Distance is normalized by `sqrt(2) * 0.5` so the diagonal maps to `1.0`.

#### Parameter tuning

| `mn` (inner) | `mx` (outer) | Effect |
|---------------|---------------|--------|
| 0.0 | 1.0 | Gradual falloff across entire image |
| 0.5 | 0.8 | Tight central region, quick falloff |
| 0.0 | 0.3 | Most of image masked, small clear center |
