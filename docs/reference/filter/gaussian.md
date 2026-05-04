# Filter — Gaussian

Image filtering operations.

Source: [`filter/gaussian.slang`](../../filter/gaussian.slang)

---

## Functions

### `GaussianWeight1d`

```cpp
float GaussianWeight1d(int x, float sigma)
```

Compute a 1D Gaussian weight for offset `x` with standard deviation `sigma`.

### `Gaussian1d`

```cpp
float4 Gaussian1d(TextureView tex, float2 uv, float sigma, int radius, bool vertical)
```

Apply a separable 1D Gaussian blur in a single pass.

Uses a stride-2 optimization: adjacent texel pairs are combined into a
single bilinear sample at their weighted midpoint, halving texture reads
while maintaining accuracy.

#### Radius selection

A good default is `radius = int(sigma * 3.0)`, which captures ~99.7% of the
Gaussian energy. Clamp to a maximum (e.g. 128) for performance.

#### Sigma/radius guidelines

| Blur strength | `sigma` | `radius` (`int(sigma * 3)`) |
|---------------|---------|------|
| Subtle | 1–3 | 3–9 |
| Medium | 4–8 | 12–24 |
| Heavy | 10–20 | 30–60 |
| Extreme | 20+ | 60–128 |
