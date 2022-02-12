#include "stdafx.h"

#include "app/single_window_app.h"
#include "app/screen_base.h"
#include "app/process/process_base.h"
#include "app/window/state.hpp"
#include "app/window/window_base.h"
#include "camera/basic_first_person_camera.h"
#include "graphics/font/font.h"
#include "graphics/glsl_program.h"
#include "graphics/sprite/batcher.h"
#include "io/iomanager.h"
#include "ui/input/dispatcher.h"
#include "ui/input/keys.hpp"
#include "ui/input/manager.h"

class MyIOManager : public hio::IOManagerBase {
public:
    virtual bool resolve_path(const hio::fs::path& path, OUT hio::fs::path& full_path) const override {
        full_path = hio::fs::absolute(path);
        return true;
    }
    virtual bool assure_path (  const hio::fs::path& path,
                                    OUT hio::fs::path& full_path,
                                            bool is_file      = false,
                                        OUT bool* was_existing [[maybe_unused]] = nullptr ) const override {
        if (is_file) {
            // bleh
        } else {
            hio::fs::create_directories(path);
        }
        full_path = hio::fs::absolute(path);
        return true;
    }

    virtual bool resolve_paths(IN OUT std::vector<hio::fs::path>& paths) const override {
        bool bad = false;
        for (auto& path : paths) {
            hio::fs::path tmp{};
            bad |= !resolve_path(path, tmp);
            path = tmp;
        }
        return !bad;
    }
    virtual bool assure_paths (IN OUT std::vector<hio::fs::path>& paths) const override {
        bool bad = false;
        for (auto& path : paths) {
            hio::fs::path tmp{};
            bad |= !assure_path(path, tmp);
            path = tmp;
        }
        return !bad;
    }
};

struct ThreadContext {
    bool stop = false;
    std::string message = "";
};

class MyPrinterTask : public hemlock::IThreadTask<ThreadContext> {
public:
    virtual void execute(hemlock::Thread<ThreadContext>::State* state, hemlock::TaskQueue<ThreadContext>* task_queue [[maybe_unused]]) override {
        state->context.message += "hello ";

        std::cout << state->context.message << std::endl;

        sleep(1);

        // task_queue->enqueue(state->producer_token, new MyPrinterTask());
    }
};

void add(hemlock::Sender, ui32 a, ui32 b) { std::cout << a << " + " << b << " = " << a + b << std::endl; }

class MyScreen : public happ::ScreenBase {
public:
    virtual ~MyScreen() { /* Empty */ };

    virtual void update(TimeData time) override {
        m_sprite_batcher.begin();
        m_sprite_batcher.add_sprite(
            f32v2{
                60.0f + 30.0f * sin(time.total / 1000.0f),
                60.0f + 30.0f * cos(time.total / 1000.0f)
            },
            f32v2{200.0f, 200.0f},
            colour4{255,   0, 0, 255},
            colour4{  0, 255, 0, 255},
            hg::Gradient::LEFT_TO_RIGHT
        );
        m_sprite_batcher.add_string(
            "Hello, world!",
            f32v4{300.0f, 300.0f, 1000.0f, 1000.0f},
            f32v4{295.0f, 295.0f, 1010.0f, 1010.0f},
            hg::f::StringSizing{hg::f::StringSizingKind::SCALED, f32v2{1.0f}},
            colour4{0, 0, 0, 255},
            "fonts/Orbitron-Regular.ttf",
            hg::f::TextAlign::TOP_LEFT,
            hg::f::WordWrap::NONE//,
            // 0.0f,
            // hg::f::FontStyle::NORMAL,
            // hg::f::FontRenderStyle::SOLID
        );
        m_sprite_batcher.end();

        f32v3 delta_pos{0.0f};
        if (m_input_manager->is_pressed(hui::PhysicalKey::H_W)) {
            delta_pos += glm::normalize(m_camera.direction()) * static_cast<f32>(time.frame) * 0.2f;
        }
        if (m_input_manager->is_pressed(hui::PhysicalKey::H_A)) {
            delta_pos -= glm::normalize(m_camera.right()) * static_cast<f32>(time.frame) * 0.2f;
        }
        if (m_input_manager->is_pressed(hui::PhysicalKey::H_S)) {
            delta_pos -= glm::normalize(m_camera.direction()) * static_cast<f32>(time.frame) * 0.2f;
        }
        if (m_input_manager->is_pressed(hui::PhysicalKey::H_D)) {
            delta_pos += glm::normalize(m_camera.right()) * static_cast<f32>(time.frame) * 0.2f;
        }
        if (m_input_manager->is_pressed(hui::PhysicalKey::H_Q)) {
            delta_pos += glm::normalize(m_camera.up()) * static_cast<f32>(time.frame) * 0.2f;
        }
        if (m_input_manager->is_pressed(hui::PhysicalKey::H_E)) {
            delta_pos -= glm::normalize(m_camera.up()) * static_cast<f32>(time.frame) * 0.2f;
        }

        const f32 turn_clamp_on_at = 60.0f / 360.0f * 2.0f * M_PI;
        f32 up_angle = glm::acos(glm::dot(m_camera.up(), hcam::ABSOLUTE_UP));
        if (up_angle < -1.0f * turn_clamp_on_at || up_angle > turn_clamp_on_at)
            m_camera.set_clamp_enabled(true);

        m_camera.offset_position(delta_pos);
        m_camera.update();
    }
    virtual void draw(TimeData time [[maybe_unused]]) override {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // happ::WindowDimensions dims = m_process->window()->dimensions();
        // m_sprite_batcher.render(f32v2{dims.width, dims.height});
        m_sprite_batcher.render(f32m4{1.0f}, m_camera.view_projection_matrix());
    }

    virtual void init(const std::string& name, happ::ProcessBase* process) override {
        happ::ScreenBase::init(name, process);

        m_state = happ::ScreenState::RUNNING;

        m_input_manager = static_cast<happ::SingleWindowApp*>(m_process)->input_manager();

        m_camera.attach_to_window(m_process->window());
        m_camera.set_position(f32v3{400.0f, 200.0f, -130.0f});
        m_camera.set_fov(90.0f);
        m_camera.set_clamp({false, 30.0f / 360.0f * 2.0f * M_PI});
        m_camera.update();

        handle_mouse_move = hemlock::Subscriber<hui::MouseMoveEvent>(
            [&](hemlock::Sender, hui::MouseMoveEvent ev) {
                if (m_input_manager->is_pressed(static_cast<ui8>(hui::MouseButton::LEFT))) {
                    m_camera.rotate_from_mouse_with_absolute_up(
                        -1.0f * static_cast<f32>(ev.dx),
                        -1.0f * static_cast<f32>(ev.dy),
                        0.005f
                    );
                }
            }
        );

        hui::InputDispatcher::instance()->on_mouse.move += &handle_mouse_move;

        // std::cout << std::endl << my_shader_parser("shaders/default_sprite.frag", &my_iom) << std::endl << std::endl;
        m_shader_cache.init(&m_iom, hg::ShaderCache::Parser(
            [](const hio::fs::path& path, hio::IOManagerBase* iom) -> std::string {
                std::string buffer;
                if (!iom->read_file_to_string(path, buffer)) return "";

                return buffer;
            }
        ));

        m_font_cache.init(&m_iom, hg::f::FontCache::Parser(
            [](const hio::fs::path& path, hio::IOManagerBase* iom) -> hg::f::Font {
                hio::fs::path actual_path;
                if (!iom->resolve_path(path, actual_path)) return hg::f::Font{};

                hg::f::Font font;
                font.init(actual_path.string());

                return font;
            }
        ));


        auto font = m_font_cache.fetch("fonts/Orbitron-Regular.ttf");
        font->set_default_size(50);
        font->generate(/*hg::f::FontStyle::NORMAL, hg::f::FontRenderStyle::SOLID*/);
        // auto font_instance = font->get_instance(/*hg::f::FontStyle::NORMAL, hg::f::FontRenderStyle::SOLID*/);
        // font_instance.save("test.png", {hio::img::png::save});

        m_sprite_batcher.init(&m_shader_cache, &m_font_cache);
    }
protected:
    hemlock::Subscriber<hui::MouseMoveEvent>      handle_mouse_move;

    MyIOManager                  m_iom;
    hg::ShaderCache              m_shader_cache;
    hg::f::FontCache             m_font_cache;
    hg::s::SpriteBatcher         m_sprite_batcher;
    hcam::BasicFirstPersonCamera m_camera;
    hui::InputManager*           m_input_manager;
};

template <hemlock::ResizableContiguousContainer c>
c give_me_container() {
    return c();
}

class MyApp : public happ::SingleWindowApp {
public:
    virtual ~MyApp() { /* Empty */ };
protected:
    virtual void prepare_screens() override {
        happ::ScreenBase* my_screen = new MyScreen();
        my_screen->init("my_screen", this);

        m_screens.insert({ "my_screen", my_screen });

        m_current_screen = my_screen;
    }
};

i32 main() {
    hemlock::ThreadPool<ThreadContext> pool;
    pool.init(3);

    MyPrinterTask my_tasks[3] = {
        MyPrinterTask(),
        MyPrinterTask(),
        MyPrinterTask()
    };

    hemlock::HeldTask<ThreadContext> held_tasks[3] = {
        {static_cast<hemlock::IThreadTask<ThreadContext>*>(&my_tasks[0]), false},
        {static_cast<hemlock::IThreadTask<ThreadContext>*>(&my_tasks[1]), false},
        {static_cast<hemlock::IThreadTask<ThreadContext>*>(&my_tasks[2]), false}
    };

    pool.add_tasks(held_tasks, 3);

    hemlock::Event<ui32, ui32> on_calc;

    auto listener = on_calc.add_functor(add);

    on_calc(1, 4);

    on_calc.remove(listener);
    delete listener;

    on_calc(1, 4);

    auto my_cont = give_me_container<std::vector<bool>>();
    my_cont.push_back(true);

    std::cout << "My container has " << my_cont.size() << " elements." << std::endl;

    MyApp app;
    app.init();

    auto quit_handler = hemlock::Subscriber<>([&pool](hemlock::Sender) {
        pool.dispose();
    });
    app.on_quit += &quit_handler;

    // auto [ my_second_window, err ] = app.window_manager()->add_window();
    // if (err != hg::WindowError::NONE) {
    //     std::cout << "Failed to make a second window." << std::endl;
    // }

    std::cout << "Hello, world!" << std::endl;

    app.run();
}
