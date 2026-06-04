# Math - Rotate

Rotate a raw 2D offset/direction vector by an angle in radians (CCW).

Source: [`math/rotate.slang`](../../math/rotate.slang)

---

## How to use

Use `Rotate2D` for offset/direction vectors. For centre-relative uv rotation
around `0.5` with aspect correction, use `sampling::RotateUV` instead.

---

## Functions

### `Rotate2D`

```cpp
float2 Rotate2D(float2 v, float angle)
```

Returns `v` rotated counter-clockwise by `angle` radians.

```cpp
// Spiral a dispersion offset along a sweep parameter s in [-1, 1]:
float2 off = Rotate2D(baseOffset, twist * s);
```
