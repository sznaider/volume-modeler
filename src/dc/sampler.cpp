#include <config.h>

#include <sstream>

#include "sampler.h"

#include "scene/scene.h"
#include "scene/chunk.h"
#include "scene/brush.h"

#include "compute/interop.h"

using namespace std;
namespace vm {
namespace dc {
namespace {
vector<pair<string, Brush::Id>> supported_brushes() {
    return {
        { "BRUSH_BALL", Brush::Id::Ball },
        { "BRUSH_CUBE", Brush::Id::Cube }
    };
}
} // namespace

Sampler::Sampler(const shared_ptr<ComputeContext> &compute_ctx)
        : m_compute_ctx(compute_ctx) {

    for (const auto &supported_brush : supported_brushes()) {
        auto program = compute::program::create_with_source_file(
                "media/kernels/samplers.cl", compute_ctx->context);
        ostringstream os;
        os << " -cl-mad-enable";
        os << " -cl-single-precision-constant";
        os << " -cl-fast-relaxed-math";
        os << " -D" << supported_brush.first;
        program.build(os.str());

        m_sdf_samplers.at(static_cast<size_t>(supported_brush.second)).sampler =
                program.create_kernel("sample");
        m_sdf_samplers.at(static_cast<size_t>(supported_brush.second)).updater =
                program.create_kernel("update_edges");
    }
}

compute::event Sampler::sample(const shared_ptr<Chunk> &chunk,
                               const Brush &brush,
                               Operation operation) {
    compute::kernel &sampler =
            m_sdf_samplers.at(static_cast<size_t>(brush.id())).sampler;
    sampler.set_arg(0, chunk->samples);
    sampler.set_arg(1, chunk->samples);
    sampler.set_arg(2, static_cast<cl_int>(operation));
    sampler.set_arg(3, Scene::get_chunk_origin(chunk->coord));
    sampler.set_arg(4, brush.get_origin());
    sampler.set_arg(5, 0.5f * brush.get_scale());
    sampler.set_arg(6, brush.get_rotation());

    const size_t N = VM_CHUNK_SIZE + 3;
    lock_guard<mutex> chunk_lock(chunk->lock);
    lock_guard<mutex> queue_lock(m_compute_ctx->queue_mutex);
    compute::event event = m_compute_ctx->queue.enqueue_nd_range_kernel(
            sampler, 3, nullptr,
            compute::dim(N + 1, N + 1, N + 1).data(),
            compute::dim(4, 4, 4).data());
    m_compute_ctx->queue.flush();
    return event;
}

} // namespace dc
} // namespace vm
