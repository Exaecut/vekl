# VEKL Casing Rules

VEKL follows C++ / shader-style casing conventions.

## General rules

| Element             |             Casing | Example                        |
| ------------------- | -----------------: | ------------------------------ |
| Functions           |        `camelCase` | `samplePixel()`                |
| Variables           |        `camelCase` | `inputColor`                   |
| Structs / types     |       `PascalCase` | `BlurParams`                   |
| Constants           | `UPPER_SNAKE_CASE` | `MAX_RADIUS`                   |
| Shader entry points |        `camelCase` | `gaussianBlurKernel`           |
| Swizzles            |          lowercase | `.xy`, `.rgba`, `.bgra`        |
| File names          |       `snake_case` | `gaussian_blur.vekl`           |
| Vendor names        |      vendor casing | `Metal`, `CUDA`, `WGSL`, `MSL` |

## Macros

Function-like macros must use `snake_case`.

```vekl
define_backend_type!(...)
declare_vector_type!(...)
```

Constant-like macros and compile-time flags must use `UPPER_SNAKE_CASE`.

```vekl
ENABLE_FAST_MATH
MAX_VECTOR_WIDTH
```

Code-generation macros, including X-Macros, must use `UPPER_SNAKE_CASE`.

```vekl
DECLARE_VECTOR_TYPES(...)
EXPAND_SWIZZLE_SET(...)
```

## Examples

```vekl
struct BlurParams {
    float radius;
    float intensity;
};

const uint MAX_RADIUS = 64;

float4 samplePixel(Texture2D inputTexture, float2 uv) {
    return inputTexture.sample(uv);
}

kernel void gaussianBlurKernel() {
    float4 inputColor = samplePixel(sourceTexture, uv);
}
```
