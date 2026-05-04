# Texture — Format

Low-level pixel I/O with automatic format conversion and address mode handling.

Source: [`texture/format.slang`](../../texture/format.slang)

---

## Color Space Conversion

### `ToRGBA`

```cpp
float4 ToRGBA(float4 c, PixelLayout layout)
```

Convert a raw pixel value to RGBA working space. Handles BGRA swizzle and
VUYA → RGB conversion using the appropriate BT.601 or BT.709 coefficients.

| Parameter | Description |
|-----------|-------------|
| `c` | Raw pixel value in native layout |
| `layout` | Source pixel layout |
| **Returns** | Pixel value in RGBA linear space |

### `FromRGBA`

```cpp
float4 FromRGBA(float4 c, PixelLayout layout)
```

Convert an RGBA working-space value back to the native pixel layout. Inverse
of `ToRGBA`.

| Parameter | Description |
|-----------|-------------|
| `c` | Pixel value in RGBA linear space |
| `layout` | Target pixel layout |
| **Returns** | Pixel value in native layout |

---

## Raw Pixel I/O

### `LoadPixelRaw` (read-only buffer)

```cpp
float4 LoadPixelRaw(StructuredBuffer<uint> data, uint pitchBytes, uint2 xy,
                     uint bytesPerPixel, PixelStorage storage)
```

Read a pixel from a read-only buffer without layout conversion. Decodes the
packed integer representation to `float4` based on `PixelStorage`.

### `LoadPixelRaw` (read-write buffer)

```cpp
float4 LoadPixelRaw(RWStructuredBuffer<uint> data, uint pitchBytes, uint2 xy,
                     uint bytesPerPixel, PixelStorage storage)
```

Same as above but accepts a `RWStructuredBuffer<uint>`.

### `StorePixelRaw`

```cpp
void StorePixelRaw(RWStructuredBuffer<uint> data, uint pitchBytes, uint2 xy,
                    float4 c, uint bytesPerPixel, PixelStorage storage)
```

Write a pixel to a buffer without layout conversion. Values are saturated to
`[0, 1]` before encoding for `Unorm8x4` and `Unorm16x4`.

---

## Pixel I/O with Layout Conversion

### `LoadPixel` (read-only buffer)

```cpp
float4 LoadPixel(StructuredBuffer<uint> data, uint pitchBytes, uint2 xy,
                  uint bytesPerPixel, PixelStorage storage, PixelLayout layout)
```

Read a pixel and convert to RGBA working space. Equivalent to
`ToRGBA(LoadPixelRaw(...), layout)`.

### `LoadPixel` (read-write buffer)

```cpp
float4 LoadPixel(RWStructuredBuffer<uint> data, uint pitchBytes, uint2 xy,
                  uint bytesPerPixel, PixelStorage storage, PixelLayout layout)
```

Same as above but accepts a `RWStructuredBuffer<uint>`.

### `StorePixel`

```cpp
void StorePixel(RWStructuredBuffer<uint> data, uint pitchBytes, uint2 xy,
                 float4 c, uint bytesPerPixel, PixelStorage storage, PixelLayout layout)
```

Convert from RGBA working space and write. Equivalent to
`StorePixelRaw(..., FromRGBA(c, layout), ...)`.

---

## Address Helpers

| Function | Signature | Description |
|----------|-----------|-------------|
| `AddressClamp` | `uint2 AddressClamp(uint2 xy, uint2 sizePx)` | Clamp to `[0, sizePx - 1]` |
| `AddressRepeat` | `uint2 AddressRepeat(uint2 xy, uint2 sizePx)` | Wrap with modulo |
| `AddressMirror` | `uint2 AddressMirror(uint2 xy, uint2 sizePx)` | Reflect at boundaries |
| `AddressXY` | `uint2 AddressXY(uint2 xy, uint2 sizePx, AddressMode mode)` | Dispatch by mode |

---

## Safe Pixel Load

### `LoadPixelSafe`

```cpp
float4 LoadPixelSafe(StructuredBuffer<uint> data, uint pitchBytes, uint2 xy,
                      uint2 sizePx, uint bytesPerPixel, PixelStorage storage,
                      PixelLayout layout, AddressMode address)
```

Load a pixel with address mode handling. Applies `AddressXY` before loading,
ensuring out-of-bounds coordinates are resolved safely.
