# Noise — Hash

Cheap hash-based value noise. Pair with stylistic effects (VHS, CRT, film
grain) where a slightly aliased look is desirable.

Source: [`noise/hash.slang`](../../../noise/hash.slang)

---

## Public Functions

### `Hash21`

```cpp
float Hash21(float2 p)
```

Scalar pseudo-random value in `[0, 1)` derived from a 2D coordinate.

| Parameter | Description |
|-----------|-------------|
| `p` | Sample coordinate |
| **Returns** | Pseudo-random scalar in `[0, 1)` |

### `HashValueNoise`

```cpp
float HashValueNoise(float2 p, float2 lattice)
```

Smoothstep-interpolated value noise on an integer lattice. `lattice`
controls cell grid density (e.g. `float2(8, 8)` → 8×8 cells across
`p ∈ [0, 1)`). Returns scalar approximately in `[0, 1)`.

### `HashFbm`

```cpp
float HashFbm(float2 p, int octaves)
```

Stacks `octaves` scales of [`HashValueNoise`] at lattice `2·2^i` with
amplitude `1/2^i`. Output range ≈ `[0, 1]`. Use ~4–8 octaves for VHS-style
tape noise; more octaves add finer grit but diminishing visible detail.

| Parameter | Description |
|-----------|-------------|
| `p` | Sample coordinate |
| `octaves` | Number of octaves (1-based, loops `i = 1..octaves`) |
| **Returns** | Multi-octave noise scalar |
