#include "te_hash.hpp"

#include <unordered_map>
#include <format>
#include <array>

#include <yaml-cpp/yaml.h>

namespace aurora {
    uint32_t hash(unsigned char const* array, unsigned int size) {
        uint32_t h = 0x811c9dc5;

        while (size > 0) {
            size--;
            h = (h ^ *array) * 0x1000193;
            array++;
        }

        h *= 0x2001;
        h = (h ^ (h >> 0x7)) * 0x9;
        h = (h ^ (h >> 0x11)) * 0x21;

        return h;
    }

    uint32_t hash(std::string_view str) {
        return hash((unsigned char const*)str.data(), static_cast<unsigned int>(str.size()));
    }

    namespace {
        std::unordered_map<uint32_t, std::string> gHashtable;
    }

    void reload_hashtable() {
        gHashtable.clear();
        YAML::Node yaml = YAML::LoadFile("hashtable.yaml");

        for (YAML::Node const& node : yaml["known"]) {
            std::string str = node.as<std::string>();
            gHashtable[hash(str)] = std::move(str);
        }

        for (YAML::const_iterator it = yaml["unknown"].begin(); it != yaml["unknown"].end(); ++it) {
            uint32_t hash = it->first.as<uint32_t>();
            std::string str = it->second.as<std::string>();
            gHashtable[hash] = std::move(str);
        }
    }

	std::string rev_hash(uint32_t hash) {
        auto it = gHashtable.find(hash);
        if (it == gHashtable.end()) return std::format("{:x}", hash);
        return it->second;
	}
}