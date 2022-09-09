#ifndef __hemlock_ai_pathing_next_step_finder_hpp
#define __hemlock_ai_pathing_next_step_finder_hpp

namespace hemlock {
    namespace ai {
        namespace pathing {
            template <typename VertexCoordType>
            class NextStepFinderBase :
                public std::iterator<std::input_iterator_tag, VertexCoordType>
            {
                reference operator*() const { return last_coord;  }
                pointer  operator->() const { return &last_coord; }
            protected:
                size_t          index;
                VertexCoordType last_coord;
            };

            template <typename ...ConcreteStepFinders>
            class NextStepFinder : public NextStepFinderBase {
                
            };
        }
    }
}
namespace hai = hemlock::ai;

#endif // __hemlock_ai_pathing_next_step_finder_hpp
