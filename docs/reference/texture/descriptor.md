# Texture — Descriptor

Defines the core data types that describe pixel buffers and frame metadata.

Source: [`texture/descriptor.slang`](../../texture/descriptor.slang)

---

## Enums

### `PixelStorage`

Controls how pixel data is packed in memory.

| Value | Description |
|-------|-------------|
| `Unorm8x4` | 4 channels × 8-bit unsigned normalized (32 bits/pixel) |
| `Unorm16x4` | 4 channels × 16-bit unsigned normalized (64 bits/pixel) |
| `Float32x4` | 4 channels × 32-bit float (128 bits/pixel) |

### `PixelLayout`

Controls the channel ordering and color space of pixel data.

| Value | Description |
|-------|-------------|
| `Rgba` | RGBA channel order, linear color space |
| `Bgra` | BGRA channel order (swizzled RGBA) |
| `Vuya` | VUYA (BT.601) — YCbCr with Rec. 601 coefficients |
| `Vuya709` | VUYA (BT.709) — YCbCr with Rec. 709 coefficients |

### `AddressMode`

Controls out-of-bounds UV behavior when sampling.

| Value | Description |
|-------|-------------|
| `Clamp` | Clamp coordinates to `[0, size-1]` |
| `Repeat` | Wrap coordinates with modulo |
| `Mirror` | Reflect coordinates at boundaries |

---

## Constants

### `MAX_MIP`

```cpp
public static const uint MAX_MIP = 5u;
```

Upper bound on the number of mip levels a `TextureDesc` can describe.
Five levels cover 1/16 per axis (1920×1080 → 60×33), enough for any
sweep-blur / glow / bokeh pyramid. The constant **must** match the
host-side `prgpu::types::MAX_MIP` exactly; any change has to land on
both sides in lockstep.

---

## Structs

### `TextureDesc`

Describes the layout and format of a pixel buffer, with optional
mip-chain metadata for kernels that sample from multiple resolutions.

| Field | Type | Description |
|-------|------|-------------|
| `width` | `uint` | Buffer width in pixels (level 0) |
| `height` | `uint` | Buffer height in pixels (level 0) |
| `pitchBytes` | `uint` | Row pitch in bytes (level 0) |
| `bytesPerPixel` | `uint` | Bytes per pixel (4, 8, or 16) |
| `storage` | `PixelStorage` | Pixel storage format |
| `layout` | `PixelLayout` | Channel order / color space |
| `addressMode` | `AddressMode` | Out-of-bounds addressing mode |
| `mipLevelCount` | `uint` | Number of valid mip levels (1 = no chain) |
| `mipOffsetBytes[MAX_MIP]` | `uint[5]` | Byte offset of each level inside `buffer` |
| `mipWidth[MAX_MIP]` | `uint[5]` | Pixel width of each level |
| `mipHeight[MAX_MIP]` | `uint[5]` | Pixel height of each level |
| `mipPitchBytes[MAX_MIP]` | `uint[5]` | Row pitch (bytes) of each level |

#### Mip-chain layout

When `mipLevelCount > 1`, the backing buffer contains every level
concatenated byte-wise from level 0 outward:

```
byte 0                                        buffer size
|                                                        |
[      lod 0      ][ lod 1 ][lod 2][lod 3][lod 4]
 mipOffsetBytes[0] = 0
                   mipOffsetBytes[1] = pitch0 * height
                            mipOffsetBytes[2] = ...
```

- Level 0 keeps the host-chosen pitch (may be padded so rows align
  with backend requirements).
- Levels 1..N use **tight** pitches (`mipWidth[i] * bytesPerPixel`),
  so no space is wasted on padding.
- Level dimensions are `max(width >> lod, 1)` / `max(height >> lod, 1)`
  — odd sizes floor with a 1-pixel minimum.

The total byte budget for the whole chain is the `prgpu::types::mip_buffer_size_bytes`
sum, which stays under `ceil(4/3) × base` for any level count.

#### Single-level buffers

Kernels that never call the lod-aware overloads on `TextureView` /
`RWTextureView` can ignore the `mip*` fields entirely. When the host
doesn't request a mip chain, `mipLevelCount == 1` and level 0 mirrors
`width / height / pitchBytes` so single-level code paths see no
behaviour change.

### `FrameParams`

Per-frame metadata passed to every kernel as a constant buffer.

| Field | Type | Description |
|-------|------|-------------|
| `outDesc` | `TextureDesc` | Descriptor for the outgoing (primary source) buffer |
| `inDesc` | `TextureDesc` | Descriptor for the incoming (secondary source) buffer |
| `dstDesc` | `TextureDesc` | Descriptor for the destination buffer |
| `width` | `uint` | Frame width in pixels |
| `height` | `uint` | Frame height in pixels |
| `time` | `float` | Current time in seconds |
| `progress` | `float` | Transition progress `[0, 1]` |
