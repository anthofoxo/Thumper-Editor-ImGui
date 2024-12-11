#include "te_structs.hpp"

namespace tcle {
	void Datapoint::deserialize(ByteStream& aStream, uint32_t aDatatype) {
		time = aStream.read_f32();

        enum Datatype : uint32_t {
            kTraitBool = 1,
            kTraitFloat = 2,
            kTraitColor = 3,
            kTraitAction = 8,
        };

        switch (aDatatype) {
        case kTraitBool:
            value = aStream.read_u8();
            break;
        case kTraitFloat:
            value = aStream.read_f32();
            break;
        case kTraitColor:
            value = aStream.read_f32vec4();
            break;
        case kTraitAction:
            value = aStream.read_u8();
            break;
        default:
            __debugbreak(); // Unsupported datatype
        }

        interpolation = aStream.read_str();
        easing = aStream.read_str();
	}

    void Trait::deserialize(ByteStream& aStream) {
        object = aStream.read_str();
        unknown0 = aStream.read_u32();
        selector = aStream.read_u32();
        selectorShareIdx = aStream.read_s32();
        datatype = aStream.read_u32();

        datapoints.resize(aStream.read_u32());
        for (size_t iDatapoint = 0; iDatapoint < datapoints.size(); ++iDatapoint) {
            datapoints[iDatapoint].deserialize(aStream, datatype);
        }

        editorDatapoints.resize(aStream.read_u32());
        for (size_t iDatapoint = 0; iDatapoint < editorDatapoints.size(); ++iDatapoint) {
            editorDatapoints[iDatapoint].deserialize(aStream, datatype);
        }

        unknown1 = aStream.read_u32();
        unknown2 = aStream.read_u32();
        unknown3 = aStream.read_u32();
        unknown4 = aStream.read_u32();
        unknown5 = aStream.read_u32();

        intensity0 = aStream.read_str();
        intensity1 = aStream.read_str();

        unknown6 = aStream.read_u8();
        unknown7 = aStream.read_u8();
        unknown8 = aStream.read_u32();

        unknown9 = aStream.read_f32();
        unknown10 = aStream.read_f32();
        unknown11 = aStream.read_f32();
        unknown12 = aStream.read_f32();
        unknown13 = aStream.read_f32();

        unknown14 = aStream.read_u8();
        unknown15 = aStream.read_u8();
        unknown16 = aStream.read_u8();
    }

    void Leaf::deserialize(ByteStream& aStream) {
        hash0 = aStream.read_u32();
        unknown0 = aStream.read_u32();
        hash1 = aStream.read_u32();
        timeUnit = aStream.read_str();
        hash2 = aStream.read_u32();

        traits.resize(aStream.read_u32());
        for (int iTrait = 0; iTrait < traits.size(); ++iTrait) {
            traits[iTrait].deserialize(aStream);
        }

        unknown1 = aStream.read_u32();

        unknown2.resize(aStream.read_u32());
        for (int iUnknown2 = 0; iUnknown2 < unknown2.size(); ++iUnknown2) {
            unknown2[iUnknown2] = aStream.read_u32vec3();
        }

        unknown3 = aStream.read_u32();
        unknown4 = aStream.read_u32();
        unknown5 = aStream.read_u32();
    }
}