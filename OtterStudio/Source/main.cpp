#include <iostream>

#include "Core/Logger.h"
#include "Core/Application.h"


int main() {
	OtterEngine::Application app;

	OTTER_CLIENT_LOG("Starting OtterStudio!");

	app.Run();

	return EXIT_SUCCESS;
}
