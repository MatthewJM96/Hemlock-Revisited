#include "stdafx.h"

void add(hemlock::Sender, ui32 a, ui32 b) { std::cout << a << " + " << b << " = " << a + b << std::endl; }

i32 main() {
    hemlock::Event<ui32, ui32> on_calc;

    auto listener = on_calc.add_functor(add);

    on_calc(1, 4);

    on_calc.remove(listener);
    delete listener;

    on_calc(1, 4);

    SDL_Init(SDL_INIT_EVERYTHING);
    SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    ui32 flags  = SDL_WINDOW_OPENGL;
        //  flags |= SDL_WINDOW_FULLSCREEN;
        //  flags |= SDL_WINDOW_BORDERLESS;
         flags |= SDL_WINDOW_RESIZABLE;

    auto window = SDL_CreateWindow("Hemlock", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 720, 480, flags);
    if (window == nullptr) {
        return 1;
    }

    auto context = SDL_GL_CreateContext(window);
    if (context == NULL) {
        return 2;
    }

    std::cout << "Hello, world!" << std::endl;

    while (true) {}
}
