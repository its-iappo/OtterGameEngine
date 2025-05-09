#ifdef OTTER_USE_MODULES
import OtterEngine.EngineCore;
import OtterEngine.Logger;
#else
#include "EngineCore.h"
#include "Logger.h"
#endif

#include <iostream>
#include "LoggerMacros.h"

int main() {
    OtterEngine::EngineCore engine;

    engine.Start();
    OTTER_CLIENT_LOG("Starting OtterStudio!");

    std::cout << "OtterStudio is running!" << std::endl;
    std::cin.get();

    return 0;
}
