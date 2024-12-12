#pragma once

#include "te_stream.hpp"

#include <any>
#include <string>

namespace tcle {
	struct Datapoint final {
		float time;
		std::any value;
		std::string interpolation;
		std::string easing;

		void deserialize(ByteStream& aStream, uint32_t aDatatype);
	};

	struct Trait final {
		std::string object;
		uint32_t unknown0;
		uint32_t selector;
		int32_t selectorShareIdx;
		uint32_t datatype;
		std::vector<Datapoint> datapoints;
		std::vector<Datapoint> editorDatapoints;

		uint32_t unknown1;
		uint32_t unknown2;
		uint32_t unknown3;
		uint32_t unknown4;
		uint32_t unknown5;

		std::string intensity0;
		std::string intensity1;

		uint8_t unknown6;
		uint8_t unknown7;
		uint32_t unknown8;

		float unknown9;
		float unknown10;
		float unknown11;
		float unknown12;
		float unknown13;

		uint8_t unknown14;
		uint8_t unknown15;
		uint8_t unknown16;

		void deserialize(ByteStream& aStream);
	};

	struct Leaf final {
		std::string _declaredName;

		uint32_t header[4];
		uint32_t hash0;
		uint32_t unknown0;
		uint32_t hash1;
		std::string timeUnit;
		uint32_t hash2;
		std::vector<Trait> traits;
		uint32_t unknown1;
		std::vector<u32vec3> unknown2;
		uint32_t unknown3;
		uint32_t unknown4;
		uint32_t unknown5;

		void deserialize(ByteStream& aStream);
	};

	struct LibraryImport final {
		uint32_t unknown0;
		std::string library;
	};

	struct ObjectImport final {
		uint32_t type;
		std::string name;
		uint32_t unknown0;
		std::string library;
	};

	struct ObjectDeclaration final {
		size_t _definitionOffset = 0;

		uint32_t type;
		std::string name;
	};

	struct ObjlibLevel final {
		std::vector<char> _bytes;
		std::vector<Leaf> _leafs;

		uint32_t filetype; // 0x8
		uint32_t objlibType; // 0x19621c9d
		uint32_t unknown0;
		uint32_t unknown1;
		uint32_t unknown2;
		uint32_t unknown3;
		std::vector<LibraryImport> libraryImports;
		std::string origin;
		std::vector<ObjectImport> objectImports;
		std::vector<ObjectDeclaration> objectDeclarations;

		// Object definitions are dumped here

		// Footer

		void deserialize(ByteStream& aStream);
	};
}