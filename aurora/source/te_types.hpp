#pragma once

#include <cstdint>

namespace aurora {
	struct u32vec3 final {
		union { uint32_t x, r; };
		union { uint32_t y, g; };
		union { uint32_t z, b; };
	};

	struct f32vec3 final {
		union { float x, r; };
		union { float y, g; };
		union { float z, b; };
	};

	struct f32vec4 final {
		union { float x, r; };
		union { float y, g; };
		union { float z, b; };
		union { float w, a; };
	};

	struct Transform final {
		f32vec3 position;
		f32vec3 rotX;
		f32vec3 rotY;
		f32vec3 rotZ;
		f32vec3 scale;
	};
}