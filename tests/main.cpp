#include "stdafx.h"

#include "graphics/window.h"

void add(hemlock::Sender, ui32 a, ui32 b) { std::cout << a << " + " << b << " = " << a + b << std::endl; }

i32 main() {
    hemlock::Event<ui32, ui32> on_calc;

    auto listener = on_calc.add_functor(add);

    on_calc(1, 4);

    on_calc.remove(listener);
    delete listener;

    on_calc(1, 4);

    hg::Window window({
        .dimensions={1024, 800}
    });
    window.init();

    std::cout << "Hello, world!" << std::endl;

    while (true) {}
}
