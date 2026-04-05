# VEKL (Video Effects Kernel Language)

VEKL is an open-source, cross-platform compute kernel foundation designed for
developing high-performance video effects and transitions for Adobe plugins and
similar pipelines. It abstracts the differences between CUDA, Metal, and CPU
backend execution into a single, unified, C++/Metal-like syntax.

> License: Apache-2.0

## Why this exists

GPU video processing requires writing shaders that run across disparate platforms:
- **NVIDIA** uses CUDA
- **Apple** uses Metal
- **CPU fallback** needs plain C++

Maintaining three separate implementations for every effect is error-prone, leads to
behavioral drift, and multiplies maintenance cost. VEKL solves this by providing:

1. **Single source of truth** — Write once in VEKL, compile to CUDA, Metal, and CPU
2. **Deterministic parity** — Identical algorithm, identical output across all backends
3. **Zero abstraction tax** — VEKL macros compile to native backend constructs; no runtime overhead
4. **NVRTC-ready** — CUDA backend designed for runtime compilation, no static CUDA dependency
5. **SIMD-friendly CPU** — CPU path generates vectorizable tight loops for software fallback

## TLDR features

- **Unified kernel syntax** — Metal-inspired DSL compiles to CUDA, Metal, and CPU
- **Zero-cost abstractions** — Macros expand to native constructs, no virtual calls
- **NVRTC compatible** — Runtime CUDA compilation without NVCC dependency
- **Auto-vectorizing CPU** — Sequential fallback that compilers can optimize
- **POD-first design** — All parameters are Plain Old Data for stable ABIs
- **Half precision support** — Toggle `USE_HALF_PRECISION` for memory bandwidth savings
- **Math + Image utilities** — Included helpers for coordinates, transforms, tonemapping

## Supported Backends

1. **CUDA** (NVRTC Runtime Compilation)
2. **Metal** (MSL)
3. **CPU** (C++17 Fallback)

---

## Directory

```dir
vekl/
├─ common.h              # Aggregates vekl.h + math + image utilities
├─ vekl.h                # Backend switch (includes correct shim)
├─ vekl_cuda.h           # CUDA shims (NVRTC compatible)
├─ vekl_metal.h          # Metal shims (native MSL mapping)
├─ vekl_cpu.h            # CPU shims (sequential dispatch)
├─ types.h               # pixel_format, FrameParams
├─ LICENSE               # Apache-2.0
├─ README.md             # This file
├─ filters/
│  └─ blur.h             # (Planned) Gaussian, radial, directional blur
├─ image/
│  ├─ 2d.h               # image_2d helpers
│  ├─ coords.h           # UV and coordinate utilities
│  └─ tonemapping.h      # sRGB, ACES, exposure ops
├─ maths/
│  ├─ easing.h           # Easing functions
│  ├─ transform.h        # Matrix and transform ops
│  └─ trigonometry.h     # Trig helpers
└─ sdf/
   ├─ operators.h        # SDF boolean ops
   ├─ sdf.h              # SDF primitives
   └─ shapes.h           # Shape distance functions
```

---

## Add as a submodule

```bash
git submodule add https://github.com/exaecut/vekl.git third_party/vekl
```

Then include headers from `third_party/vekl`

## Add as a subtree

If you prefer subtrees over submodules (e.g., for monorepo integration without
nested `.git` directories):

```bash
# Add VEKL as a remote
git remote add vekl-remote https://github.com/exaecut/vekl.git

# Add the subtree (first time)
git subtree add --prefix=third_party/vekl vekl-remote main --squash

# Future updates
git subtree pull --prefix=third_party/vekl vekl-remote main --squash
```

---

## Quick start example

Below is a minimal tint kernel that applies a color tint with adjustable intensity:

```cpp
// file: tint.vekl
#include "vekl/vekl.h"

struct TintParams {
    float3 tint_color;
    float intensity;
};

kernel void apply_tint(
    param_ro(pixel_format, src, 0),
    param_wo(pixel_format, dst, 1),
    param_cbuf(FrameParams, img, 2),
    param_cbuf(TintParams, tint, 3)
) {
    uint2 gid = dispatch_id();

    if (gid.x >= img.width || gid.y >= img.height)
        return;

    uint idx = gid.y * img.out_pitch + gid.x;

    // Read source pixel
    float4 color = float4(src[idx].x, src[idx].y,
                          src[idx].z, src[idx].w);

    // Apply tint by mixing original with tinted version
    float3 tinted = color.xyz * tint.tint_color;
    float3 result = mix(color.xyz, tinted, clamp(tint.intensity, 0.0f, 1.0f));

    // Write output
    dst[idx] = pixel_format(result.x, result.y, result.z, color.w);
}
```

---

## Compile VEKL code

VEKL files (`.vekl`) are C++ source files with backend-specific includes. The
compilation strategy differs per target:

### CUDA Backend (NVRTC)

```bash
# Compile to PTX at runtime using NVRTC
# Example using prgpu's NVRTC wrapper:

# 1. Read .vekl source
# 2. Pass to nvrtcCreateProgram with:
#    - --device-as-default-execution-space
#    - --gpu-architecture=sm_XX (target architecture)
# 3. Compile to PTX
# 4. Load with cuModuleLoadData
# 5. Get kernel with cuModuleGetFunction
```

The CUDA backend uses NVRTC for runtime compilation. No static CUDA toolkit
dependency required. VEKL headers are designed for NVRTC compatibility.

### Metal Backend

```bash
# Compile .vekl -> .metal -> .metallib

# Step 1: Rename/preprocess (VEKL is valid Metal with vekl_metal.h)
xcrun -sdk macosx metal -c tint.vekl -o tint.air

# Step 2: Link to metallib
xcrun -sdk macosx metallib tint.air -o tint.metallib

# Load in host code:
# id<MTLDevice> device = MTLCreateSystemDefaultDevice();
# id<MTLLibrary> library = [device newLibraryWithFile:@"tint.metallib" error:nil];
# id<MTLFunction> kernel = [library newFunctionWithName:@"apply_tint"];
```

### CPU Backend

The CPU backend requires special handling — `.vekl` files must be compiled as
separate object files and statically linked with the host binary.

```bash
# Step 1: Compile .vekl as C++ object
g++ -std=c++17 -c tint.vekl -o tint.o -I/path/to/vekl

# Step 2: Link object with host binary
g++ host_main.cpp tint.o -o host_binary
```

**Host integration pattern:**

The CPU kernel is compiled as a `static inline` function. The build system must
generate an `extern "C"` dispatch wrapper that loops over pixels:

```cpp
// Generated by build system if using PrGPU (e.g., build.rs)
#include "vekl/vekl.h"
#include "tint.vekl"  // brings in the kernel

extern "C" void dispatch_tint(
    const pixel_format* src,
    pixel_format* dst,
    const FrameParams* img,
    const TintParams* tint,
    uint width, uint height
) {
    // Set CPU dispatch globals
    __cpu_dispatch_w = width;
    __cpu_dispatch_h = height;
    
    for (uint y = 0; y < height; ++y) {
        for (uint x = 0; x < width; ++x) {
            __cpu_gid_x = x;
            __cpu_gid_y = y;
            apply_tint(src, dst, img, tint);
        }
    }
}
```

The host binary calls `dispatch_tint` which internally invokes the VEKL kernel
for each pixel. After inlining, modern compilers auto-vectorize the inner loop.

---

## VEKL quick reference

### Kernel declaration

```cpp
kernel void my_kernel(
    param_ro(pixel_format, input, 0),    // read-only buffer
    param_wo(pixel_format, output, 1),   // write-only buffer
    param_cbuf(MyParams, params, 2)     // constant buffer (POD struct)
)
```

### Parameter qualifiers

| Macro | Expands to (CUDA) | Expands to (Metal) | Expands to (CPU) |
|-------|-------------------|-------------------|------------------|
| `param_ro(T, n, s)` | `const T* __restrict__ n` | `device const T* n [[buffer(s)]]` | `const T* __restrict n` |
| `param_wo(T, n, s)` | `T* __restrict__ n` | `device T* n [[buffer(s)]]` | `T* __restrict n` |
| `param_cbuf(T, n, s)` | `const T n` | `constant T& n [[buffer(s)]]` | `const T n` |

### Thread dispatch

```cpp
uint2 gid = dispatch_id();    // Current thread position (x, y)
uint2 size = dispatch_size(); // Total grid dimensions
```

### Memory qualifiers

| Keyword | CUDA | Metal | CPU |
|---------|------|-------|-----|
| `kernel` | `extern "C" __global__` | `kernel` | `static inline` |
| `threadgroup_mem` | `__shared__` | `threadgroup` | `static` |
| `device` | (no qualifier) | `device` | (no qualifier) |
| `constant` | `const` | `constant` | `const` |

### Barrier

```cpp
threadgroup_barrier_all();  // Sync threads in threadgroup
```

### Built-in types

- `float2`, `float3`, `float4` — vector types with `.xyzw`/`.rgba` access
- `uint2` — unsigned integer vector for coordinates
- `pixel_format` — `float4` or `half4` based on `USE_HALF_PRECISION`

### Math functions

| Function | Description |
|----------|-------------|
| `clamp(v, lo, hi)` | Clamp value/vector to range |
| `mix(a, b, t)` | Linear interpolation |
| `min(a, b)`, `max(a, b)` | Minimum/maximum |
| `abs(v)` | Absolute value |
| `floor(v)`, `fract(v)` | Floor and fractional parts |

---

## Tips

### Use pixel_format for portability

Prefer `pixel_format` over raw `float4`/`half4`. It adapts to `USE_HALF_PRECISION`
and ensures consistent memory layout across host and device.

```cpp
// Good: portable
param_ro(pixel_format, src, 0);

// Avoid: hardcoded precision
param_ro(float4, src, 0);
```

### Keep parameters POD

POD (Plain Old Data) structs have:

- No constructors, destructors, or virtual functions
- No pointers or references inside
- Only scalars (`uint`, `float`) and vectors (`float2/3/4`)
- Fixed size with explicit padding for alignment

```cpp
// Good: POD
struct MyParams {
    float intensity;
    float _pad[3];  // Align to 16 bytes
    float3 color;
};

// Bad: non-POD
struct BadParams {
    std::vector<float> data;  // Heap allocation
    MyParams(const MyParams& other) { ... }  // Constructor
};
```

### Bounds check early

Always guard against out-of-bounds access at kernel entry:

```cpp
kernel void my_kernel(...) {
    uint2 gid = dispatch_id();
    if (gid.x >= width || gid.y >= height)
        return;
    // ... safe processing
}
```

### Use coordinate helpers

For UV calculations, use helpers from `image/coords.h`:

```cpp
#include "vekl/common.h"

float2 uv = tex_coord(gid, float2(width, height));  // Normalized [0,1]
float2 centered = uv_to_centered(uu);               // [-1,1] centered
```

---

## License

Apache-2.0. See `LICENSE`.