# Texture — View

High-level texture access through `TextureView` and `RWTextureView` structs.

Source: [`texture/view.slang`](../../texture/view.slang)

---

## `TextureView`

Read-only texture accessor. Combines a buffer with a `TextureDesc` for
type-safe pixel reads and sampling.

| Field | Type | Description |
|-------|------|-------------|
| `buffer` | `StructuredBuffer<uint>` | Underlying pixel buffer |
| `desc` | `TextureDesc` | Buffer format and dimensions |

### Methods

#### Level-0 (no mip)

| Method | Signature | Description |
|--------|-----------|-------------|
| `Contains` | `bool Contains(uint2 pixel)` | True if pixel is within bounds |
| `Size` | `uint2 Size()` | Dimensions as `uint2(width, height)` |
| `Load` | `float4 Load(uint2 pixel)` | Read pixel at integer coords, converts to RGBA. No bounds check. |
| `LoadSafe` | `float4 LoadSafe(uint2 pixel)` | Read pixel with address mode handling |
| `SampleNearest` | `float4 SampleNearest(float2 uv)` | Nearest-neighbor sample at UV `[0, 1]`, clamped |
| `SampleLinear` | `float4 SampleLinear(float2 uv)` | Bilinear sample at UV `[0, 1]`, clamped |
| `SampleLinearRepeat` | `float4 SampleLinearRepeat(float2 uv)` | Bilinear with tiling address mode |
| `SampleLinearMirror` | `float4 SampleLinearMirror(float2 uv)` | Bilinear with mirroring address mode |

#### Mip-chain (lod ≥ 0)

These overloads read from a specific mip level and are no-ops for
single-level buffers (`mipLevelCount == 1` → any `lod` clamps to 0).
See [`descriptor.md`](descriptor.md) for the chain layout.

| Method | Signature | Description |
|--------|-----------|-------------|
| `Size(lod)` | `uint2 Size(uint lod)` | Dimensions of the requested mip level |
| `Contains(lod)` | `bool Contains(uint2 pixel, uint lod)` | True if pixel is within that level's bounds |
| `Load(lod)` | `float4 Load(uint2 pixel, uint lod)` | Read from a specific mip level |
| `SampleLinear(lod)` | `float4 SampleLinear(float2 uv, uint lod)` | Bilinear sample on a specific level, clamp-addressed |
| `SampleLinearTrilinear` | `float4 SampleLinearTrilinear(float2 uv, float lodF)` | Trilinear sample — lerps between `floor(lodF)` and `ceil(lodF)`. Use this to pick a continuous lod per pixel (pyramidal blur / glow). |

Example — pick a lod from a per-pixel trail length:

```cpp
// trail_px is the screen-space length this pixel sweeps over; lower-res
// mips are sampled when the trail exceeds the Nyquist of level 0.
float lodF = clamp(log2(trail_px / 16.0), 0.0, float(src.desc.mipLevelCount - 1u));
float4 c = src.SampleLinearTrilinear(uv, lodF);
```

---

## `RWTextureView`

Read-write texture accessor with write capability. Lod-aware reads and
writes let a single kernel pass both read from and write to disjoint
regions of the same backing buffer (used by `prgpu`'s built-in mip
downsampler).

| Field | Type | Description |
|-------|------|-------------|
| `buffer` | `RWStructuredBuffer<uint>` | Underlying pixel buffer |
| `desc` | `TextureDesc` | Buffer format and dimensions |

### Methods

#### Level-0 (no mip)

| Method | Signature | Description |
|--------|-----------|-------------|
| `Contains` | `bool Contains(uint2 pixel)` | True if pixel is within bounds |
| `Size` | `uint2 Size()` | Dimensions as `uint2(width, height)` |
| `Load` | `float4 Load(uint2 pixel)` | Read pixel at integer coords, converts to RGBA |
| `Store` | `void Store(uint2 pixel, float4 value)` | Write pixel, converts from RGBA to native format |

#### Mip-chain (lod ≥ 0)

| Method | Signature | Description |
|--------|-----------|-------------|
| `Size(lod)` | `uint2 Size(uint lod)` | Dimensions of the requested mip level |
| `Contains(lod)` | `bool Contains(uint2 pixel, uint lod)` | True if pixel is within that level's bounds |
| `Load(lod)` | `float4 Load(uint2 pixel, uint lod)` | Read from a specific mip level |
| `Store(lod)` | `void Store(uint2 pixel, float4 value, uint lod)` | Write to a specific mip level |

Example — generate one mip level from the previous one (2×2 box average):

```cpp
RWTextureView mip = RWTextureView(buffer, frame.outDesc);
uint2 dstSize = mip.Size(srcLod + 1u);
if (threadId.x >= dstSize.x || threadId.y >= dstSize.y) return;

uint2 base = threadId.xy * 2u;
uint2 m    = mip.Size(srcLod) - uint2(1u, 1u);
float4 avg = (mip.Load(min(base,                 m), srcLod)
            + mip.Load(min(base + uint2(1,0),    m), srcLod)
            + mip.Load(min(base + uint2(0,1),    m), srcLod)
            + mip.Load(min(base + uint2(1,1),    m), srcLod)) * 0.25;
mip.Store(threadId.xy, avg, srcLod + 1u);
```
