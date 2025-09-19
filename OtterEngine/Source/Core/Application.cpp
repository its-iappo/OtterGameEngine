#include "Core/Application.h"
#include "Core/Logger.h"
#include "Core/EngineCore.h"
#include <Events/EventDispatcher.h>
#include <Events/WindowCloseEvent.h>
#include <Rendering/VulkanRenderer.h>

namespace OtterEngine {

	Application::Application() {
		EngineCore::Start();

		mWindow = std::make_unique<Window>(800, 600, "Otter Engine Window");
		mWindow->SetEventCallback([this](Event& e) {OnEvent(e); });

		mRenderer = std::make_unique<VulkanRenderer>(mWindow->getWindow());
		mRenderer->Init();

		OTTER_CORE_LOG("Application created");
	}

	Application::~Application() {
		mRenderer->Clear();
	}

	void Application::Run() {
		while (mRunning) {
			mWindow->OnUpdate();
			mRenderer->DrawFrame();
		}
	}

	void Application::OnEvent(Event& e) {
		EventDispatcher dispatcher(e);

		dispatcher.Dispatch<WindowCloseEvent>([this](WindowCloseEvent& e) {
			OTTER_CLIENT_LOG("Closing application window!");
			mRunning = false;
			return true;
		});
	}
}
