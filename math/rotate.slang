implementing vekl;

/// Rotate a raw 2D offset/direction vector by `angle` radians (CCW). Use this
/// for offset vectors, not centre-relative uv (see sampling::RotateUV for uv
/// rotation around 0.5 with aspect correction).
///
/// Example — spiral a dispersion offset along a sweep parameter s in [-1, 1]:
///   float2 off = Rotate2D(baseOffset, twist * s);
public float2 Rotate2D(float2 v, float angle)
{
    float c = cos(angle);
    float s = sin(angle);
    return float2(c * v.x - s * v.y, s * v.x + c * v.y);
}
