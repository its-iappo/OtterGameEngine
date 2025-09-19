#include "Core/Window.h"
#include "Core/Logger.h"
#include "Events/WindowCloseEvent.h"

namespace OtterEngine {

    Window::Window(int width, int height, const std::string& title)
        : mWidth(width), mHeight(height), mTitle(title)
    {
        if (!glfwInit()) {
            OTTER_CORE_ERROR("Failed to initialize GLFW!");
        }

		// Prevent GLFW from creating an OpenGL context
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

        pWindow = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
        if (!pWindow) {
            OTTER_CORE_ERROR("Failed to create GLFW window!");
            glfwTerminate();
        }

		glfwSetWindowUserPointer(pWindow, this);

        glfwSetFramebufferSizeCallback(pWindow, 
           [](GLFWwindow* window, int newWidth, int newHeight) {
				Window* wndw = (Window*)glfwGetWindowUserPointer(window);
                wndw->mWidth = newWidth;
                wndw->mHeight = newHeight;
				wndw->mFrameBufferResized = true;

                OTTER_CORE_LOG("Window resized with Width = {} - Height = {}", newWidth, newHeight);
           });

        glfwSetWindowCloseCallback(pWindow, [](GLFWwindow* window) {
			Window* wndw = (Window*)glfwGetWindowUserPointer(window);
            WindowCloseEvent closeEvent;
            if (wndw->mEventCallback) {
				wndw->mEventCallback(closeEvent);
            }
            });
    }

    Window::~Window() {
        glfwDestroyWindow(pWindow);
        glfwTerminate();
    }

    void Window::OnUpdate() {
        glfwPollEvents();
        glfwSwapBuffers(pWindow);
    }
}
