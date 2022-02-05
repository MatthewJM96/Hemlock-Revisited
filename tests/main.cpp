#include "stdafx.h"

#include "app/app.h"
#include "app/screen.h"
#include "graphics/window_manager.h"

struct ThreadContext {
    bool stop = false;
    std::string message = "";
};

class MyPrinterTask : public hemlock::IThreadTask<ThreadContext> {
public:
    virtual void execute(hemlock::Thread<ThreadContext>::State* state, hemlock::TaskQueue<ThreadContext>* task_queue) override {
        state->context.message += "hello ";

        std::cout << state->context.message << std::endl;

        sleep(1);

        task_queue->enqueue(state->producer_token, new MyPrinterTask());
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

    auto [ my_second_window, err ] = app.window_manager()->add_window();

    if (err != hg::WindowError::NONE) {
        std::cout << "Failed to make a second window." << std::endl;
    }

    std::cout << "Hello, world!" << std::endl;

    app.run();
}
