#ifndef UTILS_H
#define UTILS_H

const sampler_t nearest_sampler = CLK_NORMALIZED_COORDS_FALSE
                                  | CLK_ADDRESS_CLAMP_TO_EDGE
                                  | CLK_FILTER_NEAREST;

#define CONCAT3(x, y, z) \
    x ## y ## z

#define FUNCTION_NAME_CONCAT(prefix, type) \
    CONCAT3(prefix, _, type)

#define MAKE_IMPL(prefix, type) \
    FUNCTION_NAME_CONCAT(prefix, type)

float3 vertex_at(int x, int y, int z, float3 chunk_origin) {
    // OPTIMIZATION: If chunk_origin were chunk's minimal point, then the coordinates
    // of the vertex would be obtainable via single MAD: chunk_origin + VOXEL_SIZE * xyz
    const float3 half_dim = 0.5f * (float3)(VM_CHUNK_SIZE + 2,
                                            VM_CHUNK_SIZE + 2,
                                            VM_CHUNK_SIZE + 2);
    return (float)(VM_VOXEL_SIZE) * ((float3)(x, y, z) - half_dim) + chunk_origin;
}

typedef struct {
    float3 row0;
    float3 row1;
    float3 row2;
} mat3;

typedef struct {
    float4 row0;
    float4 row1;
    float4 row2;
    float4 row3;
} mat4;

float3 mul_mat3_float3(mat3 A, float3 v) {
    return (float3)(dot(A.row0, v), dot(A.row1, v), dot(A.row2, v));
}

float4 mul_mat4_float4(mat4 A, float4 v) {
    return (float4)(dot(A.row0, v), dot(A.row1, v), dot(A.row2, v), dot(A.row3, v));
}

#endif // UTILS_H
