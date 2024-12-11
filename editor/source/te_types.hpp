#pragma once

namespace tcle {
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
}