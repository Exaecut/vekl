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

---

## `RWTextureView`

Read-write texture accessor with write capability.

| Field | Type | Description |
|-------|------|-------------|
| `buffer` | `RWStructuredBuffer<uint>` | Underlying pixel buffer |
| `desc` | `TextureDesc` | Buffer format and dimensions |

### Methods

| Method | Signature | Description |
|--------|-----------|-------------|
| `Contains` | `bool Contains(uint2 pixel)` | True if pixel is within bounds |
| `Size` | `uint2 Size()` | Dimensions as `uint2(width, height)` |
| `Load` | `float4 Load(uint2 pixel)` | Read pixel at integer coords, converts to RGBA |
| `Store` | `void Store(uint2 pixel, float4 value)` | Write pixel, converts from RGBA to native format |
