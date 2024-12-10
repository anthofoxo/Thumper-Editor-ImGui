#pragma once

#include <glad/gl.h>

#include <array>

namespace tcle {
	void gui_diff_table(bool& aOpen, std::array<GLuint, 8> const& aDiffTextures);
}