#pragma once

#include "Core/Window.h"
#include <Rendering/IRenderer.h>

namespace OtterEngine {

	class Application {

	private:
		bool mRunning = true;

		std::unique_ptr<Window> mWindow;
		std::unique_ptr<IRenderer> mRenderer;


	public:
		Application();
		virtual ~Application();

		void Run();

		void OnEvent(Event& e);
	};
}
