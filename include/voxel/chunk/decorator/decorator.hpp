#ifndef __hemlock_voxel_chunk_decorator_decorator_hpp
#define __hemlock_voxel_chunk_decorator_decorator_hpp

namespace hemlock {
    namespace voxel {
        template <typename ChunkDecoratorCandidate>
        concept ChunkDecorator = requires (ChunkDecoratorCandidate d) {
                                     typename ChunkDecoratorCandidate::type;
                                     {
                                         d.attach_to_events()
                                         } -> std::same_as<void>;
                                 };

        // TODO(Matthew): Arguments...
        template <typename ChunkMeshDecoratorCandidate>
        concept ChunkMeshDecorator = ChunkDecorator<ChunkMeshDecoratorCandidate>
                                     && requires (ChunkMeshDecoratorCandidate d) {
                                            {
                                                d.add_mesh()
                                                } -> std::same_as<void>;
                                            {
                                                d.remesh()
                                                } -> std::same_as<void>;
                                        };
    }  // namespace voxel
}  // namespace hemlock
namespace hvox = hemlock::voxel;

#endif  // __hemlock_voxel_chunk_decorator_decorator_hpp
