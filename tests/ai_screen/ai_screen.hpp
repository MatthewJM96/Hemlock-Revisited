#ifndef __hemlock_tests_ai_screen_ai_screen_hpp
#define __hemlock_tests_ai_screen_ai_screen_hpp

#define HEMLOCK_DEBUG_ACS

#include "app/screen_base.h"

#include "algorithm/acs/acs.hpp"

#include "voxel/ai/navmesh/navmesh_task.hpp"

#include "../iomanager.hpp"

#include "basic_graph.hpp"
#include "maze_graph.hpp"

class TestAIScreen : public happ::ScreenBase {
public:
    TestAIScreen() : happ::ScreenBase(), m_input_manager(nullptr) { /* Empty. */
    }

    virtual ~TestAIScreen(){ /* Empty */ };

    virtual void start(hemlock::FrameTime time) override {
        happ::ScreenBase::start(time);

        // std::cout << "********************\n* Basic Graph Test
        // *\n********************"
        //           << std::endl
        //           << std::endl;

        // do_basic_graph_test();

        // std::cout << std::endl << std::endl;

        // std::cout << "*******************\n* Maze Graph Test *\n*******************"
        //           << std::endl
        //           << std::endl;

        // do_maze_graph_test();

        std::cout << std::endl << std::endl;

        std::cout << "**************************\n* Maze Graph Timing Test "
                     "*\n**************************"
                  << std::endl
                  << std::endl;

        do_maze_graph_timing_test(4000000);

        // do_maze_graph_timing_test(40000, 3);

        // do_maze_graph_timing_test(40000, 7);
    }

    virtual void update(hemlock::FrameTime) override { }

    virtual void draw(hemlock::FrameTime) override {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

    virtual void init(const std::string& name, happ::ProcessBase* process) override {
        happ::ScreenBase::init(name, process);

        m_state = happ::ScreenState::RUNNING;

        m_input_manager
            = static_cast<happ::SingleWindowApp*>(m_process)->input_manager();
    }
protected:
    hui::InputManager* m_input_manager;
};

#endif  // __hemlock_tests_ai_screen_ai_screen_hpp
