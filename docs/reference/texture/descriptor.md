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

## Structs

### `TextureDesc`

Describes the layout and format of a pixel buffer.

| Field | Type | Description |
|-------|------|-------------|
| `width` | `uint` | Buffer width in pixels |
| `height` | `uint` | Buffer height in pixels |
| `pitchBytes` | `uint` | Row pitch in bytes |
| `bytesPerPixel` | `uint` | Bytes per pixel (4, 8, or 16) |
| `storage` | `PixelStorage` | Pixel storage format |
| `layout` | `PixelLayout` | Channel order / color space |
| `addressMode` | `AddressMode` | Out-of-bounds addressing mode |

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
