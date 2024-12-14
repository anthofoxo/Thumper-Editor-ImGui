#pragma once

#include "te_stream.hpp"

#include <any>
#include <string>

namespace aurora {
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

	struct SampleEntry final {	// correct
		std::string sampleName;
		uint32_t loopBeats;
		uint32_t unknown2;
	};

	struct LvlLeafSequin final {
		uint8_t unknownBool0; //might not exist, but was found at 0x5590 in demo.objlib
		uint32_t beatCount;
		uint8_t unknownBool1;
		std::string leafName;
		std::string defaultPath;
		uint32_t subPathCount; //may not be needed
		std::vector<std::string> subpaths;

		uint32_t unknown0;
		std::string stepType;	//available options are		kStepAny	kStepFirst	kStepGameplay	kStepLast	kStepProp

		uint32_t unknown1;
		uint32_t unknown2;
		uint32_t unknown3;
		uint32_t unknown4;

		//vec3
		float unknownFloat0;
		float unknownFloat1;
		float unknownFloat2;

		float unknownFloat3;
		float unknownFloat4;
		float unknownFloat5;

		float unknownFloat6;
		float unknownFloat7;
		float unknownFloat8;

		float unknownFloat9;
		float unknownFloat10;		// scale
		float unknownFloat11;

		uint8_t unknownBool2;
		uint8_t unknownBool3;
		uint8_t unknownBool4;

		uint32_t unknown5;			// this is placed in between each leaf sequin, but is exempt in the final leaf sequin. !!UNSURE WHAT TO DO WITH THIS!!

	};


	struct Leaf final {
		std::string _declaredName;
		size_t _beginOffset = 0;
		size_t _endOffset = 0;

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

	struct Samp final {

		std::string _declaredName;
		
		uint32_t header[3];
		uint32_t hash0;
		std::string samplePlayMode;
		uint32_t unknown0;
		std::string filePath;

		//In the files, it's either a boolean then an int, or an int then a boolean. The order is unknown. I changed these on the debug version and they didn't do anything.
		uint8_t unknown1;
		uint32_t unknown2;

		//end

		float volume;
		float pitch;
		float pan;
		float offset;
		std::string channelGroup;


	};

	struct Lvl final {						// struct developed from 0x5177 offset from demo.objlib
		std::string _declaredName;

		uint32_t header[4];
		uint32_t hash0;
		uint32_t unknown0;
		uint32_t hash1;
		std::string timeUnit;
		uint32_t unknown1;
		uint32_t unknownInt0;
		uint32_t editStateCompHash;
		std::vector<Trait> traits;
		uint32_t unknown2;
		std::string phaseMoveType;
		uint32_t unknown3;
		uint32_t leafSequinCount;
		uint8_t unknownBool0;

		std::vector<LvlLeafSequin> LeafSequin;		//all leafs are read in and the tunnels attached to them. everything to here should be right


		uint32_t sampCount;
		std::vector<SampleEntry> sampNames;

		uint8_t unknownBool5;

		float unknownFloat12;
		float unknownFloat13;	//unknown if float
		float unknownfloat14;	//unknown if float
		std::string kNumTraitType;	//available options are		kNumTraitInterps	kNumTraitTypes
		uint8_t unknownBool6;
		std::string tutorialType;	//available options are		TUTORIAL_GRIND, TUTORIAL_GRIND, TUTORIAL_JUMP, TUTORIAL_LANES, TUTORIAL_NONE, TUTORIAL_POUND, TUTORIAL_POUND_REMINDER, TUTORIAL_POWER_GRIND, TUTORIAL_THUMP, TUTORIAL_THUMP_REMINDER, TUTORIAL_TURN_LEFT, TUTORIAL_TURN_RIGHT

		//footer (maybe????)

		float footer1;
		float footer2;
		float footer3;


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