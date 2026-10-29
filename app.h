#pragma once

#include "nocopy.h"

namespace clessidra {

class Application {
public:
    Application()  = default;
    ~Application() = default;

    void init();
    void loop();

private:
    NOCOPY(Application)
};

} // namespace clessidra
