#pragma once

#ifdef TE_WINDOWS
#	include <Windows.h> // Prevent APIENTRY redefinition
#endif

#include <GLFW/glfw3.h>

#include <utility>

namespace tcle {
	class Window final {
	public:
		struct CreateInfo final {
			int width = 1280;
			int height = 720;
			char const* title = nullptr;

			bool maximized = false;
			bool visible = true;
		};

		constexpr Window() = default;
		Window(CreateInfo const& aInfo);
		~Window() noexcept;

		Window(Window const&) = delete;
		Window& operator=(Window const&) = delete;
		inline Window(Window&& aInfo) noexcept { *this = std::move(aInfo); };
		Window& operator=(Window&& aInfo) noexcept;

		inline GLFWwindow* handle() const { return mHandle; }
		inline explicit operator bool() const { return mHandle; }
		inline operator GLFWwindow*() const { return mHandle; }
	private:
		GLFWwindow* mHandle = nullptr;
	};
}