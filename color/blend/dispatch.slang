implementing vekl;

// Mode discriminants — must match prgpu::ui::blend_mode::BlendMode.
public static const uint BLEND_NORMAL       = 0;
public static const uint BLEND_ADD          = 1;
public static const uint BLEND_MULTIPLY     = 2;
public static const uint BLEND_SCREEN       = 3;
public static const uint BLEND_COLOR_BURN   = 4;
public static const uint BLEND_COLOR_DODGE  = 5;
public static const uint BLEND_DARKER_COLOR = 6;
public static const uint BLEND_OVERLAY      = 7;
public static const uint BLEND_DIFFERENCE   = 8;
public static const uint BLEND_SUBTRACT     = 9;
public static const uint BLEND_DIVIDE       = 10;
public static const uint BLEND_HUE          = 11;
public static const uint BLEND_SATURATION   = 12;
public static const uint BLEND_COLOR        = 13;
public static const uint BLEND_LUMINOSITY   = 14;

/// Apply blend mode `mode` to (base, blend). Unknown modes return `blend`
/// (Normal). Strength/alpha mixing is the caller's responsibility.
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
