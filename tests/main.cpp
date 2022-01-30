#include "stdafx.h"

#include "app/app.h"
#include "app/screen.h"

void add(hemlock::Sender, ui32 a, ui32 b) { std::cout << a << " + " << b << " = " << a + b << std::endl; }

class MyScreen : public happ::IScreen {
public:
    virtual ~MyScreen() { /* Empty */ };

    virtual void update(TimeData time [[maybe_unused]]) override { /* Empty. */ }
    virtual void draw(TimeData time [[maybe_unused]])   override { /* Empty. */ }

    virtual void init(std::string name) override {
        happ::IScreen::init(name);

        m_state = happ::ScreenState::RUNNING;
    }
};

class MyApp : public happ::BasicApp {
public:
    virtual ~MyApp() { /* Empty */ };
protected:      
    virtual void prepare_screens() override {
        happ::IScreen* my_screen = new MyScreen();

        m_screens.insert({ "my_screen", my_screen });

        m_current_screen = my_screen;
    }
};

i32 main() {
    hemlock::Event<ui32, ui32> on_calc;

    auto listener = on_calc.add_functor(add);

    on_calc(1, 4);

    on_calc.remove(listener);
    delete listener;

    on_calc(1, 4);

    MyApp app;
    app.init();

    std::cout << "Hello, world!" << std::endl;

    app.run();
}
