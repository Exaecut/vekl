implementing vekl;

// Multi-tap filters must run in premultiplied space; summing N straight-alpha
// samples drags RGB toward zero (alpha-edge darkening). `VisibleEnergy` is
// the alpha-weighted RGB: a fully transparent pixel contributes zero
// regardless of its straight RGB.

public float4 ToPremul(float4 straight)
{
    return float4(straight.rgb * straight.a, straight.a);
}

// Returns zero RGB for fully-transparent pixels; division by alpha would
// yield meaningless values.
public float4 FromPremul(float4 premul)
{
    float a = premul.a;
    return (a > 1e-5)
        ? float4(premul.rgb / a, a)
        : float4(0.0, 0.0, 0.0, a);
}

public float3 VisibleEnergy(float3 straight_rgb, float alpha)
{
    return straight_rgb * alpha;
}

public float Luma709(float3 rgb)
{
    return dot(rgb, float3(0.2126, 0.7152, 0.0722));
}
