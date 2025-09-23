#pragma once

#include <string>
#include <memory>
#include <GLFW/glfw3.h>

#include "Events/Event.h"

namespace OtterEngine {

	class Window {

	private:
		int mWidth, mHeight;
		std::string mTitle;
		GLFWwindow* pWindow;
		EventCallback mEventCallback;
		bool mFrameBufferResized = false;

	public:
		Window(int width, int height, const std::string& title);
		~Window();

		void OnUpdate();
		inline int getWidth() const { return mWidth; }
		inline int getHeight() const { return mHeight; }
		inline GLFWwindow* getWindow() const { return pWindow; }
		inline bool wasFrameBufferResized() const { return mFrameBufferResized; }
		inline void resetFrameBufferResizedFlag() { mFrameBufferResized = false; }

		void SetEventCallback(const EventCallback& callback) { mEventCallback = callback; }
	};
}
