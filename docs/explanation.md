# VEKL Architecture

This document explains why VEKL exists, how it's structured, and the design
decisions behind it.

---

## Why VEKL

GPU video effects share the same building blocks across every project: pixel
I/O, format conversion, coordinate transforms, noise, and filtering. Writing
these from scratch for each effect leads to:

- **Behavioral drift** — each effect's pixel I/O code subtly diverges
- **Duplication** — every shader reimplements bilinear sampling, UV math, etc.
- **Format bugs** — BGRA/VUYA conversion mistakes are easy to make and hard to catch
- **Maintenance cost** — fixing a bug in one shader doesn't fix it in others

VEKL centralizes these primitives into a single, tested module. Write once,
import everywhere.

---

## Why Slang

VEKL is written in [Slang](https://shader-slang.com/), a shader language and
compiler that extends HLSL with modern features:

| Feature | Benefit for VEKL |
|---------|------------------|
| **Module system** | `import vekl;` gives clean namespacing — no includes or macros |
| **Cross-compilation** | Single source compiles to Metal, CUDA, SPIR-V, DXIL |
| **Type system** | Structs, enums, generics — more expressiveness than raw HLSL |
| **Backward compatibility** | Valid Slang is valid HLSL — low learning curve |

### Why not C++ macros / includes?

The previous VEKL design used C++ headers with `#ifdef` dispatch to CUDA,
Metal, and CPU backends. This had several problems:

- Macro expansion is opaque — hard to debug, hard to read
- No module isolation — every include pollutes the global namespace
- CPU fallback required a separate compilation path with manual dispatch wrappers
- The three-backend maintenance burden was the problem VEKL was meant to solve

Slang's module system and cross-compilation eliminate these issues: one source,
one import, all backends.

---

## Module structure

VEKL uses Slang's module system with a two-level organization:

```
vekl.slang             ← module vekl;         (includes aggregator files)
├─ texture.slang       ← implementing vekl;   (includes texture/ subfiles)
│  ├─ texture/descriptor.slang                (enums + TextureDesc + FrameParams)
│  ├─ texture/format.slang                    (pixel I/O, address helpers)
│  └─ texture/view.slang                      (TextureView / RWTextureView)
├─ sampling.slang      ← implementing vekl;   (includes sampling/ subfiles)
│  └─ sampling/coordinate.slang               (TexCoord, ScaleUV, RotateUV)
├─ math.slang          ← implementing vekl;   (includes math/ subfiles)
│  └─ math/mask.slang                         (DistanceMask)
├─ filter.slang        ← implementing vekl;   (includes filter/ subfiles)
│  └─ filter/gaussian.slang                   (Gaussian blur)
├─ noise.slang         ← implementing vekl;   (includes noise/ subfiles)
│  ├─ noise/perlin.slang                      (PerlinNoise, PerlinNoise2)
│  └─ noise/fbm.slang                         (FbmNoise)
└─ color.slang         ← implementing vekl;   (includes color/ subfiles)
   └─ color/blend/
      ├─ color/blend/add.slang                (BlendAdd)
      └─ color/blend/multiply.slang           (BlendMultiply)
```

### Two-level include pattern

Each concern has an **aggregator file** at the root (e.g. `texture.slang`)
that `__include`s its implementing files from the subdirectory. `vekl.slang`
then `__include`s only the aggregator files.

This pattern:
- Keeps `vekl.slang` concise — it lists concerns, not individual files
- Makes the public API surface visible at a glance from the root
- Allows subdirectories to grow without touching `vekl.slang`
- Follows Slang's recommended module organization

### Visibility

Functions and types marked `public` are accessible to importers. Internal
helpers (like `Hash2`, `Grad`, `Fade` in `noise/perlin.slang`) are private
by default (Slang's `internal` visibility).

### Adding new features

To add a new feature within an existing concern:

1. Create the file in the concern's subdirectory with `implementing vekl;`
2. Mark public functions with `public`
3. Add `__include` to the concern's aggregator file
4. Users automatically get the new functions via `import vekl;`

To add a new concern:

1. Create the subdirectory with implementing files
2. Create an aggregator file at the root
3. Add `__include` of the aggregator to `vekl.slang`

---

## Kernel contract

Every VEKL kernel follows a fixed parameter contract that host code relies on:

```
Slot 0: StructuredBuffer<uint> outgoing      — primary source
Slot 1: StructuredBuffer<uint> incoming       — secondary source
Slot 2: RWStructuredBuffer<uint> dst          — destination
Slot 3: ConstantBuffer<FrameParams> frame     — frame metadata (mandatory)
Slot 4: ConstantBuffer<CustomParams> params   — effect parameters
```

This convention enables:

- **Host-side automation** — PrGPU's build system parses the kernel signature
  to generate CPU dispatch wrappers, GPU pipeline setup, and benchmark harnesses
- **Consistent buffer routing** — multi-pass effects chain intermediate buffers
  through the same slots
- **FrameParams guarantee** — width, height, pitch, pixel format, and time are
  always available without per-effect plumbing

### Thread group size

All VEKL kernels use `[numthreads(16, 16, 1)]` (256 threads per group). This:

- Matches CUDA's warp size (32) with good occupancy (8 warps per block)
- Is a multiple of Metal's wavefront size (32 or 64)
- Provides enough threads per group for effective shared memory usage
- Creates a natural 16×16 pixel tile per group

---

## Pixel I/O pipeline

VEKL decouples pixel **storage** (bit depth) from pixel **layout** (channel
order / color space):

```
Raw buffer → LoadPixelRaw (decode storage) → ToRGBA (convert layout) → float4 RGBA
float4 RGBA → FromRGBA (convert layout) → StorePixelRaw (encode storage) → Raw buffer
```

This separation means:

- Adding a new pixel format only requires changes in `texture/format.slang`
- Shader code always works in RGBA `float4` — no format conditionals
- `TextureView` encapsulates the full pipeline, so most code never touches
  `LoadPixelRaw` or `ToRGBA` directly

### Supported formats

| Storage | Layout | Use case |
|---------|--------|----------|
| `Unorm8x4` | `Rgba` | Standard 8bpc |
| `Unorm8x4` | `Bgra` | Windows display |
| `Unorm16x4` | `Rgba` | Deep color |
| `Float32x4` | `Rgba` | HDR / float processing |
| `Unorm8x4` | `Vuya` | SD video (BT.601) |
| `Unorm8x4` | `Vuya709` | HD video (BT.709) |

---

## Gaussian blur optimization

`Gaussian1d` uses a **stride-2 bilinear optimization** instead of sampling
every texel individually:

1. Pair adjacent texel offsets `(2i-1, 2i)` into a single bilinear sample
2. Compute the weighted midpoint between the pair
3. Sample at the midpoint — the hardware bilinear filter blends both texels
4. Weight the result by the combined weight `w_a + w_b`

This halves the number of texture reads while maintaining accuracy because
bilinear hardware filtering is free on GPU texture units.

For a full 2D Gaussian blur, apply `Gaussian1d` twice (horizontal, then
vertical) — the classic separable filter approach that reduces O(n²) to O(2n).

---

## Relationship to PrGPU

VEKL is the shader library. [PrGPU](../../prgpu/) is the Rust host-side
framework that:

- Compiles `.slang` shaders at build time via Slang
- Generates CPU dispatch wrappers for software rendering
- Manages GPU pipelines (Metal/CUDA) for hardware rendering
- Provides buffer pools for multi-pass effects
- Handles parameter extraction from After Effects / Premiere Pro

---

## Design principles

1. **Zero abstraction tax** — `TextureView` and helper functions compile to
   inline code with no runtime overhead
2. **Format agnostic** — shaders never reason about pixel format; `TextureView`
   handles conversion transparently
3. **Single source** — one `.slang` file compiles to all GPU backends
4. **Composable** — import what you need, ignore what you don't
5. **POD parameters** — all parameter structs are plain data for stable ABIs
   across the host/device boundary
6. **Templated utilities** — prefer generic functions over per-type overloads
   to reduce code duplication and maintenance surface
