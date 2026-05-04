# Noise — FBM

Fractal Brownian motion — layered Perlin noise at increasing frequencies.

Source: [`noise/fbm.slang`](../../noise/fbm.slang)

---

## Public Functions

### `FbmNoise`

```cpp
float2 FbmNoise(float2 p, float2 seed, int octaves, float lacunarity, float gain)
```

Layered Perlin noise accumulating across multiple octaves.

| Parameter | Description |
|-----------|-------------|
| `p` | Sample position |
| `seed` | Seed for hash randomization |
| `octaves` | Number of noise layers (3–6 typical) |
| `lacunarity` | Frequency multiplier per octave (typically `2.0`) |
| `gain` | Amplitude multiplier per octave (typically `0.5`) |
| **Returns** | Accumulated noise vector |
