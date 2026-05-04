# VEKL How-to Guides

Practical recipes for common tasks when writing VEKL effects. Each guide
solves a specific problem with copy-paste-ready code.

---

## Apply a Gaussian Blur

Use `Gaussian1d` from the `filter` concern. Gaussian blur is separable —
apply it in two passes (horizontal then vertical) for an O(n) instead of
O(n²) algorithm.

### Single-pass 1D blur

```cpp
import vekl;

struct BlurParams { float sigma; uint direction; };

[shader("compute")]
[numthreads(16, 16, 1)]
void blur_pass(
    uint3 threadId : SV_DispatchThreadID,
    StructuredBuffer<uint> outgoing,
    StructuredBuffer<uint> incoming,
    RWStructuredBuffer<uint> dst,
    ConstantBuffer<FrameParams> frame,
    ConstantBuffer<BlurParams> params
)
{
    RWTextureView output = RWTextureView(dst, frame.dstDesc);
    if (!output.Contains(threadId.xy))
        return;

    TextureView src = TextureView(outgoing, frame.outDesc);
    float2 uv = TexCoord(threadId.xy, output.Size());

    int radius = clamp(int(params.sigma * 3.0), 1, 128);
    bool vertical = (params.direction == 1);

    float4 result = Gaussian1d(src, uv, params.sigma, radius, vertical);
    output.Store(threadId.xy, result);
}
```

### Two-pass full Gaussian blur

For a full 2D blur, dispatch the kernel twice — once horizontal, once
vertical. The host code chains the passes using intermediate buffers:

**Pass 1 (horizontal):** `direction = 0`, source → intermediate buffer

**Pass 2 (vertical):** `direction = 1`, intermediate → destination

See the [vignette blur shader](../../vignette/shaders/blur.slang) for a
working example.

### Choosing sigma and radius

| Blur strength | `sigma` | `radius` (`int(sigma * 3)`) |
|---------------|---------|------|
| Subtle | 1–3 | 3–9 |
| Medium | 4–8 | 12–24 |
| Heavy | 10–20 | 30–60 |
| Extreme | 20+ | 60–128 |

Always clamp radius to a maximum (e.g. 128) to prevent excessive texture
reads.

---

## Add Noise to an Effect

Use the `PerlinNoise` or `FbmNoise` functions from the `noise` concern.

### Simple Perlin noise distortion

```cpp
TextureView src = TextureView(outgoing, frame.outDesc);
uint2 size = src.Size();
float2 uv = TexCoord(threadId.xy, size);

float2 noiseUV = uv * 10.0;
float2 seed = float2(0.0, 0.0);

float n = PerlinNoise(noiseUV, seed);
float2 offset = float2(n, PerlinNoise(noiseUV + float2(5.2, 1.3), seed)) * 0.02;

float4 color = src.SampleLinear(uv + offset);
```

### Fractal Brownian Motion (layered noise)

```cpp
float2 noiseUV = uv * 8.0;
float2 seed = float2(frame.time, 0.0);

float2 n = FbmNoise(noiseUV, seed, /*octaves=*/4, /*lacunarity=*/2.0, /*gain=*/0.5);
float2 offset = n * 0.03;

float4 color = src.SampleLinear(uv + offset);
```

---

## Blend Colors

Use the generic blend functions from the `color/blend` concern. They work
with `float3` and `float4` without overloads.

### Additive glow

```cpp
float4 base = src.Load(threadId.xy);
float4 glow = float4(0.5, 0.3, 0.1, 0.0);
float4 result = BlendAdd(base, glow);
```

### Multiply vignette

```cpp
float4 base = src.Load(threadId.xy);
float4 shadow = float4(vignetteMask, vignetteMask, vignetteMask, 1.0);
float4 result = BlendMultiply(base, shadow);
```

---

## Transform UV Coordinates

The `sampling` concern provides utilities for common UV transformations.

### Scale (zoom in/out)

```cpp
float2 uv = TexCoord(threadId.xy, size);
float2 scaled = ScaleUV(uv, 0.5);
float4 color = src.SampleLinear(scaled);
```

### Rotate

```cpp
float2 uv = TexCoord(threadId.xy, size);
float aspect = float(size.x) / float(size.y);
float2 rotated = RotateUV(uv, 0.785, aspect);
float4 color = src.SampleLinear(rotated);
```

### Correct for aspect ratio

```cpp
float2 uv = TexCoord(threadId.xy, size);
float2 corrected = UniformAspectRatio(uv, size);
float mask = DistanceMask(corrected, float2(0.5, 0.5), 0.2, 0.8);
```

---

## Create a Distance-based Mask

Use `DistanceMask` for radial falloff patterns — vignettes, spotlights,
radial wipes, etc.

```cpp
float2 uv = TexCoord(threadId.xy, size);

float vignette = DistanceMask(uv, float2(0.5, 0.5), 0.3, 0.8);
float spotlight = DistanceMask(uv, float2(0.3, 0.7), 0.1, 0.5);

float4 original = src.Load(threadId.xy);
float4 darkened = original * (1.0 - vignette);
output.Store(threadId.xy, darkened);
```

---

## Read and Write Pixels Directly

For per-pixel effects that don't need UV sampling (color adjustments,
channel swaps, threshold operations), use `Load` and `Store` directly.

### Simple pixel copy

```cpp
TextureView src = TextureView(outgoing, frame.outDesc);
RWTextureView dst = RWTextureView(dst, frame.dstDesc);

if (!dst.Contains(threadId.xy))
    return;

float4 color = src.Load(threadId.xy);
dst.Store(threadId.xy, color);
```

### Safe pixel access with address modes

When reading pixels at coordinates that may be out of bounds (e.g., in
convolution kernels), use `LoadSafe`:

```cpp
float4 color = src.LoadSafe(threadId.xy + int2(-1, 0));
```

---

## Use Time for Animation

`FrameParams.time` provides the current time in seconds, automatically
populated from the host application.

```cpp
float2 seed = float2(frame.time * 0.5, 0.0);
float n = PerlinNoise(uv * 10.0, seed);

float pulse = sin(frame.time * 3.0) * 0.5 + 0.5;

float2 blendedUV = lerp(uv, distortedUV, frame.progress);
```

---

## Handle Multiple Pixel Formats

VEKL handles format conversion automatically through `TextureView`. Your
shader code works in RGBA `float4` space regardless of the underlying pixel
format.

```cpp
TextureView src = TextureView(outgoing, frame.outDesc);
RWTextureView dst = RWTextureView(dst, frame.dstDesc);

float4 color = src.Load(threadId.xy);   // Always returns float4 in RGBA
// ... process color ...
dst.Store(threadId.xy, color);           // Converts back to native format
```

### Supported formats

| `PixelStorage` | Bits/pixel | Description |
|----------------|-----------|-------------|
| `Unorm8x4` | 32 | 8-bit per channel, standard 8bpc |
| `Unorm16x4` | 64 | 16-bit per channel, deep color |
| `Float32x4` | 128 | Full float per channel, HDR |

| `PixelLayout` | Description |
|---------------|-------------|
| `Rgba` | Standard RGBA order |
| `Bgra` | Swapped blue/red (common on Windows) |
| `Vuya` | YCbCr BT.601 (video pipelines) |
| `Vuya709` | YCbCr BT.709 (HD video) |
