#ifndef __hemlock_voxel_predicate_hpp
#define __hemlock_voxel_predicate_hpp

#include "voxel/coordinate_system.h"

namespace hemlock {
    namespace voxel {
        struct Voxel;
        struct Chunk;

        /**
         * @brief Defines a struct whose opeartor() determines if a voxel at a
         * specified location satisfies a certain constraint.
         */
        template <typename ConstraintCandidate>
        concept ActualVoxelConstraint
            = requires (ConstraintCandidate s, VoxelChunkPosition p, Chunk* c) {
                  {
                      s.operator()(p, c)
                      } -> std::same_as<bool>;
              };

        /**
         * @brief Defines a struct whose opeartor() determines if a voxel type satisfies
         * a certain constraint.
         */
        template <typename ConstraintCandidate>
        concept IdealVoxelConstraint
            = requires (ConstraintCandidate s, const Voxel* b) {
                  {
                      s.operator()(b)
                      } -> std::same_as<bool>;
              };

        /**
         * @brief Defines a struct whose opeartor() determines if two voxels at
         * specified locations satisfy a certain constraint.
         */
        template <typename ComparatorCandidate>
        concept ActualVoxelComparator
            = requires (ComparatorCandidate s, VoxelChunkPosition p, Chunk* c) {
                  {
                      s.operator()(p, p, c)
                      } -> std::same_as<bool>;
              };

        /**
         * @brief Defines a struct whose opeartor() determines if a voxel at
         * a specific location satisfies a constraint defined by a specific voxel type.
         *
         * The first voxel pointer is to the comparator voxel, while the second
         * is the actual voxel at the specified position within the specified
         * chunk.
         */
        template <typename ComparatorCandidate>
        concept IdealVoxelComparator = requires (
            ComparatorCandidate s, const Voxel* b, VoxelChunkPosition p, Chunk* c
        ) {
                                           {
                                               s.operator()(b, b, p, c)
                                               } -> std::same_as<bool>;
                                       };
    }  // namespace voxel
}  // namespace hemlock
namespace hvox = hemlock::voxel;

#endif  // __hemlock_voxel_predicate_hpp
