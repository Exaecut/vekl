# VEKL API Reference

Complete reference for all types, enums, and functions provided by the VEKL
module. Import everything with a single statement:

```cpp
import vekl;
```

## Concerns

| Concern | Files | Description |
|---------|-------|-------------|
| [texture](texture/) | `descriptor.slang`, `format.slang`, `view.slang` | Enums, descriptors, pixel I/O, texture views |
| [sampling](sampling/) | `coordinate.slang`, `bicubic.slang`, `radial.slang` | UV utilities, bicubic reconstruction, sweep / pyramid sampling |
| [math](math/) | `mask.slang` | Distance masks |
| [filter](filter/) | `gaussian.slang` | Gaussian blur |
| [noise](noise/) | `perlin.slang`, `fbm.slang`, `hash.slang` | Perlin noise, FBM, and stateless hash primitives |
| [color](color/) | `blend/dispatch.slang`, `accumulate.slang`, `yiq.slang`, `oklab.slang` | Blend mode dispatcher, chroma-correct accumulators, YIQ + Oklab color spaces |

## Standard Kernel Signature

All VEKL-compatible kernels follow this parameter pattern:

```cpp
[shader("compute")]
[numthreads(16, 16, 1)]
void kernel_name(
    uint3 threadId : SV_DispatchThreadID,       // Thread ID
    StructuredBuffer<uint> outgoing,             // Buffer 0: primary source
    StructuredBuffer<uint> incoming,             // Buffer 1: secondary source
    RWStructuredBuffer<uint> dst,                // Buffer 2: destination
    ConstantBuffer<FrameParams> frame,           // Buffer 3: frame metadata
    ConstantBuffer<MyEffectParams> params        // Buffer 4: effect parameters
)
```

The thread group size `16 × 16 = 256` matches common GPU warp/wavefront
sizes and provides good occupancy across CUDA and Metal.
