#ifndef __hemlock_tests_test_script_screen_hpp
#define __hemlock_tests_test_script_screen_hpp

#include "app/screen_base.h"
#include "io/yaml.hpp"
#include "script/lua/environment.hpp"

#include "iomanager.hpp"

class TestScriptScreen : public happ::ScreenBase {
public:
    TestScriptScreen() :
        happ::ScreenBase(),
        m_input_manager(nullptr)
    { /* Empty. */ }
    virtual ~TestScriptScreen() { /* Empty */ };

    virtual void start(hemlock::FrameTime time) override {
        happ::ScreenBase::start(time);

        m_lua_env = new hscript::lua::Environment();
        m_lua_env->init(&m_iom);

        m_lua_env->run(hio::fs::path("scripts/hello_world.lua"));

        hscript::ScriptDelegate<void> hello_world;
        m_lua_env->get_script_function<void>("hello_world", hello_world);

        hello_world();
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

        m_input_manager = static_cast<happ::SingleWindowApp*>(m_process)->input_manager();
    }
protected:
    MyIOManager                  m_iom;
    hui::InputManager*           m_input_manager;
    hscript::lua::Environment*   m_lua_env;
};

#endif // __hemlock_tests_test_script_screen_hpp
