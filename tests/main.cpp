#include "stdafx.h"

#include <iostream>

ui32 add(ui32 a, ui16 b) {
    return a + static_cast<ui32>(b);
}

i32 main() {
    std::cout << "1 + 5 = " << add(1, 5) << std::endl;

    std::cout << "Hello, world!" << std::endl;
}
