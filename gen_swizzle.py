"""Generate vekl/common/types/swizzle.h with flat enumeration macros.

IntelliSense's preprocessor cannot handle callback-style macro patterns
(passing macro X as argument to macro Y). This script generates fully
flattened macros where each VEKL_SWIZZLE{N}_{COMPONENTS} directly
enumerates all X(rank, lanes, name, a, b, c, d) invocations.
"""

from itertools import product

def swizzles(rank, components):
    """Generate all rank-length swizzles from [(name, index), ...]."""
    results = []
    for combo in product(components, repeat=rank):
        name = "".join(c[0] for c in combo)
        indices = [c[1] for c in combo]
        padded = indices + [0] * (4 - rank)
        results.append((name, padded))
    return results

def format_flat_macro(rank, components_list, comp_name):
    """Format a flat enumeration macro."""
    combos = swizzles(rank, components_list)
    comp_upper = comp_name.upper()
    macro_name = f"VEKL_SWIZZLE{rank}_{comp_upper}"
    entries = []
    for name, idx in combos:
        entries.append(f"X({rank},L,{name},{idx[0]},{idx[1]},{idx[2]},{idx[3]})")
    body = " ".join(entries)
    return f"#define {macro_name}(X, L) {body}"

COMPONENTS = {
    'xy':   [('x', 0), ('y', 1)],
    'xyz':  [('x', 0), ('y', 1), ('z', 2)],
    'rgb':  [('r', 0), ('g', 1), ('b', 2)],
    'xyzw': [('x', 0), ('y', 1), ('z', 2), ('w', 3)],
    'rgba': [('r', 0), ('g', 1), ('b', 2), ('a', 3)],
}

# Map vector name → (lanes, component_sets)
VECTOR_SWIZZLES = {
    'FLOAT2': (2, ['xy']),
    'FLOAT3': (3, ['xyz', 'rgb']),
    'FLOAT4': (4, ['xyzw', 'rgba']),
}

lines = []
lines.append("#pragma once")
lines.append("")
lines.append("#ifndef VEKL_SWIZZLE_H")
lines.append("#define VEKL_SWIZZLE_H")
lines.append("")
lines.append("struct vekl_float2;")
lines.append("struct vekl_float3;")
lines.append("struct vekl_float4;")
lines.append("")
lines.append("template<int Lanes, int A, int B>")
lines.append("struct vekl_swizzle2 {")
lines.append("    float vekl_storage[Lanes];")
lines.append("")
lines.append("    host device forceinline operator vekl_float2() const;")
lines.append("    host device forceinline vekl_swizzle2& operator=(vekl_float2 value);")
lines.append("};")
lines.append("")
lines.append("template<int Lanes, int A, int B, int C>")
lines.append("struct vekl_swizzle3 {")
lines.append("    float vekl_storage[Lanes];")
lines.append("")
lines.append("    host device forceinline operator vekl_float3() const;")
lines.append("    host device forceinline vekl_swizzle3& operator=(vekl_float3 value);")
lines.append("};")
lines.append("")
lines.append("template<int Lanes, int A, int B, int C, int D>")
lines.append("struct vekl_swizzle4 {")
lines.append("    float vekl_storage[Lanes];")
lines.append("")
lines.append("    host device forceinline operator vekl_float4() const;")
lines.append("    host device forceinline vekl_swizzle4& operator=(vekl_float4 value);")
lines.append("};")
lines.append("")

# Generate flat enumeration macros
for comp_name, comp_list in COMPONENTS.items():
    for rank in [2, 3, 4]:
        lines.append(format_flat_macro(rank, comp_list, comp_name))
        lines.append("")

# Generate vector swizzle macros
for vec_name, (lanes, comp_names) in VECTOR_SWIZZLES.items():
    parts = []
    for comp_name in comp_names:
        for rank in [2, 3, 4]:
            comp_upper = comp_name.upper()
            parts.append(f"VEKL_SWIZZLE{rank}_{comp_upper}(X, {lanes})")
    body = " ".join(parts)
    lines.append(f"#define VEKL_{vec_name}_SWIZZLES(X) {body}")
    lines.append("")

# Declaration macros
lines.append("#define VEKL_DECLARE_SWIZZLE(RANK, LANES, NAME, A, B, C, D) \\")
lines.append("    VEKL_DECLARE_SWIZZLE_IMPL(RANK, LANES, NAME, A, B, C, D)")
lines.append("")
lines.append("#define VEKL_DECLARE_SWIZZLE_IMPL(RANK, LANES, NAME, A, B, C, D) \\")
lines.append("    VEKL_DECLARE_SWIZZLE_##RANK(LANES, NAME, A, B, C, D)")
lines.append("")
lines.append("#define VEKL_DECLARE_SWIZZLE_2(LANES, NAME, A, B, C, D) \\")
lines.append("    vekl_swizzle2<LANES, A, B> NAME;")
lines.append("")
lines.append("#define VEKL_DECLARE_SWIZZLE_3(LANES, NAME, A, B, C, D) \\")
lines.append("    vekl_swizzle3<LANES, A, B, C> NAME;")
lines.append("")
lines.append("#define VEKL_DECLARE_SWIZZLE_4(LANES, NAME, A, B, C, D) \\")
lines.append("    vekl_swizzle4<LANES, A, B, C, D> NAME;")
lines.append("")
lines.append("#endif")
lines.append("")

with open("common/types/swizzle.h", "w", newline="\n") as f:
    f.write("\n".join(lines))

print("Generated swizzle.h successfully")
