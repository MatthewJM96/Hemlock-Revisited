#ifndef __hemlock_ai_pathing_voxel_h
#define __hemlock_ai_pathing_voxel_h

#include "voxel/coordinate_system.h"

namespace hemlock {
    namespace ai {
        namespace pathing {
            namespace Voxel {
                // TODO(Matthew): We want to handle general cases of movement. I.e. some arbitrary combination of movement capabilities such as
                //                stepping, jumping up, jumping down, teleporting, flying, gliding, etc. This likely means not providing the
                //                candidate start/end coordinates as opposed to just the step from coordinate, and then have evaluation
                //                strategies determine the field of candidates they can evaluate (perhaps make that two-stage?). We likely
                //                need some method of letting the evaluators know "agent desires outcome X" or "agent is in state Y", this
                //                could be achieved if we define a generic AI agent interface and have one passed in. That said, this needs to
                //                capture concrete ideas like "agent X is following agent Y", and also not be exorbitant - minimising
                //                recomputation where possible. Pheromone map sharing MAY be a sufficient solution for this, but implies
                //                some gameplay limitations as this is sharing knowledge between agents. Inevitably test, but keep in mind
                //                caching opportunities such as in identifying valid blocks to step to (perhaps cahce by building up graphs,
                //                one per evaluation strategy, that can be referred to and thus skip the step search stage of pathing).

                /**
                 * @brief Defines a struct whose opeartor() evaluates a coordinate for being a valid
                 * location to step to.
                 */
                template <typename StrategyCandidate>
                concept StepEvaluationStrategy = requires (
                        StrategyCandidate s,
                      hmem::Handle<Chunk> c,
                       ChunkWorldPosition p
                ) {
                    { s.operator()(c, p, p) } -> std::same_as<bool>;
                };

                template <StepEvaluationStrategy... Strategies>
                bool is_valid_step(BlockWorldPosition);

                template <StepEvaluationStrategy... Strategies>
                void find_valid_steps(        BlockWorldPosition step_from,
                                              BlockWorldPosition candidate_start,
                                              BlockWorldPosition candidate_end,
                            OUT std::vector<BlockWorldPosition>& valid_steps,
                                         hmem::Handle<ChunkGrid> chunk_grid     );
            }
        }
    }
}
namespace hai = hemlock::ai;

#endif // __hemlock_ai_pathing_voxel_h
