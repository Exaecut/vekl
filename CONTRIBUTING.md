# Contributing to VEKL

Guidelines for developing the Video Effects Kernel Library.

---

## Module Structure

VEKL is a single Slang module organized into **concerns**. Each concern maps
to a root-level aggregator file and a subdirectory of implementing files:

```
vekl/
├── vekl.slang            # module vekl; — includes aggregator files only
├── texture.slang         # implementing vekl; — includes texture/* files
├── texture/
│   ├── descriptor.slang  # implementing vekl;
│   ├── format.slang
│   └── view.slang
├── noise.slang           # implementing vekl; — includes noise/* files
├── noise/
│   ├── perlin.slang
│   └── fbm.slang
└── ...
```

### Rules

1. **`vekl.slang` only includes aggregator files** — never implementing files
   from subdirectories directly.
2. **Each aggregator file lives at the root** next to its subdirectory.
   The aggregator `__include`s all implementing files within that concern.
3. **Maximum two levels deep** — `color/blend/add.slang` is the deepest.
   Never `color/blend/gaussian/add.slang`.
4. **Every implementing file starts with `implementing vekl;`** — no
   exceptions.
5. **Users never see the internal structure** — they write `import vekl;`
   and get everything. Slang's dead code elimination strips unused functions.

### Adding a feature to an existing concern

1. Create the `.slang` file in the concern's subdirectory
2. Add `implementing vekl;` at the top
3. Mark public functions with `public`
4. Add `__include "subdir/filename"` to the concern's aggregator file

### Adding a new concern

1. Create the subdirectory with implementing files
2. Create the aggregator file at the root (e.g. `color.slang`)
3. Add `__include "concern"` to `vekl.slang`

---

## Prefer Generic Functions

Use Slang generics with interface constraints instead of per-type overloads
whenever possible. Constrain type parameters to the appropriate builtin
interface:

- **`IFloat`** — floating-point types with arithmetic (`float`, `float2`,
  `float3`, `float4`, `half`, `double`, and their vectors)
- **`IArithmetic`** — any type supporting `+`, `-`, `*`, `/` (scalars and
  vectors, both integer and float)
- **`IInteger`** — integer types with arithmetic

```slang
// ✅ Preferred — generic with IFloat constraint, works for float/float2/float3/float4
public T BlendAdd<T>(T base, T blend) where T : IFloat
{
    return base + blend;
}

// ❌ Avoid — separate overloads for each type
public float4 BlendAdd(float4 base, float4 blend) { return base + blend; }
public float3 BlendAdd(float3 base, float3 blend) { return base + blend; }
```

**When generics don't work** (e.g., functions that construct specific types
like `float4(...)` from individual components, or functions that only make
sense for a specific vector width), concrete signatures are acceptable.
But always try the generic version first.

---

## Naming Conventions

Follow Slang/HLSL conventions:

| Category | Style | Examples |
|----------|-------|---------|
| Types, structs, enums | PascalCase | `TextureView`, `PixelLayout`, `FrameParams` |
| Functions | PascalCase | `LoadPixel`, `Gaussian1d`, `BlendAdd` |
| Struct fields | camelCase | `pitchBytes`, `bytesPerPixel`, `addressMode` |
| Enum values | PascalCase | `Unorm8x4`, `AddressMode.Clamp` |
| Local variables | camelCase | `wordOffset`, `chromaOffset` |
| Internal helpers | PascalCase | `Hash2`, `Grad`, `Fade`, `AddressClamp` |
| Generic type params | Single uppercase | `T` |

### File naming

- Descriptive names matching the feature: `descriptor.slang`, `coordinate.slang`,
  `gaussian.slang`, `perlin.slang`
- One primary concept per file
- If a file grows beyond ~200 lines, split by feature (e.g., `perlin.slang`
  and `fbm.slang` instead of one `noise.slang`)

---

## Documentation

### Code comments

- **Short, clear, concise** — one line is ideal, two at most
- **Describe "why" and "when"**, not "what" — the code says what
- **Self-document via naming** — a function called `BlendAdd` doesn't need
  "Adds two colors together" in its doc comment

```slang
// ✅ Good — explains purpose and use case
/// Multiply blend. Use for shadows, vignettes, and darkening.
public T BlendMultiply<T>(T base, T blend)

// ❌ Bad — restates what the code already says
/// Multiplies base by blend and returns the result.
public T BlendMultiply<T>(T base, T blend)
```

### Reference docs

Each implementing file has a corresponding doc in `docs/reference/`:

```
texture/descriptor.slang  → docs/reference/texture/descriptor.md
texture/format.slang      → docs/reference/texture/format.md
noise/perlin.slang        → docs/reference/noise/perlin.md
color/blend/add.slang     → docs/reference/color/blend.md
```

Reference docs follow the concern structure (condensed), not a 1:1 file
mapping. If multiple files in a subdirectory are small, combine them into
one doc page.

**Reference doc template:**

```markdown
# Concern — Feature

One-line description.

Source: [`concern/feature.slang`](../../concern/feature.slang)

---

## Functions

### `FunctionName`

\```cpp
T FunctionName<T>(T param, uint count)
\```

Brief description.

| Parameter | Description |
|-----------|-------------|
| `param` | What it represents |
| `count` | What it controls |
| **Returns** | What comes back |
```

---

## Visibility

- **`public`** — part of the VEKL API, accessible to importers
- **No modifier** (`internal` by default) — shared within the module but
  hidden from importers. Use for helpers that other implementing files call
  but users shouldn't access directly (e.g., `AddressClamp`, `Hash2`, `Grad`)
- **Never use `private`** on free functions — it restricts visibility to the
  same struct, which is meaningless for module-level functions

---

## Pixel Format Handling

All kernel math operates in **`float4` RGBA space**. Format-specific logic
(storage decode, layout conversion, address modes) lives in
`texture/format.slang` and is encapsulated by `TextureView`/`RWTextureView`.

Never write format-conditional code in effect shaders. If you find yourself
writing `if (storage == ...)` in a kernel, use `TextureView` methods instead.

---

## Testing Changes

After modifying any `.slang` file, validate compilation:

```bash
slangc -I /path/to/vekl -target cpp -target metal vekl/vekl.slang -o /dev/null
```

This checks that the module resolves and compiles for both CPU and Metal
targets. If you have a CUDA environment, add `-target cuda` as well.

---

## Commit Messages

- Prefix with the concern name: `texture: add bicubic sampling`, `noise: split perlin from fbm`
- For cross-cutting changes: `module: reorganize into concern-based structure`
- Keep the summary under 72 characters
