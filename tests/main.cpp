#include "stdafx.h"

#include "app/app.h"
#include "app/screen.h"
#include "graphics/glsl_program.h"
#include "graphics/sprite/batcher.h"
#include "graphics/window_manager.h"
#include "io/iomanager.h"

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

    virtual void update(TimeData time [[maybe_unused]]) override { /* Empty. */ }
    virtual void draw(TimeData time [[maybe_unused]])   override { /* Empty. */ }

    virtual void init(std::string name) override {
        happ::ScreenBase::init(name);

        m_state = happ::ScreenState::RUNNING;
    }
};

template <hemlock::ResizableContiguousContainer c>
c give_me_container() {
    return c();
}

class MyApp : public happ::BasicApp {
public:
    virtual ~MyApp() { /* Empty */ };
protected:      
    virtual void prepare_screens() override {
        happ::ScreenBase* my_screen = new MyScreen();

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

    MyPrinterTask* my_task_ptrs[3] = {
        &my_tasks[0],
        &my_tasks[1],
        &my_tasks[2]
    };

    pool.add_tasks((hemlock::IThreadTask<ThreadContext>**)my_task_ptrs, 3);

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

    auto quit_handler = hemlock::Delegate<void(hemlock::Sender)>([&pool](hemlock::Sender) {
        pool.dispose();
    });
    app.on_quit += &quit_handler;

    MyIOManager my_iom;
    hg::ShaderCache my_shader_cache;
    auto my_shader_parser = hg::ShaderCache::Parser([](const hio::fs::path& path, hio::IOManagerBase* iom) -> std::string {
        std::string buffer;
        if (!iom->read_file_to_string(path, buffer)) return "";

        return buffer;
    });
    // std::cout << std::endl << my_shader_parser("shaders/default_sprite.frag", &my_iom) << std::endl << std::endl;
    my_shader_cache.init(&my_iom, my_shader_parser);
    hg::s::SpriteBatcher my_sprite_batcher;
    my_sprite_batcher.init(&my_shader_cache);

    auto [ my_second_window, err ] = app.window_manager()->add_window();

    if (err != hg::WindowError::NONE) {
        std::cout << "Failed to make a second window." << std::endl;
    }

    std::cout << "Hello, world!" << std::endl;

    app.run();
}
