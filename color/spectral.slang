implementing vekl;

// Wavelength-band weights for spectral dispersion / prism effects. Maps a sweep
// coordinate s in [0,1] to per-channel response that ramps Red -> Green -> Blue,
// so accumulating source taps along a line weighted by these bands integrates a
// continuous prism smear instead of a 2-channel split. The three triangular
// bands sum to 1 at every s, so normalizing each channel by its summed weight
// reproduces a flat field unchanged (no tint, no energy loss).
//
// Example — sweep N taps from -offset to +offset, red fringe at -offset, blue at +offset:
//   float3 num = float3(0), den = float3(0);
//   for (uint i = 0; i < n; ++i) {
//       float s = float(i) / float(n - 1);
//       float3 w = SpectralWeights(s);
//       float3 c = src.SampleLinear(uv0 + offset * (s * 2.0 - 1.0)).rgb;
//       num += c * w; den += w;
//   }
//   float3 prism = num / max(den, float3(1e-6));

public float3 SpectralWeights(float s)
{
    float t = clamp(s, 0.0, 1.0);
    float r = max(1.0 - 2.0 * t, 0.0);
    float g = 1.0 - abs(2.0 * t - 1.0);
    float b = max(2.0 * t - 1.0, 0.0);
    return float3(r, g, b);
}
