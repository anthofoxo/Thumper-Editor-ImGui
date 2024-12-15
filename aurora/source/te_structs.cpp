#include "te_structs.hpp"

#include <array>
#include <span>
#include <iostream>

namespace aurora {
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
        _beginOffset = aStream.mOffset;

        for(int i = 0; i < 4; ++i)
            header[i] = aStream.read_u32();

        assert(header[0] == 0x22);
        assert(header[1] == 0x21);
        assert(header[2] == 0x04);
        assert(header[3] == 0x02);

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

        _endOffset = aStream.mOffset;
    }

    void ObjlibLevel::deserialize(ByteStream& aStream) {
        filetype = aStream.read_u32();
        assert(filetype == 0x8);
        objlibType = aStream.read_u32();
        assert(objlibType == 0x0b374d9e);

        unknown0 = aStream.read_u32();
        unknown1 = aStream.read_u32();
        unknown2 = aStream.read_u32();
        unknown3 = aStream.read_u32();

        libraryImports.resize(aStream.read_u32());
        for (int i = 0; i < libraryImports.size(); ++i) {
            libraryImports[i].unknown0 = aStream.read_u32();
            libraryImports[i].library = aStream.read_str();
        }

        origin = aStream.read_str();
        
        objectImports.resize(aStream.read_u32());
        for (int i = 0; i < objectImports.size(); ++i) {
            objectImports[i].type = aStream.read_u32();
            objectImports[i].name = aStream.read_str();
            objectImports[i].unknown0 = aStream.read_u32();
            objectImports[i].library = aStream.read_str();
        }

        objectDeclarations.resize(aStream.read_u32());
        for (int i = 0; i < objectDeclarations.size(); ++i) {
            objectDeclarations[i].type = aStream.read_u32();
            objectDeclarations[i].name = aStream.read_str();
        }

        // Begin object readback

        for (size_t iDeclaration = 0; iDeclaration < objectDeclarations.size(); ++iDeclaration) {
            auto& declaration = objectDeclarations[iDeclaration];

            // Leaf
            if (declaration.type == 0xce7e85f6) {
                uint32_t header[]{ 0x22, 0x21, 0x04, 0x02 };
                auto headerBytes = std::span<char>(reinterpret_cast<char*>(std::addressof(header)), sizeof(header));

                auto it = std::search(aStream.mData.begin() + aStream.mOffset, aStream.mData.end(), headerBytes.begin(), headerBytes.end());
                if (it != aStream.mData.end()) {
                    aStream.advance(std::distance(aStream.mData.begin() + aStream.mOffset, it));

                    declaration._definitionOffset = aStream.mOffset;
                    Leaf leaf;
                    leaf._declaredName = declaration.name;
                    leaf.deserialize(aStream);

                    _leafs.push_back(std::move(leaf));
                }
            }

            // Samp
            else if (declaration.type == 0x7aa8f390) {
                uint32_t header[]{ 0x0C, 0x04 };
                auto headerBytes = std::span<char>(reinterpret_cast<char*>(std::addressof(header)), sizeof(header));

                auto it = std::search(aStream.mData.begin() + aStream.mOffset, aStream.mData.end(), headerBytes.begin(), headerBytes.end());
                if (it != aStream.mData.end()) {
                    aStream.advance(std::distance(aStream.mData.begin() + aStream.mOffset, it));

                    declaration._definitionOffset = aStream.mOffset;
                    Samp samp;
                    samp._declaredName = declaration.name;
                    samp.deserialize(aStream);

                    _samps.push_back(std::move(samp));
                }
                
            }

            // Spn
            else if (declaration.type == 0xd897d5db) {


                uint32_t header[]{ 0x01, 0x04, 0x02 };
                auto headerBytes = std::span<char>(reinterpret_cast<char*>(std::addressof(header)), sizeof(header));

                auto it = std::search(aStream.mData.begin() + aStream.mOffset, aStream.mData.end(), headerBytes.begin(), headerBytes.end());
                if (it != aStream.mData.end()) {
                    aStream.advance(std::distance(aStream.mData.begin() + aStream.mOffset, it));

                    declaration._definitionOffset = aStream.mOffset;
                    Spn definition;
                    definition._declaredName = declaration.name;
                    definition._beginOffset = aStream.mOffset;
                    definition.deserialize(aStream);
                    definition._endOffset = aStream.mOffset;

                    _spns.push_back(std::move(definition));
                }
            }

            // Master
            else if (declaration.type == 0x490780b9) {
                uint32_t header[]{ 33, 33, 4, 2 };
                auto headerBytes = std::span<char>(reinterpret_cast<char*>(std::addressof(header)), sizeof(header));

                auto it = std::search(aStream.mData.begin() + aStream.mOffset, aStream.mData.end(), headerBytes.begin(), headerBytes.end());
                if (it != aStream.mData.end()) {
                    aStream.advance(std::distance(aStream.mData.begin() + aStream.mOffset, it));

                    declaration._definitionOffset = aStream.mOffset;
                    SequinMaster definition;
                    definition._declaredName = declaration.name;
                    definition._beginOffset = aStream.mOffset;
                    definition.deserialize(aStream);
                    definition._endOffset = aStream.mOffset;

                    _masters.push_back(std::move(definition));
                }
            }

            // Drawer
            else if (declaration.type == 0xd3058b5d) {
                uint32_t header[]{ 7, 4, 1 };
                auto headerBytes = std::span<char>(reinterpret_cast<char*>(std::addressof(header)), sizeof(header));

                auto it = std::search(aStream.mData.begin() + aStream.mOffset, aStream.mData.end(), headerBytes.begin(), headerBytes.end());
                if (it != aStream.mData.end()) {
                    aStream.advance(std::distance(aStream.mData.begin() + aStream.mOffset, it));

                    declaration._definitionOffset = aStream.mOffset;
                    SequinDrawer definition;
                    definition._declaredName = declaration.name;
                    definition._beginOffset = aStream.mOffset;
                    definition.deserialize(aStream);
                    definition._endOffset = aStream.mOffset;

                    _drawers.push_back(std::move(definition));
                }

            }

        }

        // Footer
    }

    void Samp::deserialize(ByteStream& aStream) {
        _beginOffset = aStream.mOffset;

        header[0] = aStream.read_u32();
        assert(header[0] == 0x0C);
        header[1] = aStream.read_u32();
        assert(header[1] == 0x04);

        // if 0x01 the next value is a hash, if 0 there is no hash
        // The datatype doesnt match a thumper boolean with a 1 byte size
        // This may likely be a hash count, but we've only observed values of 0 and 1
        // and thus are assuming this is a boolean
        // Theres only a handful of cases where this is 0 and is typically 1
        uint32_t hasHash = aStream.read_u32();
        if (hasHash == 0x01) hash0 = aStream.read_u32();
        assert(hasHash < 2); // If this is ever >= 2, we want to know about it

        samplePlayMode = aStream.read_str();
        unknown0 = aStream.read_u32();
        filePath = aStream.read_str();

        for (int i = 0; i < 5; ++i) {
            unknown1[i] = aStream.read_u8();
        }

        volume = aStream.read_f32();
        pitch = aStream.read_f32();
        pan = aStream.read_f32();
        offset = aStream.read_f32();
        channelGroup = aStream.read_str();

        _endOffset = aStream.mOffset;
    }

    void Spn::deserialize(ByteStream& aStream) {
        header[0] = aStream.read_u32();
        assert(header[0] == 0x01);
        header[1] = aStream.read_u32();
        assert(header[1] == 0x04);
        header[2] = aStream.read_u32();
        assert(header[2] == 0x02);

        hash0 = aStream.read_u32();
        hash1 = aStream.read_u32();
        unknown0 = aStream.read_u32();
        xfmName = aStream.read_str();

        constraint = aStream.read_str();

        transform = aStream.read_transform();

        unknown2 = aStream.read_u32();

        objlibPath = aStream.read_str();
        bucketType = aStream.read_str();
    }

    void SequinMasterLvl::deserialize(ByteStream& aStream) {
        lvlName = aStream.read_str();
        gateName = aStream.read_str();
        isCheckpoint = aStream.read_u8();
        checkpointLeaderLvlName = aStream.read_str();
        restLvlName = aStream.read_str();

        unknownBool0 = aStream.read_u8();
        unknownBool1 = aStream.read_u8();
        unknown0 = aStream.read_u32();
        unknownBool2 = aStream.read_u8();
        //unknownBool3 = aStream.read_u8();

        playPlus = aStream.read_u8();
    }

    void SequinMaster::deserialize(ByteStream& aStream) {
        header[0] = aStream.read_u32();
        assert(header[0] == 33);
        header[1] = aStream.read_u32();
        assert(header[1] == 33);
        header[2] = aStream.read_u32();
        assert(header[2] == 0x04);
        header[3] = aStream.read_u32();
        assert(header[3] == 0x02);

        hash0 = aStream.read_u32();
        unknown0 = aStream.read_u32();
        hash1 = aStream.read_u32();
        timeUnit = aStream.read_str();
        hash2 = aStream.read_u32();
        unknown1 = aStream.read_u32();
        unknown2 = aStream.read_f32();
        skybox = aStream.read_str();
        introLvl = aStream.read_str();

        sublevels.resize(aStream.read_u32());

        for (auto& sublevel : sublevels) {
            sublevel.deserialize(aStream);
        }

        footer1 = aStream.read_u8();
        footer2 = aStream.read_u8();
        footer3 = aStream.read_u32();
        footer4 = aStream.read_u32();
        footer5 = aStream.read_u32();
        footer6 = aStream.read_u32();
        footer7 = aStream.read_f32();
        footer8 = aStream.read_f32();
        footer9 = aStream.read_f32();
        checkpointLvl = aStream.read_str();
        pathGameplay = aStream.read_str();
    }

    void SequinDrawer::deserialize(ByteStream& aStream) {
        header[0] = aStream.read_u32();
        assert(header[0] == 7);
        header[1] = aStream.read_u32();
        assert(header[1] == 4);
        header[2] = aStream.read_u32();
        assert(header[2] == 1);

        hash0 = aStream.read_u32();
        unknown0 = aStream.read_u32();
        unknownBool0 = aStream.read_u8();
        drawLayers = aStream.read_str();
        bucketType = aStream.read_str();
        unknown1 = aStream.read_u32();
    }
}