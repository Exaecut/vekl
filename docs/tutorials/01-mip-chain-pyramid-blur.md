# Tutorial: Mip chain pyramids — a 20-line pyramidal blur

Learn how to sample from multiple mip levels in a single shader. You'll
build a tiny pyramidal blur that picks a lower-resolution mip per pixel
based on a "trail length" — the same technique every production motion-
blur / glow / bokeh effect uses to stay interactive at 4K.

**Time to complete**: ~15 minutes

**Prerequisites**: Completed [tutorial 00 — Hello World](00-hello-world-chromatic-aberration.md)
or equivalent knowledge of VEKL's `TextureView` API.

**What you'll learn**:

- Reading the `mipLevelCount` + `mipWidth[]` / `mipHeight[]` / `mipOffsetBytes[]`
  fields of `TextureDesc`
- Calling the lod-aware overloads on `TextureView` (`Size(lod)`,
  `Load(px, lod)`, `SampleLinear(uv, lod)`, `SampleLinearTrilinear(uv, lodF)`)
- Picking a continuous lod per pixel and using trilinear to hide seams
- What the **host** side has to do to enable mips (see
  [`prgpu/docs/mip_chain.md`](../../../../prgpu/docs/mip_chain.md))

---

## The idea

A blur with radius `r` has to average `~r²` pixels. If you sample the
same blur from mip level `lod = log2(r)`, you're averaging 4× fewer
pixels for each level down the pyramid — *four* taps at mip 2 give the
same perceptual result as *sixteen* taps at mip 0, because a mip 2 texel
already covers 4×4 of the original.

Effects that sample a variable-size neighborhood (motion blur, zoom
blur, spin blur, glow, bokeh, depth-of-field) exploit this to stay flat
in compute cost regardless of blur radius: bigger radius → deeper mip,
same number of taps.

---

## Step 1 — Preconditions

This tutorial assumes the **host** has already populated a mip chain
for you. That means:

1. The host allocated a mip-capable buffer via
   `prgpu::gpu::buffer::get_or_create_with_mips(...)` (GPU) or
   `prgpu::cpu::buffer::get_or_create_with_mips(...)` (CPU).
2. The host copied the source frame into level 0 of that buffer.
3. The host ran `prgpu::kernels::mip::generate_mips(&config)` to fill
   levels 1..N-1.
4. The host set `config.outgoing_mip_levels = N` **before** dispatching
   your effect kernel.

When those four steps are done, prgpu's dispatcher auto-fills
`frame.outDesc.mipLevelCount / mipWidth / mipHeight / mipPitchBytes /
mipOffsetBytes` for you — no shader-side setup required.

See [`prgpu/docs/mip_chain.md`](../../../../prgpu/docs/mip_chain.md) for
the full host-side recipe.

---

## Step 2 — Bind the mip-chained source as a `TextureView`

Nothing new here. The same `TextureView src = TextureView(outgoing,
frame.outDesc);` you already use for level-0 sampling also exposes the
mip overloads.

```cpp
import vekl;

struct PyramidBlurParams
{
    float radius;    // blur radius in pixels at level 0
    float _pad0;
    float _pad1;
    float _pad2;
};

[shader("compute")]
[numthreads(16, 16, 1)]
void pyramid_blur(
    uint3 threadId : SV_DispatchThreadID,
    StructuredBuffer<uint> outgoing,
    StructuredBuffer<uint> incoming,
    RWStructuredBuffer<uint> dst,
    ConstantBuffer<FrameParams> frame,
    ConstantBuffer<PyramidBlurParams> params)
{
    RWTextureView output = RWTextureView(dst, frame.dstDesc);
    if (!output.Contains(threadId.xy))
        return;

    TextureView src = TextureView(outgoing, frame.outDesc);
    uint2  size = src.Size();
    float2 uv   = TexCoord(threadId.xy, size);

    // ...lod selection + sampling below...
}
```

---

## Step 3 — Pick a continuous lod

The rule of thumb: one mip level halves both axes, so moving from lod N
to lod N+1 multiplies the effective filter radius by 2.

For a blur of radius `r` pixels, you want `lod = log2(r / T)` where `T`
is the target tap count per axis at the chosen level (4 taps is a good
default: a 2×2 bilinear read already does most of the smoothing).

```cpp
float maxLod = float(max(frame.outDesc.mipLevelCount, 1u) - 1u);
float lodF   = clamp(log2(max(params.radius, 1.0) / 4.0), 0.0, maxLod);
```

`clamp(..., 0.0, maxLod)` keeps us in range when the radius is tiny
(fall back to lod 0) or enormous (stop at the deepest level the host
built).

---

## Step 4 — Sample with trilinear

`SampleLinearTrilinear` lerps between `floor(lodF)` and `ceil(lodF)` so
you don't see seams at the integer boundaries:

```cpp
float4 blurred = src.SampleLinearTrilinear(uv, lodF);
output.Store(threadId.xy, blurred);
```

That's the whole inner loop — one trilinear tap replaces ~`(radius)²`
taps a naive blur would need.

---

## Step 5 — Full shader

Twenty lines, uses every mip API you need:

```cpp
import vekl;

struct PyramidBlurParams
{
    float radius;
    float _pad0;
    float _pad1;
    float _pad2;
};

[shader("compute")]
[numthreads(16, 16, 1)]
void pyramid_blur(
    uint3 threadId : SV_DispatchThreadID,
    StructuredBuffer<uint> outgoing,
    StructuredBuffer<uint> incoming,
    RWStructuredBuffer<uint> dst,
    ConstantBuffer<FrameParams> frame,
    ConstantBuffer<PyramidBlurParams> params)
{
    RWTextureView output = RWTextureView(dst, frame.dstDesc);
    if (!output.Contains(threadId.xy))
        return;

    TextureView src = TextureView(outgoing, frame.outDesc);
    float2 uv  = TexCoord(threadId.xy, src.Size());

    float maxLod = float(max(frame.outDesc.mipLevelCount, 1u) - 1u);
    float lodF   = clamp(log2(max(params.radius, 1.0) / 4.0), 0.0, maxLod);

    output.Store(threadId.xy, src.SampleLinearTrilinear(uv, lodF));
}
```

---

## Variations

### Per-pixel lod (zoom / motion / spin)

Sweep blurs pick `radius` from a per-pixel trail length. Replace the
uniform `params.radius` with a local expression:

```cpp
float2 d_px   = (uv - origin) * float2(src.Size());
float  trail  = 2.0 * length(d_px) * abs(sin(params.angle * 0.5));
float  lodF   = clamp(log2(max(trail, 1.0) / 4.0), 0.0, maxLod);
```

Origin-adjacent pixels have `trail ≈ 0` → `lod 0` (no blur); far
pixels have large `trail` → deep mips (cheap heavy blur). This is the
basis of the Phase 3 radialblur in this repo.

### Explicit `Load` per level

When you don't want bilinear interpolation (tile-based effects, mosaic,
pixelate), use `Load(px, lod)` and treat each mip as its own integer-
indexed grid:

```cpp
uint   lod = 3u;               // 1/8 per axis
uint2  sz  = src.Size(lod);
uint2  px  = uv * float2(sz);
float4 c   = src.Load(min(px, sz - uint2(1u, 1u)), lod);
```

### Querying the chain

`src.desc.mipLevelCount` is `1` when the host didn't request a chain
— your effect can fall back to a plain `SampleLinear(uv)` in that case:

```cpp
if (frame.outDesc.mipLevelCount > 1u) {
    output.Store(threadId.xy, src.SampleLinearTrilinear(uv, lodF));
} else {
    output.Store(threadId.xy, src.SampleLinear(uv));
}
```

---

## CPU vs GPU behaviour

The shader above is **identical** on Metal, CUDA, and the CPU
(slang-cpp) fallback. Slang compiles each target separately, but the
mip-chain code path uses only the byte-addressed `StructuredBuffer<uint>`
access that VEKL already maps consistently across backends. There is no
native mip-texture type in play — the pyramid lives in one flat buffer
with `mipOffsetBytes[]` telling you where each level starts.

The **host** side is where the backends differ. See
[`prgpu/docs/mip_chain.md`](../../../../prgpu/docs/mip_chain.md) for:

- How the CPU allocator (`std::Vec<u8>` cache) differs from the Metal
  allocator (`MTLBuffer` via `newBufferWithLength:options:`) and the
  CUDA allocator (`cuMemAlloc_v2`).
- Why the GPU paths need a one-shot buffer-to-buffer blit to put level
  0 in the right place, while the CPU path can reuse the Layer buffer
  directly if it's contiguous.
- How `generate_mips(&config)` auto-routes to `render_cpu_direct` vs
  `backends::dispatch_kernel` based on whether `device_handle` is
  bound.
