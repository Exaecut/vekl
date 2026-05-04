# Noise — Perlin

2D gradient noise (Perlin-style) with scalar and vector variants.

Source: [`noise/perlin.slang`](../../noise/perlin.slang)

---

## Public Functions

### `PerlinNoise`

```cpp
float PerlinNoise(float2 p, float2 seed)
```

2D gradient noise returning a scalar value.

| Parameter | Description |
|-----------|-------------|
| `p` | Sample position |
| `seed` | Seed for hash randomization (different seeds = different patterns) |
| **Returns** | Noise value approximately in `[-1, 1]` |

### `PerlinNoise2`

```cpp
float2 PerlinNoise2(float2 p, float2 seed)
```

2D gradient noise returning a `float2` vector. Each component uses a
different offset for independent noise fields.

---

## Internal Functions

| Function | Description |
|----------|-------------|
| `Hash2(p, seed)` | Hash mapping position + seed to pseudo-random `float2` |
| `Grad(p, seed)` | Gradient vector from hash, normalized to unit length |
| `Fade(t)` | Quintic fade curve: `6t⁵ - 15t⁴ + 10t³` |
| `ScalarNoise(p, seed)` | Core 2D gradient noise implementation |
