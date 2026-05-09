# Color — Blend

Blending modes for compositing pixel colors.

Source: [`color/blend/add.slang`](../../color/blend/add.slang), [`color/blend/multiply.slang`](../../color/blend/multiply.slang), [`color/blend/dispatch.slang`](../../color/blend/dispatch.slang)

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

### `BlendApply`

```cpp
float3 BlendApply(uint mode, float3 base, float3 blend)
```

Runtime dispatcher that selects one of the `Blend*` functions based on
`mode`. Unknown modes return `blend` unchanged (pass-through); fade between
`base` and the result with a user-facing strength slider rather than a
dedicated "no blend" mode.

## Mode discriminants

The `BLEND_*` constants are **0-based selected-indexes**. The same number
shows up in three other places — pick whichever reads best at the call
site:

* The `prgpu::ui::BlendMode` Rust enum (`BlendMode::Multiply as u32 == 1`).
* The popup option list `prgpu::ui::BLEND_MODE_OPTIONS[k]`
  (`BLEND_MODE_OPTIONS[1] == "Multiply"`).
* The `u32` your kernel reads via `popup(V)` in `prgpu::kernel_params!`.

There is no `Normal` sentinel — express "no blend / pass-through" with a
strength slider that fades `base` toward the blended result, never with a
dedicated mode value.

| Constant            | u32 | Popup option (`prgpu::ui::BLEND_MODE_OPTIONS[k]`) |
|---------------------|-----|---------------------------------------------------|
| `BLEND_ADD`         | 0   | `"Add"`                                           |
| `BLEND_MULTIPLY`    | 1   | `"Multiply"`                                      |
| `BLEND_SCREEN`      | 2   | `"Screen"`                                        |
| `BLEND_COLOR_BURN`  | 3   | `"Color Burn"`                                    |
| `BLEND_COLOR_DODGE` | 4   | `"Color Dodge"`                                   |
| `BLEND_DARKER_COLOR`| 5   | `"Darker Color"`                                  |
| `BLEND_OVERLAY`     | 6   | `"Overlay"`                                       |
| `BLEND_DIFFERENCE`  | 7   | `"Difference"`                                    |
| `BLEND_SUBTRACT`    | 8   | `"Subtract"`                                      |
| `BLEND_DIVIDE`      | 9   | `"Divide"`                                        |
| `BLEND_HUE`         | 10  | `"Hue"`                                           |
| `BLEND_SATURATION`  | 11  | `"Saturation"`                                    |
| `BLEND_COLOR`       | 12  | `"Color"`                                         |
| `BLEND_LUMINOSITY`  | 13  | `"Luminosity"`                                    |

## Example — tint with strength

```cpp
// Host-side (Rust):
kernel_params! {
    TintParams for crate::params::Params {
        tint_strength:   f32 = [float(TintStrength) / 100.0];
        tint_blend_mode: u32 = [popup(TintBlendMode)];
    }
}

// Shader — `tintBlendMode` arrives 0-based, matching BLEND_* constants:
float3 tinted   = BlendApply(params.tintBlendMode, base.rgb, tint.rgb);
float3 finalRGB = lerp(base.rgb, tinted, params.tintStrength);
```
