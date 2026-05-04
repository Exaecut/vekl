# VEKL (Video Effects Kernel Library)

VEKL is an open-source, modular compute kernel library written in
[Slang](https://shader-slang.com/) for developing high-performance video
effects and transitions. It provides reusable primitives for pixel I/O,
texture sampling, coordinate transforms, noise generation, filtering, and
color blending — all compiled through Slang's cross-platform compiler to
CUDA, Metal, and other GPU backends.

> License: Apache-2.0

## Why this exists

GPU video effects share the same building blocks across every project:

- Reading and writing pixels in multiple formats (8-bit, 16-bit, 32-bit float)
- Converting between pixel layouts (RGBA, BGRA, VUYA 601/709)
- Sampling textures with nearest, bilinear, or repeat/mirror addressing
- Computing UV coordinates, rotations, and aspect-correct transforms
- Generating noise patterns for organic effects
- Applying separable Gaussian blur
- Blending colors with compositing modes

Rewriting these from scratch for each effect is error-prone and leads to
behavioral drift. VEKL solves this by providing:

1. **Single source of truth** — Write once in Slang, compile to any backend Slang supports
2. **Deterministic parity** — Identical algorithm, identical output across all backends
3. **Modular design** — Import only what you need via Slang's module system
4. **Format-agnostic pixel I/O** — Automatic handling of pixel storage, layout, and address modes

## Quick start

Below is a minimal vignette kernel that reads pixels, applies a distance-based
mask, and writes the result:

```cpp
import vekl;

struct VignetteParams
{
    float tintR; float tintG; float tintB; float tintA;
    float anchorX; float anchorY;
    float scaleX; float scaleY;
    float darkenStrength; float darkenMin; float darkenMax;
};

[shader("compute")]
[numthreads(16, 16, 1)]
void vignette(
    uint3 threadId : SV_DispatchThreadID,
    StructuredBuffer<uint> outgoing,
    StructuredBuffer<uint> incoming,
    RWStructuredBuffer<uint> dst,
    ConstantBuffer<FrameParams> frame,
    ConstantBuffer<VignetteParams> params
)
{
    RWTextureView output = RWTextureView(dst, frame.dstDesc);
    if (!output.Contains(threadId.xy))
        return;

    TextureView src = TextureView(incoming, frame.inDesc);
    uint2 size = src.Size();
    float2 uv = TexCoord(threadId.xy, size);

    float2 scaledUV = ScaleUV(uv, float2(1.0 / params.scaleX, 1.0 / params.scaleY));
    float2 anchorUV = ScaleUV(float2(params.anchorX, params.anchorY),
                              float2(1.0 / params.scaleX, 1.0 / params.scaleY));

    float mask = DistanceMask(scaledUV, anchorUV, params.darkenMin, params.darkenMax);

    float4 original = src.Load(threadId.xy);
    float4 tint = float4(params.tintR, params.tintG, params.tintB, 1.0);
    float strength = mask * params.darkenStrength * params.tintA;

    float4 result = float4(
        lerp(original.x, original.x * tint.x, strength),
        lerp(original.y, original.y * tint.y, strength),
        lerp(original.z, original.z * tint.z, strength),
        original.w
    );
    output.Store(threadId.xy, result);
}
```

## Directory

```
vekl/
├── vekl.slang               # Module declaration — includes aggregator files
│
├── texture.slang             # Aggregator — includes texture/* files
├── texture/
│   ├── descriptor.slang      # PixelStorage, PixelLayout, AddressMode, TextureDesc, FrameParams
│   ├── format.slang          # ToRGBA, FromRGBA, LoadPixel, StorePixel, address helpers
│   └── view.slang            # TextureView, RWTextureView with sample methods
│
├── sampling.slang            # Aggregator — includes sampling/* files
├── sampling/
│   └── coordinate.slang      # TexCoord, PixelCoord, ScaleUV, RotateUV, UniformAspectRatio
│
├── math.slang                # Aggregator — includes math/* files
├── math/
│   └── mask.slang            # DistanceMask
│
├── filter.slang              # Aggregator — includes filter/* files
├── filter/
│   └── gaussian.slang        # GaussianWeight1d, Gaussian1d
│
├── noise.slang               # Aggregator — includes noise/* files
├── noise/
│   ├── perlin.slang          # PerlinNoise, PerlinNoise2
│   └── fbm.slang             # FbmNoise
│
├── color.slang               # Aggregator — includes color/* files
├── color/
│   └── blend/
│       ├── add.slang          # BlendAdd (generic)
│       └── multiply.slang     # BlendMultiply (generic)
│
├── docs/
│   ├── reference/            # API reference (nested by concern)
│   ├── tutorials/            # Step-by-step learning guides
│   ├── how-to-guides.md      # Task-oriented recipes
│   └── explanation.md        # Architecture and design decisions
│
├── CONTRIBUTING.md           # Development guidelines
├── LICENSE                   # Apache-2.0
└── README.md                 # This file
```

## Add as a submodule

```bash
git submodule add https://github.com/exaecut/vekl.git third_party/vekl
```

Then import in your Slang shader:

```cpp
import vekl;
```

## Add as a subtree

```bash
# Add VEKL as a remote
git remote add vekl-remote https://github.com/exaecut/vekl.git

# Add the subtree (first time)
git subtree add --prefix=third_party/vekl vekl-remote main --squash

# Future updates
git subtree pull --prefix=third_party/vekl vekl-remote main --squash
```

## Compilation

VEKL is a Slang module. Compile it through the Slang compiler targeting your
preferred backend:

```bash
# Compile to SPIR-V (Vulkan)
slangc my_effect.slang -target spirv -entry my_kernel -stage compute

# Compile to Metal (MSL)
slangc my_effect.slang -target metal -entry my_kernel -stage compute

# Compile to CUDA
slangc my_effect.slang -target cuda -entry my_kernel -stage compute

# Compile to DXIL (DirectX)
slangc my_effect.slang -target dxil -entry my_kernel -stage compute
```

Slang resolves `import vekl;` by searching the include paths. Add the VEKL
directory to your include search path:

```bash
slangc -I third_party/vekl my_effect.slang ...
```

## Key concepts

### Kernel signature pattern

Every VEKL-compatible compute kernel follows this parameter layout:

```cpp
[shader("compute")]
[numthreads(16, 16, 1)]
void my_kernel(
    uint3 threadId : SV_DispatchThreadID,
    StructuredBuffer<uint> outgoing,      // slot 0: primary source buffer
    StructuredBuffer<uint> incoming,      // slot 1: secondary source (transitions/multi-pass)
    RWStructuredBuffer<uint> dst,         // slot 2: destination buffer
    ConstantBuffer<FrameParams> frame,    // slot 3: frame metadata (mandatory)
    ConstantBuffer<MyParams> params       // slot 4: effect parameters
)
```

The `FrameParams` constant buffer is mandatory — it provides frame dimensions,
pitch, pixel format, time, and progress.

### Texture views

Use `TextureView` for read-only access and `RWTextureView` for read-write
access. Both encapsulate buffer pointer + descriptor:

```cpp
TextureView src = TextureView(outgoing, frame.outDesc);
RWTextureView dst = RWTextureView(dst, frame.dstDesc);
```

### Bounds checking

Always guard against out-of-bounds access at kernel entry:

```cpp
if (!output.Contains(threadId.xy))
    return;
```

### Pixel formats

VEKL handles pixel storage (`Unorm8x4`, `Unorm16x4`, `Float32x4`) and layout
(`Rgba`, `Bgra`, `Vuya`, `Vuya709`) conversion transparently through
`TextureView.Load()` and `RWTextureView.Store()`. All shader math operates in
RGBA float4 space.

### Blend modes

VEKL provides generic blend functions that work with any vector type:

```cpp
float4 result = BlendAdd(baseColor, glowColor);
float3 darkened = BlendMultiply(baseColor.rgb, shadowColor.rgb);
```

## Documentation

- **[Tutorials](docs/tutorials/)** — Learn VEKL step-by-step
  - [Hello World: Chromatic Aberration](docs/tutorials/00-hello-world-chromatic-aberration.md)
- **[API Reference](docs/reference/)** — Complete type and function catalog
  - [texture](docs/reference/texture/) — Descriptor, format, views
  - [sampling](docs/reference/sampling/) — UV coordinate utilities
  - [math](docs/reference/math/) — Distance masks
  - [noise](docs/reference/noise/) — Perlin noise and FBM
  - [filter](docs/reference/filter/) — Gaussian blur
  - [color](docs/reference/color/) — Blend modes
- **[How-to Guides](docs/how-to-guides.md)** — Recipes for common tasks
- **[Architecture](docs/explanation.md)** — Design decisions and module structure
- **[Contributing](CONTRIBUTING.md)** — Development guidelines

## License

Apache-2.0. See `LICENSE`.
