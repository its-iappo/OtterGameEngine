#include "OtterPCH.h"

#include "Core/EngineCore.h"
#include "Core/Application.h"
#include <Events/EventDispatcher.h>
#include <Events/WindowCloseEvent.h>
#include <Rendering/Vulkan/VulkanRenderer.h>

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
		static uint16_t frameCount = 0;
		while (mRunning) {
			if (OtterCrashReporter::HasCrashed()) [[unlikely]] {
				OtterCrashReporter::ShowDetailedCrashWindow();
				mRunning = false;
			}

			if (frameCount >= 100) {
				OTTER_ASSERT(false, "Intentional crash for testing purposes");
			}

			mWindow->OnUpdate();
			mRenderer->DrawFrame();
			frameCount++;
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
