#ifndef __hemlock_tests_test_mod_screen_hpp
#define __hemlock_tests_test_mod_screen_hpp

#include "app/screen_base.h"
#include "mod/metadata.h"

#include "iomanager.hpp"

#include "algorithm/graph/loop_locator.hpp"

class TestModScreen : public happ::ScreenBase {
public:
    TestModScreen() : happ::ScreenBase(), m_input_manager(nullptr) { /* Empty. */
    }

    virtual ~TestModScreen(){ /* Empty */ };

    virtual void start(hemlock::FrameTime time) override {
        happ::ScreenBase::start(time);

        std::cout << "Hey up." << std::endl;

        using Graph = boost::adjacency_list<
            boost::vecS,
            boost::vecS,
            boost::directedS,
            boost::property<boost::vertex_color_t, boost::default_color_type>>;
        using Edge   = boost::graph_traits<Graph>::edge_descriptor;
        using Vertex = boost::graph_traits<Graph>::vertex_descriptor;

        Graph g;

        std::vector<Vertex> vertices;

        for (size_t i = 0; i < 7; ++i) {
            vertices.push_back(boost::add_vertex(g));
        }

        std::vector<Edge> edges;

        edges.push_back(boost::add_edge(vertices[0], vertices[1], g).first);
        edges.push_back(boost::add_edge(vertices[0], vertices[4], g).first);
        edges.push_back(boost::add_edge(vertices[1], vertices[3], g).first);
        edges.push_back(boost::add_edge(vertices[1], vertices[1], g).first);
        edges.push_back(boost::add_edge(vertices[1], vertices[4], g).first);
        edges.push_back(boost::add_edge(vertices[2], vertices[1], g).first);
        edges.push_back(boost::add_edge(vertices[3], vertices[2], g).first);
        edges.push_back(boost::add_edge(vertices[3], vertices[6], g).first);
        edges.push_back(boost::add_edge(vertices[4], vertices[3], g).first);
        edges.push_back(boost::add_edge(vertices[4], vertices[5], g).first);
        // edges.push_back(boost::add_edge(vertices[4], vertices[6], g).first);
        edges.push_back(boost::add_edge(vertices[5], vertices[0], g).first);
        edges.push_back(boost::add_edge(vertices[5], vertices[5], g).first);
        edges.push_back(boost::add_edge(vertices[6], vertices[4], g).first);

        halgo::LoopLocator<Graph> vis;

        hmem::Handle<std::vector<std::vector<Edge>>> loops_vis
            = hmem::make_handle<std::vector<std::vector<Edge>>>();

        vis.init(loops_vis);

        boost::depth_first_search(g, boost::visitor(vis));

        size_t counter = 0;
        for (const auto& loop : *loops_vis) {
            std::cout << "Loop #" << counter++ << std::endl;

            for (const auto& edge : loop) {
                std::cout << "  " << boost::source(edge, g) << " -> "
                          << boost::target(edge, g) << std::endl;
            }

            std::cout << std::endl;
        }
    }

    virtual void update(hemlock::FrameTime) override {
        // Empty.
    }

    virtual void draw(hemlock::FrameTime) override {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }

    virtual void init(const std::string& name, happ::ProcessBase* process) override {
        happ::ScreenBase::init(name, process);

        m_state = happ::ScreenState::RUNNING;

        m_input_manager
            = static_cast<happ::SingleWindowApp*>(m_process)->input_manager();
    }
protected:
    MyIOManager        m_iom;
    hui::InputManager* m_input_manager;
};

#endif  // __hemlock_tests_test_mod_screen_hpp
