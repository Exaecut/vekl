implementing vekl;

// Mode discriminants — must match prgpu::ui::blend_mode::BlendMode.
// Values are 0-based selected-indexes matching the `popup(V)` extractor
// in prgpu::kernel_params! and `BLEND_MODE_OPTIONS[k]` in prgpu.
public static const uint BLEND_ADD          = 0;
public static const uint BLEND_MULTIPLY     = 1;
public static const uint BLEND_SCREEN       = 2;
public static const uint BLEND_COLOR_BURN   = 3;
public static const uint BLEND_COLOR_DODGE  = 4;
public static const uint BLEND_DARKER_COLOR = 5;
public static const uint BLEND_OVERLAY      = 6;
public static const uint BLEND_DIFFERENCE   = 7;
public static const uint BLEND_SUBTRACT     = 8;
public static const uint BLEND_DIVIDE       = 9;
public static const uint BLEND_HUE          = 10;
public static const uint BLEND_SATURATION   = 11;
public static const uint BLEND_COLOR        = 12;
public static const uint BLEND_LUMINOSITY   = 13;

/// Apply blend mode `mode` to (base, blend). Unknown modes return `blend`
/// unchanged (pass-through). Strength / alpha mixing is the caller's
/// responsibility — fade between `base` and the returned value with a
/// user-facing strength slider.
public float3 BlendApply(uint mode, float3 base, float3 blend)
{
    switch (mode)
    {
        case BLEND_ADD:          return BlendAdd(base, blend);
        case BLEND_MULTIPLY:     return BlendMultiply(base, blend);
        case BLEND_SCREEN:       return BlendScreen(base, blend);
        case BLEND_COLOR_BURN:   return BlendColorBurn(base, blend);
        case BLEND_COLOR_DODGE:  return BlendColorDodge(base, blend);
        case BLEND_DARKER_COLOR: return BlendDarkerColor(base, blend);
        case BLEND_OVERLAY:      return BlendOverlay(base, blend);
        case BLEND_DIFFERENCE:   return BlendDifference(base, blend);
        case BLEND_SUBTRACT:     return BlendSubtract(base, blend);
        case BLEND_DIVIDE:       return BlendDivide(base, blend);
        case BLEND_HUE:          return BlendHue(base, blend);
        case BLEND_SATURATION:   return BlendSaturation(base, blend);
        case BLEND_COLOR:        return BlendColor(base, blend);
        case BLEND_LUMINOSITY:   return BlendLuminosity(base, blend);
        default:                 return blend;
    }
}
