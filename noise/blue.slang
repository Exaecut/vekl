implementing vekl;

// Screen-space blue-noise dither. Unlike white noise (`Hash21`), a blue-noise
// pattern concentrates its energy in the high frequencies: there is no
// low-frequency clumping, so a downstream blur — or the eye at normal viewing
// distance — averages the grain away far more cleanly. Use it to break up
// banding in iterative samplers (sweeps, ray marches) where evenly-spaced taps
// would otherwise quantise into visible rings or steps.
//
// `frame` is timeline-locked by the caller, not render-generation-locked: pass
// a value derived from clip time so a paused frame keeps a *static* grain (no
// flicker on still frames) while footage gets evolving grain during playback.

/// Interleaved Gradient Noise (Jimenez, 2014). One `frac` of a dot product
/// yields a blue-noise-like value in `[0, 1)` from an integer pixel coordinate.
/// The constants are the published IGN tuning — they are what give the pattern
/// its spectral properties, so do not round or "simplify" them.
public float BlueNoise(float2 px, uint frame)
{
    // Golden-ratio (≈1.618 × √2 grid step) per-frame shift decorrelates
    // successive frames while staying low-discrepancy, so the animated
    // sequence never clumps back into white noise.
    float2 p = px + 5.588238 * float(frame);
    return frac(52.9829189 * frac(dot(p, float2(0.06711056, 0.00583715))));
}

/// Zero-centred blue noise in `(-0.5, +0.5]`, for symmetric jitter offsets.
public float BlueNoiseSigned(float2 px, uint frame)
{
    return BlueNoise(px, frame) - 0.5;
}
