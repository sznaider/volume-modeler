#include "gtest/gtest.h"

#include <config.h>

#include "compute/context.h"

#include "dc/sampler.h"

#include "scene/brush-ball.h"
#include "scene/brush-cube.h"
#include "scene/chunk.h"

#include <glm/gtc/packing.hpp>

namespace {
struct TestContext {
    std::shared_ptr<vm::ComputeContext> compute_ctx;

    vm::Chunk chunk;
    std::vector<int16_t> cpu_samples;
    std::vector<int16_t> gpu_samples;

    TestContext()
            : compute_ctx(vm::make_compute_context())
            , chunk({ 0, 0, 0 }, compute_ctx->context, 0)
            , cpu_samples((VM_CHUNK_SIZE + 3) * (VM_CHUNK_SIZE + 3)
                                  * (VM_CHUNK_SIZE + 3),
                          2)
            , gpu_samples(cpu_samples.size(), 0) {
        const compute::short4_ fill_color(2, 2, 2, 2);
        compute_ctx->queue.enqueue_fill_image<3>(chunk.samples,
                                                 &fill_color,
                                                 compute::dim(0, 0, 0),
                                                 chunk.samples.size());
        compute_ctx->queue.flush();
        compute_ctx->queue.finish();
    }
};

glm::vec3
vertex_at(int x, int y, int z, const glm::vec3 &origin = { 0, 0, 0 }) {
    auto half_dim = 0.5f * glm::vec3(VM_CHUNK_SIZE + 3,
                                     VM_CHUNK_SIZE + 3,
                                     VM_CHUNK_SIZE + 3);
    return float(VM_VOXEL_SIZE) * (glm::vec3(x, y, z) - half_dim) + origin;
}

int16_t sign(float value) {
    if (value == 0) {
        return 0;
    } else if (value < 0) {
        return -1;
    } else {
        return +1;
    }
}

float sdf_cube(const glm::vec3 &q, const glm::vec3 &scale = { 1, 1, 1 }) {
    glm::vec3 p = glm::abs(q);
    return glm::max(p.x - scale.x, glm::max(p.y - scale.y, p.z - scale.z));
}

} // namespace

TEST(sampler, signs_match) {
    TestContext ctx{};
    const vm::BrushCube cube{};
    vm::dc::Sampler sampler(ctx.compute_ctx);
    sampler.sample(ctx.chunk, cube, vm::dc::Sampler::Operation::Add);
    ctx.compute_ctx->queue.flush();
    ctx.compute_ctx->queue.finish();

    for (size_t i = 0; i < VM_CHUNK_SIZE + 3; ++i) {
        for (size_t j = 0; j < VM_CHUNK_SIZE + 3; ++j) {
            for (size_t k = 0; k < VM_CHUNK_SIZE + 3; ++k) {
                const size_t index =
                        k + (VM_CHUNK_SIZE + 3) * (j + (VM_CHUNK_SIZE + 3) * i);
                const glm::vec3 p = vertex_at(k, j, i) - cube.get_origin();
                ctx.cpu_samples[index] = sign(
                        glm::min(ctx.cpu_samples[index],
                                 sign(sdf_cube(p, 0.5f * cube.get_scale()))));
            }
        }
    }
    ctx.compute_ctx->queue
            .enqueue_read_image<3>(ctx.chunk.samples,
                                   compute::dim(0, 0, 0),
                                   compute::dim(VM_CHUNK_SIZE + 3,
                                                VM_CHUNK_SIZE + 3,
                                                VM_CHUNK_SIZE + 3),
                                   ctx.gpu_samples.data())
            .wait();

    for (size_t i = 0; i < VM_CHUNK_SIZE + 3; ++i) {
        for (size_t j = 0; j < VM_CHUNK_SIZE + 3; ++j) {
            for (size_t k = 0; k < VM_CHUNK_SIZE + 3; ++k) {
                const size_t index =
                        k + (VM_CHUNK_SIZE + 3) * (j + (VM_CHUNK_SIZE + 3) * i);
                ASSERT_EQ(ctx.cpu_samples[index], ctx.gpu_samples[index]);
            }
        }
    }
}
