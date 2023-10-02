#ifndef __hemlock_algorithm_graph_loop_locator_hpp
#define __hemlock_algorithm_graph_loop_locator_hpp

namespace hemlock {
    namespace algorithm {
        /**
         * @brief When attached to a DFS traversal, this visitor tracks paths traversed
         * through a graph, determining to first-order all loops present in the graph.
         * If this locator finds no loops, this means no loop is indeed present in the
         * graph, however if it does return loops these are non-exhaustive. Any loops
         * that are composed of parts of multiple loops adjoining by a vertex will
         * not be discovered. Thus, the utility of this locator depends on such
         * "higher-order" loops not being needed to be known about. An example of this
         * is in achieving an orderable set of mods, where the first-order loops can be
         * used to determine a reasonable minimum (or reasonably close to softest) set
         * of dependencies that must be broken in order to be able to achieve
         * orderability.
         *   As an example, consider a graph:
         *      0 ---> 1 <--- 2
         *       \    / \   /\
         *        \  /   \  /
         *        \/\/   \//
         *         4 ---> 3
         *         /\    /
         *          \   /
         *           \ \/
         *            6
         *   performing DFS with this locator with vertex 0 as the root, the loops:
         *     1 -> 3 -> 2 -> 1
         *     3 -> 6 -> 4 -> 3
         *   will consistently be discovered, while the loop:
         *     1 -> 4 -> 3 -> 2 -> 1
         *   will not.
         *
         * @tparam Graph the type of the graph on which this visitor will be visiting.
         */
        template <typename Graph>
        class LoopLocator : public boost::default_dfs_visitor {
        public:
            using Edge   = boost::graph_traits<Graph>::edge_descriptor;
            using Vertex = boost::graph_traits<Graph>::vertex_descriptor;

            using Edges         = std::vector<Edge>;
            using Path          = Edges;
            using BackEdges     = std::unordered_map<Vertex, Edges>;
            using Loops         = hmem::Handle<std::vector<std::vector<Edge>>>;
            using LoopVertexMap = std::unordered_map<Vertex, std::vector<size_t>>;
            using LoopsVisited  = std::vector<size_t>;

            LoopLocator(){ /* Empty. */ };
            LoopLocator(const LoopLocator& rhs);
            LoopLocator(LoopLocator&& rhs);
            ~LoopLocator(){ /* Empty. */ };

            void init(const Loops& loops);
            void dispose();

            /**
             * @brief Called whenever DFS iterates over a tree edge and the edge is not
             * determined to be a back edge. In order that we may determine the path
             * constituting a given loop within the graph being searched, we maintain
             * the edges traversed by the DFS algorithm, adding those edges here. Edges
             * are popped in finish_vertex.
             *
             * @param edge The edge that has just been traversed, making up part of the
             * DFS tree.
             * @param graph The graph being traversed.
             */
            void tree_edge(Edge edge, const Graph& graph);

            /**
             * @brief Called whenever DFS iterates a back edge. Such a traversal
             * identifies a loop which is here stored in the set of loops so far
             * identified.
             *
             * @param edge The edge that has just been traversed, constituting a back
             * edge.
             * @param graph The graph being traversed.
             */
            void back_edge(Edge edge, const Graph& graph);

            /**
             * @brief Called whenever DFS iterates a forward or cross edge. In order
             * that we detect all loops whether connected or disconnected from one
             * another, we here inspect whether the edge we've just iterated joins an
             * existing loop.
             *
             * @param edge The edge that has just been traversed, constituting either a
             * forward or cross edge.
             * @param graph The graph being traversed.
             */
            void forward_or_cross_edge(Edge edge, const Graph& graph);

            /**
             * @brief Called before DFS traversal to denote the root vertex on which the
             * traversal will be performed. We use this to capture cases where a loop
             * travels through this root vertex, as some times there'll be no edge to
             * reference.
             *
             * @param vertex The vertex that the DFS traversal will be rooted on.
             * @param graph The graph being traversed.
             */
            void start_vertex(Vertex vertex, const Graph& graph);

            /**
             * @brief Called whenever DFS has exhausted outbound travel from the
             * provided vertex. This tells us that we should pop the edge leading to
             * that vertex within the current path.
             *
             * @param vertex The vertex that has just been exhausted.
             * @param graph The graph being traversed.
             */
            void finish_vertex(Vertex vertex, const Graph& graph);
        protected:
            Vertex        m_root_vertex;
            Path          m_curr_path;
            BackEdges     m_back_edges;
            Loops         m_loops;
            LoopVertexMap m_loop_vertex_map;
            LoopsVisited  m_loops_visited;
        };
    }  // namespace algorithm
}  // namespace hemlock
namespace halgo = hemlock::algorithm;

#include "loop_locator.inl"

#endif  // __hemlock_algorithm_graph_loop_locator_hpp
