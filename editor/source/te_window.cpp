#include "te_window.hpp"

#include <stdexcept>
#include <iostream>

namespace {
	int gWindowCount = 0;
}

namespace tcle {	
	Window::Window(CreateInfo const& aInfo) {
		if (gWindowCount == 0) {
			glfwSetErrorCallback([](int aErrorCode, char const* aDescription) {
				std::cerr << "GLFW Error " << aErrorCode << ": " << aDescription << '\n';
			});

			if (!glfwInit()) {
				throw std::runtime_error("glfwInit failed");
			}
		}

		glfwWindowHint(GLFW_VISIBLE, aInfo.visible);
		glfwWindowHint(GLFW_MAXIMIZED, aInfo.maximized);
		mHandle = glfwCreateWindow(aInfo.width, aInfo.height, aInfo.title, nullptr, nullptr);

		if (!mHandle) {
			if (gWindowCount == 0) {
				glfwTerminate();
			}

			throw std::runtime_error("glfwCreateWindow failed");
		}

		++gWindowCount;
	}

	Window::~Window() noexcept {
		if (!mHandle) return;

		glfwDestroyWindow(mHandle);
		--gWindowCount;

		if (gWindowCount == 0) {
			glfwTerminate();
		}
	}

	Window& Window::operator=(Window&& aOther) noexcept {
		std::swap(mHandle, aOther.mHandle);
		return *this;
	}
}