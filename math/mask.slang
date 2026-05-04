implementing vekl;

public float DistanceMask(float2 uv, float2 center, float mn, float mx)
{
    float2 d = uv - center;
    float norm = sqrt(dot(d, d)) / (sqrt(2.0) * 0.5);
    return smoothstep(mn, mx, norm);
}
