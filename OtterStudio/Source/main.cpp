#include <iostream>

#include "Core/Logger.h"
#include "Core/Application.h"


int main() {
	OtterEngine::Application app;

	OTTER_CLIENT_LOG("Starting OtterStudio!");

	try {
		app.Run();
	}
	catch (const std::exception& e) {
		OTTER_CLIENT_CRITICAL("Otter Studio encountered a fatal error: {}", e.what());
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}
