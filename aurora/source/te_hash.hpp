#pragma once

#include <string>
#include <string_view>
#include <cstdint>

namespace aurora {
	uint32_t hash(unsigned char const* array, unsigned int size);
	uint32_t hash(std::string_view str);
	std::string rev_hash(uint32_t hash);
	void reload_hashtable();
}