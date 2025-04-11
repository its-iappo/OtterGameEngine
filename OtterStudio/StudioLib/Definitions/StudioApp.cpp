#include "StudioApp.h"
#include <iostream>

namespace OtterStudio {

    StudioApp::StudioApp() {
        std::cout << "[OtterStudio] Editor initialized.\n";
    }

    void StudioApp::Run() {
        std::cout << "[OtterStudio] Running editor loop...\n";
        // Placeholder loop
        while (running) {
            // Your future GUI/render/editor logic here
            break; // Just exit immediately for now
        }
    }
}
