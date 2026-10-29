#pragma once

#include "nocopy.h"

namespace clessidra {

class Application {
public:
    Application()  = default;
    ~Application() = default;

    void Init();
    void Loop();

private:
    NOCOPY(Application)
};

} // namespace clessidra
