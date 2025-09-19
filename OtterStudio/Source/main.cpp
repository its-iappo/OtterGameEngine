#include "Core/Application.h"
#include "Core/Logger.h"

#include <iostream>

int main() {
    OtterEngine::Application app;

    OTTER_CLIENT_LOG("Starting OtterStudio!");
    app.Run();
    return 0;
}
