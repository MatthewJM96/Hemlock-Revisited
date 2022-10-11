#ifndef __hemlock_tests_test_script_screen_hpp
#define __hemlock_tests_test_script_screen_hpp

#include "app/screen_base.h"
#include "io/yaml.hpp"
#include "script/lua/environment.hpp"

#include "iomanager.hpp"

H_DECL_ENUM_WITH_SERIALISATION(
    ,
    TestEnum,
    ui8,
    HELLO,
    WORLD
)
H_DEF_ENUM_WITH_SERIALISATION(
    ,
    TestEnum,
    HELLO,
    WORLD
)

H_DEF_STRUCT_WITH_YAML_CONVERSION(
    ,
    Person,
    (name, std::string)
);
H_DEF_STRUCT_WITH_YAML_CONVERSION(
    ,
    TestStruct,
    (pos, i32v3),
    (person, Person)
);

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
        if (m_lua_env->get_script_function<void>("hello_world", hello_world)) {
            hello_world();
        }

        YAML::Node vec_node = YAML::Load("[1, 2, 3]");
        i32v3 vec = vec_node.as<i32v3>();
        std::cout << "vec: " << vec.x << " " << vec.y << " " << vec.z << std::endl;

        YAML::Node sq_mat_node = YAML::Load("[[1, 2], [3, 4]]");
        i32m2 sq_mat = sq_mat_node.as<i32m2>();
        std::cout << "sq mat: " << sq_mat[0][0] << " " << sq_mat[1][0] << " / " << sq_mat[0][1] << " " << sq_mat[1][1] << std::endl;

        YAML::Node mat_node = YAML::Load("[[1, 2], [3, 4], [5, 6]]");
        glm::i32mat2x3 mat = mat_node.as<glm::i32mat2x3>();
        std::cout << "mat: " << mat[0][0] << " " << mat[1][0] << " / " << mat[0][1] << " " << mat[1][1] << " / " << mat[0][2] << " " << mat[1][2] << std::endl;

        YAML::Node test_enum_node = YAML::Load("[ 'HELLO', 'WORLD' ]");
        std::array<TestEnum, 2> test_enum = test_enum_node.as<std::array<TestEnum, 2>>();
        std::cout << hio::serialisable_enum_name(test_enum[0]) << ", " << hio::serialisable_enum_name(test_enum[1]) << std::endl;

        YAML::Node test_struct_node = YAML::Load("{ pos: [1, 2, 3], person: { name: 'Matthew' } }");
        TestStruct test_struct = test_struct_node.as<TestStruct>();
        std::cout << "Person: " << test_struct.person.name << " at " << test_struct.pos.x << " " << test_struct.pos.y << " " << test_struct.pos.z << std::endl;
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
