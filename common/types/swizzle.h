#pragma once

#ifndef VEKL_SWIZZLE_H
#define VEKL_SWIZZLE_H

struct vekl_float2;
struct vekl_float3;
struct vekl_float4;

template<int Lanes, int A, int B>
struct vekl_swizzle2 {
    float vekl_storage[Lanes];

    host device forceinline operator vekl_float2() const;
    host device forceinline vekl_swizzle2& operator=(vekl_float2 value);
};

template<int Lanes, int A, int B, int C>
struct vekl_swizzle3 {
    float vekl_storage[Lanes];

    host device forceinline operator vekl_float3() const;
    host device forceinline vekl_swizzle3& operator=(vekl_float3 value);
};

template<int Lanes, int A, int B, int C, int D>
struct vekl_swizzle4 {
    float vekl_storage[Lanes];

    host device forceinline operator vekl_float4() const;
    host device forceinline vekl_swizzle4& operator=(vekl_float4 value);
};

#define VEKL_SWIZZLE_COMPONENTS_XY_A(CB, ...) \
    CB(__VA_ARGS__, x, 0) \
    CB(__VA_ARGS__, y, 1)
#define VEKL_SWIZZLE_COMPONENTS_XY_B(CB, ...) \
    CB(__VA_ARGS__, x, 0) \
    CB(__VA_ARGS__, y, 1)
#define VEKL_SWIZZLE_COMPONENTS_XY_C(CB, ...) \
    CB(__VA_ARGS__, x, 0) \
    CB(__VA_ARGS__, y, 1)
#define VEKL_SWIZZLE_COMPONENTS_XY_D(CB, ...) \
    CB(__VA_ARGS__, x, 0) \
    CB(__VA_ARGS__, y, 1)

#define VEKL_SWIZZLE_COMPONENTS_RGB_A(CB, ...) \
    CB(__VA_ARGS__, r, 0) \
    CB(__VA_ARGS__, g, 1) \
    CB(__VA_ARGS__, b, 2)
#define VEKL_SWIZZLE_COMPONENTS_RGB_B(CB, ...) \
    CB(__VA_ARGS__, r, 0) \
    CB(__VA_ARGS__, g, 1) \
    CB(__VA_ARGS__, b, 2)
#define VEKL_SWIZZLE_COMPONENTS_RGB_C(CB, ...) \
    CB(__VA_ARGS__, r, 0) \
    CB(__VA_ARGS__, g, 1) \
    CB(__VA_ARGS__, b, 2)
#define VEKL_SWIZZLE_COMPONENTS_RGB_D(CB, ...) \
    CB(__VA_ARGS__, r, 0) \
    CB(__VA_ARGS__, g, 1) \
    CB(__VA_ARGS__, b, 2)

#define VEKL_SWIZZLE_COMPONENTS_RGBA_A(CB, ...) \
    CB(__VA_ARGS__, r, 0) \
    CB(__VA_ARGS__, g, 1) \
    CB(__VA_ARGS__, b, 2) \
    CB(__VA_ARGS__, a, 3)
#define VEKL_SWIZZLE_COMPONENTS_RGBA_B(CB, ...) \
    CB(__VA_ARGS__, r, 0) \
    CB(__VA_ARGS__, g, 1) \
    CB(__VA_ARGS__, b, 2) \
    CB(__VA_ARGS__, a, 3)
#define VEKL_SWIZZLE_COMPONENTS_RGBA_C(CB, ...) \
    CB(__VA_ARGS__, r, 0) \
    CB(__VA_ARGS__, g, 1) \
    CB(__VA_ARGS__, b, 2) \
    CB(__VA_ARGS__, a, 3)
#define VEKL_SWIZZLE_COMPONENTS_RGBA_D(CB, ...) \
    CB(__VA_ARGS__, r, 0) \
    CB(__VA_ARGS__, g, 1) \
    CB(__VA_ARGS__, b, 2) \
    CB(__VA_ARGS__, a, 3)

#define VEKL_SWIZZLE_COMPONENTS_XYZ_A(CB, ...) \
    CB(__VA_ARGS__, x, 0) \
    CB(__VA_ARGS__, y, 1) \
    CB(__VA_ARGS__, z, 2)
#define VEKL_SWIZZLE_COMPONENTS_XYZ_B(CB, ...) \
    CB(__VA_ARGS__, x, 0) \
    CB(__VA_ARGS__, y, 1) \
    CB(__VA_ARGS__, z, 2)
#define VEKL_SWIZZLE_COMPONENTS_XYZ_C(CB, ...) \
    CB(__VA_ARGS__, x, 0) \
    CB(__VA_ARGS__, y, 1) \
    CB(__VA_ARGS__, z, 2)
#define VEKL_SWIZZLE_COMPONENTS_XYZ_D(CB, ...) \
    CB(__VA_ARGS__, x, 0) \
    CB(__VA_ARGS__, y, 1) \
    CB(__VA_ARGS__, z, 2)

#define VEKL_SWIZZLE_COMPONENTS_XYZW_A(CB, ...) \
    CB(__VA_ARGS__, x, 0) \
    CB(__VA_ARGS__, y, 1) \
    CB(__VA_ARGS__, z, 2) \
    CB(__VA_ARGS__, w, 3)
#define VEKL_SWIZZLE_COMPONENTS_XYZW_B(CB, ...) \
    CB(__VA_ARGS__, x, 0) \
    CB(__VA_ARGS__, y, 1) \
    CB(__VA_ARGS__, z, 2) \
    CB(__VA_ARGS__, w, 3)
#define VEKL_SWIZZLE_COMPONENTS_XYZW_C(CB, ...) \
    CB(__VA_ARGS__, x, 0) \
    CB(__VA_ARGS__, y, 1) \
    CB(__VA_ARGS__, z, 2) \
    CB(__VA_ARGS__, w, 3)
#define VEKL_SWIZZLE_COMPONENTS_XYZW_D(CB, ...) \
    CB(__VA_ARGS__, x, 0) \
    CB(__VA_ARGS__, y, 1) \
    CB(__VA_ARGS__, z, 2) \
    CB(__VA_ARGS__, w, 3)

#define VEKL_SWIZZLE_COMPONENTS_LEVEL(COMPONENTS, LEVEL) \
    VEKL_SWIZZLE_COMPONENTS_LEVEL_IMPL(COMPONENTS, LEVEL)

#define VEKL_SWIZZLE_COMPONENTS_LEVEL_IMPL(COMPONENTS, LEVEL) \
    COMPONENTS##_##LEVEL

#define VEKL_SWIZZLE2_COMBOS(COMPONENTS, X, LANES) \
    VEKL_SWIZZLE_COMPONENTS_LEVEL(COMPONENTS, A)(VEKL_SWIZZLE2_A, COMPONENTS, X, LANES)

#define VEKL_SWIZZLE2_A(COMPONENTS, X, LANES, A_NAME, A_INDEX) \
    VEKL_SWIZZLE_COMPONENTS_LEVEL(COMPONENTS, B)(VEKL_SWIZZLE2_B, X, LANES, A_NAME, A_INDEX)

#define VEKL_SWIZZLE2_B(X, LANES, A_NAME, A_INDEX, B_NAME, B_INDEX) \
    X(2, LANES, A_NAME##B_NAME, A_INDEX, B_INDEX, 0, 0)

#define VEKL_SWIZZLE3_COMBOS(COMPONENTS, X, LANES) \
    VEKL_SWIZZLE_COMPONENTS_LEVEL(COMPONENTS, A)(VEKL_SWIZZLE3_A, COMPONENTS, X, LANES)

#define VEKL_SWIZZLE3_A(COMPONENTS, X, LANES, A_NAME, A_INDEX) \
    VEKL_SWIZZLE_COMPONENTS_LEVEL(COMPONENTS, B)(VEKL_SWIZZLE3_B, COMPONENTS, X, LANES, A_NAME, A_INDEX)

#define VEKL_SWIZZLE3_B(COMPONENTS, X, LANES, A_NAME, A_INDEX, B_NAME, B_INDEX) \
    VEKL_SWIZZLE_COMPONENTS_LEVEL(COMPONENTS, C)(VEKL_SWIZZLE3_C, X, LANES, A_NAME, A_INDEX, B_NAME, B_INDEX)

#define VEKL_SWIZZLE3_C(X, LANES, A_NAME, A_INDEX, B_NAME, B_INDEX, C_NAME, C_INDEX) \
    X(3, LANES, A_NAME##B_NAME##C_NAME, A_INDEX, B_INDEX, C_INDEX, 0)

#define VEKL_SWIZZLE4_COMBOS(COMPONENTS, X, LANES) \
    VEKL_SWIZZLE_COMPONENTS_LEVEL(COMPONENTS, A)(VEKL_SWIZZLE4_A, COMPONENTS, X, LANES)

#define VEKL_SWIZZLE4_A(COMPONENTS, X, LANES, A_NAME, A_INDEX) \
    VEKL_SWIZZLE_COMPONENTS_LEVEL(COMPONENTS, B)(VEKL_SWIZZLE4_B, COMPONENTS, X, LANES, A_NAME, A_INDEX)

#define VEKL_SWIZZLE4_B(COMPONENTS, X, LANES, A_NAME, A_INDEX, B_NAME, B_INDEX) \
    VEKL_SWIZZLE_COMPONENTS_LEVEL(COMPONENTS, C)(VEKL_SWIZZLE4_C, COMPONENTS, X, LANES, A_NAME, A_INDEX, B_NAME, B_INDEX)

#define VEKL_SWIZZLE4_C(COMPONENTS, X, LANES, A_NAME, A_INDEX, B_NAME, B_INDEX, C_NAME, C_INDEX) \
    VEKL_SWIZZLE_COMPONENTS_LEVEL(COMPONENTS, D)(VEKL_SWIZZLE4_D, X, LANES, A_NAME, A_INDEX, B_NAME, B_INDEX, C_NAME, C_INDEX)

#define VEKL_SWIZZLE4_D(X, LANES, A_NAME, A_INDEX, B_NAME, B_INDEX, C_NAME, C_INDEX, D_NAME, D_INDEX) \
    X(4, LANES, A_NAME##B_NAME##C_NAME##D_NAME, A_INDEX, B_INDEX, C_INDEX, D_INDEX)

#define VEKL_FLOAT2_SWIZZLES(X) \
    VEKL_SWIZZLE2_COMBOS(VEKL_SWIZZLE_COMPONENTS_XY, X, 2) \
    VEKL_SWIZZLE3_COMBOS(VEKL_SWIZZLE_COMPONENTS_XY, X, 2) \
    VEKL_SWIZZLE4_COMBOS(VEKL_SWIZZLE_COMPONENTS_XY, X, 2)

#define VEKL_FLOAT3_SWIZZLES(X) \
    VEKL_SWIZZLE2_COMBOS(VEKL_SWIZZLE_COMPONENTS_XYZ, X, 3) \
    VEKL_SWIZZLE3_COMBOS(VEKL_SWIZZLE_COMPONENTS_XYZ, X, 3) \
    VEKL_SWIZZLE4_COMBOS(VEKL_SWIZZLE_COMPONENTS_XYZ, X, 3) \
    VEKL_SWIZZLE2_COMBOS(VEKL_SWIZZLE_COMPONENTS_RGB, X, 3) \
    VEKL_SWIZZLE3_COMBOS(VEKL_SWIZZLE_COMPONENTS_RGB, X, 3) \
    VEKL_SWIZZLE4_COMBOS(VEKL_SWIZZLE_COMPONENTS_RGB, X, 3)

#define VEKL_FLOAT4_SWIZZLES(X) \
    VEKL_SWIZZLE2_COMBOS(VEKL_SWIZZLE_COMPONENTS_XYZW, X, 4) \
    VEKL_SWIZZLE3_COMBOS(VEKL_SWIZZLE_COMPONENTS_XYZW, X, 4) \
    VEKL_SWIZZLE4_COMBOS(VEKL_SWIZZLE_COMPONENTS_XYZW, X, 4) \
    VEKL_SWIZZLE2_COMBOS(VEKL_SWIZZLE_COMPONENTS_RGBA, X, 4) \
    VEKL_SWIZZLE3_COMBOS(VEKL_SWIZZLE_COMPONENTS_RGBA, X, 4) \
    VEKL_SWIZZLE4_COMBOS(VEKL_SWIZZLE_COMPONENTS_RGBA, X, 4)

#define VEKL_DECLARE_SWIZZLE(RANK, LANES, NAME, A, B, C, D) \
    VEKL_DECLARE_SWIZZLE_IMPL(RANK, LANES, NAME, A, B, C, D)

#define VEKL_DECLARE_SWIZZLE_IMPL(RANK, LANES, NAME, A, B, C, D) \
    VEKL_DECLARE_SWIZZLE_##RANK(LANES, NAME, A, B, C, D)

#define VEKL_DECLARE_SWIZZLE_2(LANES, NAME, A, B, C, D) \
    vekl_swizzle2<LANES, A, B> NAME;

#define VEKL_DECLARE_SWIZZLE_3(LANES, NAME, A, B, C, D) \
    vekl_swizzle3<LANES, A, B, C> NAME;

#define VEKL_DECLARE_SWIZZLE_4(LANES, NAME, A, B, C, D) \
    vekl_swizzle4<LANES, A, B, C, D> NAME;

#endif
