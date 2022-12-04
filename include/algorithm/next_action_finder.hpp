#ifndef __hemlock_algorithm_next_action_finder_hpp
#define __hemlock_algorithm_next_action_finder_hpp

#include "algorithm/graph.hpp"

namespace hemlock {
    namespace algorithm {
        template <typename ActionType>
        class INextActionIterator {
        public:
            using iterator_category = std::input_iterator_tag;
            using value_type        = ActionType;
            using difference_type   = ptrdiff_t;
            using pointer           = ActionType*;
            using reference         = ActionType&;

            virtual INextActionIterator<ActionType>& operator++()    = 0;
            virtual INextActionIterator<ActionType>& operator++(i32) = 0;
        };

        // TODO(Matthew): Generally need to have same constructor args in
        //                derived classes, but don't want to enforce stack.
        template <typename Iterator>
        class INextActionFinder {
        public:
            virtual Iterator       begin()       = 0;
            virtual const Iterator begin() const = 0;
            virtual Iterator       end()         = 0;
            virtual const Iterator end() const   = 0;
        };

        // template <typename ActionType, typename ...ConcreteStepFinders>
        // class NextActionMultiIterator : public INextActionIterator<ActionType> {
        // public:
        //     NextActionMultiIterator() : m_action_finder_idx(-1) {}

        //     reference operator*() const { return m_last_action;  }
        //     pointer  operator->() const { return &m_last_action; }

        //     INextActionIterator<ActionType>& operator++()    final;
        //     INextActionIterator<ActionType>& operator++(i32) final;
        // protected:
        //     //INextActionIterator<ActionType> m_it;
        //     i32 m_action_finder_idx;
        // };

        // template <typename ActionType, typename ...ConcreteStepFinders>
        // class NextActionMultiFinder :
        //     public INextActionFinder<NextActionMultiIterator<ActionType,
        //     ConcreteStepFinders...>>
        // {
        // public:
        //           NextActionMultiIterator<ActionType, ConcreteStepFinders...>
        //           begin()       final;
        //     const NextActionMultiIterator<ActionType, ConcreteStepFinders...>
        //     begin() const final;
        //           NextActionMultiIterator<ActionType, ConcreteStepFinders...> end()
        //           final;
        //     const NextActionMultiIterator<ActionType, ConcreteStepFinders...> end()
        //     const final;
        // };

        template <typename ActionType>
        using NextActionFromGraphIterator
            = boost::graph_traits<Graph<ActionType>>::out_edge_iterator;

        template <typename ActionType>
        class NextActionFromGraphFinder :
            public INextActionFinder<NextActionFromGraphIterator<ActionType>> {
        public:
            NextActionFromGraphFinder(
                VertexDescriptor<ActionType> vertex,
                const GraphMap<ActionType>&  graph_map
            ) :
                m_iterator_pair(boost::out_edges(vertex, graph_map.graph)
                ) { /* Empty. */
            }

            NextActionFromGraphIterator<ActionType>       begin() final;
            const NextActionFromGraphIterator<ActionType> begin() const final;
            NextActionFromGraphIterator<ActionType>       end() final;
            const NextActionFromGraphIterator<ActionType> end() const final;
        protected:
            std::pair<
                typename boost::graph_traits<Graph<ActionType>>::out_edge_iterator,
                typename boost::graph_traits<Graph<ActionType>>::out_edge_iterator>
                m_iterator_pair;
        };
    }  // namespace algorithm
}  // namespace hemlock
namespace halgo = hemlock::algorithm;

#include "algorithm/next_action_finder.inl"

#endif  // __hemlock_algorithm_next_action_finder_hpp
