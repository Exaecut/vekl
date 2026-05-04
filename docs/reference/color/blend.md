# Color — Blend

Blending modes for compositing pixel colors.

Source: [`color/blend/add.slang`](../../color/blend/add.slang), [`color/blend/multiply.slang`](../../color/blend/multiply.slang)

---

## Functions

All blend functions are generic over type `T : IFloat`, supporting `float`,
`float2`, `float3`, `float4`, `half`, and their vector types without
overloads.

### `BlendAdd`

```cpp
T BlendAdd<T>(T base, T blend) where T : IFloat
```

Additive blend. Use for light leaks, glow, and additive light effects.

### `BlendMultiply`

```cpp
T BlendMultiply<T>(T base, T blend) where T : IFloat
```

Multiply blend. Use for shadows, vignettes, and darkening.
Result is always darker than either input.
