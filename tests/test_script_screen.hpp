#ifndef __hemlock_tests_test_script_screen_hpp
#define __hemlock_tests_test_script_screen_hpp

#include "app/screen_base.h"
#include "script/environment_registry.hpp"
#include "script/lua/environment.hpp"

#include "iomanager.hpp"

using TSS_Env       = hscript::lua::Environment<true, 10>;
using TSS_EnvReg    = hscript::EnvironmentRegistry<TSS_Env>;

class TestScriptScreen : public happ::ScreenBase {
public:
    TestScriptScreen() :
        happ::ScreenBase(),
        m_input_manager(nullptr)
    { /* Empty. */ }
    virtual ~TestScriptScreen() { /* Empty */ };

    virtual void start(hemlock::FrameTime time) override {
        happ::ScreenBase::start(time);

        m_lua_env_reg = new TSS_EnvReg();
        m_lua_env_reg->init(&m_iom);

        m_lua_env_1 = m_lua_env_reg->create_environment("env_1");

        m_lua_env_2 = m_lua_env_reg->create_environment("env_2");

        m_lua_env_1->run(hio::fs::path("scripts/hello_world.lua"));

        hscript::ScriptDelegate<void> hello_world;
        m_lua_env_1->get_script_function<void>("hello_world", hello_world);
        hello_world();

        m_lua_env_2->run(hio::fs::path("scripts/call_hello_world.lua"));

        m_lua_env_1->rpc.pump_calls();

        hscript::ScriptDelegate<void> check_hello_world;
        m_lua_env_2->get_script_function<void>("check_hello_world", check_hello_world);
        check_hello_world();
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
    MyIOManager         m_iom;
    hui::InputManager   *m_input_manager;
    TSS_EnvReg          *m_lua_env_reg;
    TSS_Env             *m_lua_env_1, *m_lua_env_2;
};

#endif // __hemlock_tests_test_script_screen_hpp
