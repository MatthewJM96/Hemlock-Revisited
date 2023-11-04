#ifndef __hemlock_voxel_predicate_hpp
#define __hemlock_voxel_predicate_hpp

#include "voxel/coordinate_system.h"

namespace hemlock {
    namespace voxel {
        struct Block;
        template <ChunkDecorator... Decorations>
        struct Chunk;

        /**
         * @brief Defines a struct whose opeartor() determines if a block at a
         * specified location satisfies a certain constraint.
         */
        template <typename ConstraintCandidate, typename... Decorations>
        concept ActualBlockConstraint
            = (ChunkDecorator<Decorations> && ...)
              && requires (
                  ConstraintCandidate s, BlockChunkPosition p, Chunk<Decorations...>* c
              ) {
                     {
                         s.operator()(p, c)
                         } -> std::same_as<bool>;
                 };

        /**
         * @brief Defines a struct whose opeartor() determines if a block type satisfies
         * a certain constraint.
         */
        template <typename ConstraintCandidate>
        concept IdealBlockConstraint
            = requires (ConstraintCandidate s, const Block* b) {
                  {
                      s.operator()(b)
                      } -> std::same_as<bool>;
              };

        /**
         * @brief Defines a struct whose opeartor() determines if two blocks at
         * specified locations satisfy a certain constraint.
         */
        template <typename ComparatorCandidate, typename... Decorations>
        concept ActualBlockComparator
            = (ChunkDecorator<Decorations> && ...)
              && requires (
                  ComparatorCandidate s, BlockChunkPosition p, Chunk<Decorations...>* c
              ) {
                     {
                         s.operator()(p, p, c)
                         } -> std::same_as<bool>;
                 };

        /**
         * @brief Defines a struct whose opeartor() determines if a block at
         * a specific location satisfies a constraint defined by a specific block type.
         *
         * The first block pointer is to the comparator block, while the second
         * is the actual block at the specified position within the specified
         * chunk.
         */
        template <typename ComparatorCandidate, typename... Decorations>
        concept IdealBlockComparator = (ChunkDecorator<Decorations> && ...)
                                       && requires (
                                           ComparatorCandidate    s,
                                           const Block*           b,
                                           BlockChunkPosition     p,
                                           Chunk<Decorations...>* c
                                       ) {
                                              {
                                                  s.operator()(b, b, p, c)
                                                  } -> std::same_as<bool>;
                                          };
    }  // namespace voxel
}  // namespace hemlock
namespace hvox = hemlock::voxel;

#endif  // __hemlock_voxel_predicate_hpp
