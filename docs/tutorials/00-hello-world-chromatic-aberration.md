# Tutorial: Hello World — Chromatic Aberration

Write your first VEKL effect from scratch. You'll build a chromatic aberration
shader that offsets color channels to create a prism-like color fringing
artifact.

**Time to complete**: ~20 minutes

**Prerequisites**: Familiarity with C-like syntax. No prior Slang or GPU
experience required.

**What you'll learn**:

- Importing VEKL
- Defining effect parameters
- Reading and writing pixels with `TextureView`
- Sampling with UV coordinate transforms
- Compiling to multiple GPU backends

---

## Step 1 — Create the shader file

Create `chromatic.slang`. Every VEKL shader starts with the module import:

```cpp
import vekl;
```

This gives you access to all VEKL types and functions: `TextureView`,
`RWTextureView`, `FrameParams`, `TexCoord`, `ScaleUV`, and more.

---

## Step 2 — Define effect parameters

Effects need user-controllable parameters. Define them as a Slang struct
that will be passed as a constant buffer:

```cpp
import vekl;

struct ChromaticParams
{
    float amount;
    float angle;
};
```

**Rules for parameter structs:**

- Use `float` for scalars, `float2`/`float3`/`float4` for vectors
- Use `uint` for integers (popup indices, checkboxes)
- Keep fields as plain data — no pointers, no constructors
- Group `float`s together, `uint`s together to minimize padding

---

## Step 3 — Write the kernel entry point

Every compute kernel needs the `[shader("compute")]` attribute and a thread
group size. VEKL uses `16 × 16` threads per group:

```cpp
import vekl;

struct ChromaticParams
{
    float amount;
    float angle;
};

[shader("compute")]
[numthreads(16, 16, 1)]
void chromatic(
    uint3 threadId : SV_DispatchThreadID,
    StructuredBuffer<uint> outgoing,
    StructuredBuffer<uint> incoming,
    RWStructuredBuffer<uint> dst,
    ConstantBuffer<FrameParams> frame,
    ConstantBuffer<ChromaticParams> params
)
{
    // Kernel body goes here
}
```

**Parameter breakdown:**

| Parameter | Role |
|-----------|------|
| `threadId` | GPU thread position — your pixel coordinate |
| `outgoing` | Primary source buffer (the input image) |
| `incoming` | Secondary source buffer (for transitions or multi-pass) |
| `dst` | Destination buffer (where you write output) |
| `frame` | Frame metadata: dimensions, pitch, format, time |
| `params` | Your effect-specific parameters |

---

## Step 4 — Bounds check and set up views

The first thing every kernel does: check whether this thread should process
a pixel, then create texture views for I/O:

```cpp
{
    RWTextureView output = RWTextureView(dst, frame.dstDesc);
    if (!output.Contains(threadId.xy))
        return;

    TextureView src = TextureView(outgoing, frame.outDesc);
}
```

**Why bounds check?** The GPU dispatches thread groups in fixed-size blocks.
If the image dimensions aren't divisible by 16, some threads fall outside
the image. `Contains` prevents out-of-bounds writes.

---

## Step 5 — Compute UV coordinates

UV coordinates map pixel positions to `[0, 1]`. VEKL's `TexCoord` handles
the half-pixel offset:

```cpp
    uint2 size = src.Size();
    float2 uv = TexCoord(threadId.xy, size);
```

Now `uv = (0.5, 0.5)` represents the center of the image.

---

## Step 6 — Sample each color channel at offset positions

Chromatic aberration offsets R, G, B channels to different UV positions:

```cpp
    float2 direction = float2(cos(params.angle), sin(params.angle));
    float offset = params.amount * 0.01;

    float2 uvR = uv + direction * offset;
    float2 uvG = uv;
    float2 uvB = uv - direction * offset;

    float4 colorR = src.SampleLinear(uvR);
    float4 colorG = src.SampleLinear(uvG);
    float4 colorB = src.SampleLinear(uvB);
```

`SampleLinear` performs bilinear interpolation for smooth results. The
`TextureView` handles pixel format conversion automatically — you always
work in RGBA `float4` space.

---

## Step 7 — Combine and write the result

Assemble the final color from individually-sampled channels:

```cpp
    float4 result = float4(
        colorR.x,
        colorG.y,
        colorB.z,
        colorG.w
    );

    output.Store(threadId.xy, result);
```

`Store` converts back from RGBA working space to the native pixel format
automatically.

---

## Complete shader

```cpp
import vekl;

struct ChromaticParams
{
    float amount;
    float angle;
};

[shader("compute")]
[numthreads(16, 16, 1)]
void chromatic(
    uint3 threadId : SV_DispatchThreadID,
    StructuredBuffer<uint> outgoing,
    StructuredBuffer<uint> incoming,
    RWStructuredBuffer<uint> dst,
    ConstantBuffer<FrameParams> frame,
    ConstantBuffer<ChromaticParams> params
)
{
    RWTextureView output = RWTextureView(dst, frame.dstDesc);
    if (!output.Contains(threadId.xy))
        return;

    TextureView src = TextureView(outgoing, frame.outDesc);
    uint2 size = src.Size();
    float2 uv = TexCoord(threadId.xy, size);

    float2 direction = float2(cos(params.angle), sin(params.angle));
    float offset = params.amount * 0.01;

    float2 uvR = uv + direction * offset;
    float2 uvG = uv;
    float2 uvB = uv - direction * offset;

    float4 colorR = src.SampleLinear(uvR);
    float4 colorG = src.SampleLinear(uvG);
    float4 colorB = src.SampleLinear(uvB);

    float4 result = float4(colorR.x, colorG.y, colorB.z, colorG.w);

    output.Store(threadId.xy, result);
}
```

---

## Step 8 — Compile

Compile through the Slang compiler targeting your platform:

```bash
# Metal (macOS)
slangc chromatic.slang -I /path/to/vekl -target metal \
    -entry chromatic -stage compute -o chromatic.metallib

# CUDA (NVIDIA)
slangc chromatic.slang -I /path/to/vekl -target cuda \
    -entry chromatic -stage compute -o chromatic.ptx

# SPIR-V (Vulkan)
slangc chromatic.slang -I /path/to/vekl -target spirv \
    -entry chromatic -stage compute -o chromatic.spv
```

The `-I` flag tells Slang where to find the `vekl` module.

---

## Recap

You learned:

1. **Import VEKL** with `import vekl;`
2. **Define parameters** as a plain struct for the constant buffer
3. **Write the kernel** with the standard 6-parameter signature
4. **Bounds check** with `RWTextureView.Contains()`
5. **Convert coordinates** with `TexCoord()` for UV-based sampling
6. **Read pixels** with `TextureView.SampleLinear()`
7. **Write pixels** with `RWTextureView.Store()`
8. **Compile** through Slang to any backend

### Next steps

- [API Reference](../reference/index.md) — complete type and function catalog
- [How-to Guides](../how-to-guides.md) — recipes for blur, noise, UV transforms
- [Vignette shader](../../vignette/shaders/vignette.slang) — real-world multi-feature effect
