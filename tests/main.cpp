namespace std {
    constexpr bool is_constant_evaluated() noexcept {
        return __builtin_is_constant_evaluated();
    }
}

import <iostream>;

import hemlock.event;
import hemlock.types;

void add(hemlock::Sender, ui32 a, ui32 b) { std::cout << a << " + " << b << " = " << a + b << std::endl; }

i32 main() {
    hemlock::Event<ui32, ui32> on_calc;

    auto listener = on_calc.add_functor(add);

    on_calc(1, 4);

    on_calc.remove(listener);
    delete listener;

    on_calc(1, 4);

    std::cout << "Hello, world!" << std::endl;
}
