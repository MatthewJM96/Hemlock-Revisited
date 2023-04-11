#include "stdafx.h"

#include "app.hpp"

int main(int, char*[]) {
    MyApp app;
    app.init();
    app.run();

    app.dispose();

    return 0;
}
