#ifdef TE_WINDOWS
#   include <Windows.h>
#endif

#include <glad/gl.h>
#include <GLFW/glfw3.h>

#include "te_audio.hpp"
#include "te_window.hpp"
#include "te_image.hpp"
#include "te_diff_table.hpp"

#include <stb_image.h>
#include <miniaudio.h>

#include <tinyfiledialogs.h>
#include <yaml-cpp/yaml.h>

#include <imgui.h>
#include <imgui_internal.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include <misc/cpp/imgui_stdlib.h>
#include <imgui_memory_editor.h>

#include <cstdlib> // EXIT_SUCCESS, EXIT_FAILURE
#include <stdio.h>
#include <string>
#include <vector>
#include <filesystem>
#include <fstream>
#include <memory>
#include <any>

struct Level {
    std::string name;
    std::string difficulty;
    std::string description;
    std::string author;
};

uint32_t hash32(unsigned char const* array, unsigned int size) {
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

uint32_t hash32(std::string const& str) {
    return hash32((unsigned char const*)str.data(), static_cast<unsigned int>(str.size()));
}

void hash_panel(bool& open) {
    static std::string input = "type input here";
    static uint32_t hash = hash32(input);

    if (!open) return;

    if (ImGui::Begin("Hash Panel", &open)) {
        if (ImGui::InputText("Input", &input)) hash = hash32(input);
        ImGui::Text("0x%02x", hash);
    }
    ImGui::End();
}

void about_panel(GLuint icon, bool& open) {
    if (!open) return;

    if (open) {
        if (ImGui::Begin("About Thumper Mod Loader v2.0.0.0", &open, ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::Image((ImTextureID)(uintptr_t)icon, { 128.0f, 128.0f });
            ImGui::TextUnformatted("Thumper Mod Loader");
            ImGui::TextUnformatted("Version 2.0.0.0");
            ImGui::TextUnformatted("Copyright (c) 2081");
            ImGui::TextUnformatted("CocoaMix Inc.");
            ImGui::TextUnformatted("A tool used for building custom level for the game Thumper.");
        }
        ImGui::End();
    }
}

std::string path_to_string(std::filesystem::path const& path) {
    auto const u8string = path.generic_u8string();
    return std::string(reinterpret_cast<char const*>(u8string.data()), u8string.cend() - u8string.cbegin());
}

std::optional<std::filesystem::path> select_directory_from_file() {
    char const* filter = "THUMPER_*.exe";
    char* selection = tinyfd_openFileDialog("Select your Thumper Installation Directory", nullptr, 1, &filter, nullptr, false);
    if (!selection) return std::nullopt;
    return std::filesystem::path(selection).parent_path();
}

std::optional<std::filesystem::path> select_directory_save() {
    auto path = select_directory_from_file();

    if (path) {
        YAML::Node rootNode = YAML::Node();
        rootNode["path"] = path_to_string(path.value());

        std::ofstream file("config.yaml", std::ios::out | std::ios::binary);
        file << rootNode;
    }

    return path;
}

void imgui_init(GLFWwindow* window) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    // io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

    ImGui::StyleColorsDark();

    ImGuiStyle& style = ImGui::GetStyle();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 150");

    io.Fonts->AddFontFromFileTTF("GlacialIndifference-Regular.otf", 18.0f);
}

void imgui_uninit() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

std::vector<char> read_path_binary(std::filesystem::path const& aPath) {
    std::ifstream file(aPath, std::ios::in | std::ios::binary);
    std::vector<char> buffer;
    file.seekg(0, std::ios::end);
    buffer.resize(file.tellg());
    file.seekg(0, std::ios::beg);
    file.read(buffer.data(), buffer.size());
    return buffer;
}

class Application final {
public:
    void init();
    void uninit();
    void run();
    void update();
public:
    tcle::Window mWindow;
    AudioEngine mAudioEngine;
    bool mRunning = true;

    ImFont* mFontMono = nullptr;

    GLuint mIconTexture = 0;
    std::array<GLuint, 8> mDiffTextures{};

    std::optional<std::filesystem::path> mThumperPath;

    bool mShowHashPanel = false;
    bool mShowAboutPanel = false;
    bool mShowDifficultyExplanation = false;
    bool mModMode = false;
    bool mShowDearImGuiDemo = false;
    bool mLeafEditorShown = false;
    bool mLvlEditorShown = false;
    bool mGateEditorShown = false;
    bool mMasterEditorShown = false;
    bool mWorkingFolderShown = false;
    bool mSampleEditorShown = false;

    std::vector<Level> mLevels;
};

uint32_t read_u32(std::vector<char>& bytes, size_t& offset) {
    uint32_t val = *reinterpret_cast<uint32_t*>(bytes.data() + offset);
    offset += sizeof(uint32_t);
    return val;
}

int32_t read_s32(std::vector<char>& bytes, size_t& offset) {
    int32_t val = *reinterpret_cast<int32_t*>(bytes.data() + offset);
    offset += sizeof(int32_t);
    return val;
}

float read_f32(std::vector<char>& bytes, size_t& offset) {
    float val = *reinterpret_cast<float*>(bytes.data() + offset);
    offset += sizeof(float);
    return val;
}

uint8_t read_u8(std::vector<char>& bytes, size_t& offset) {
    float val = *reinterpret_cast<uint8_t*>(bytes.data() + offset);
    offset += sizeof(uint8_t);
    return val;
}

std::string read_str(std::vector<char>& bytes, size_t& offset) {
    std::string string;
    string.resize(read_u32(bytes, offset));
    memcpy(string.data(), bytes.data() + offset, string.size());
    offset += string.size();
    return string;
}

// This current definition can fully read all vanilla thumper levels
struct ObjLibLeafDef final {
    std::string leafname; // Not in packed binary format
    size_t offset; // Not in packed binary format

    struct Datapoint final {
        float time;
        std::any value;
        std::string interpolation;
        std::string easing;

        void read(std::vector<char>& bytes, size_t& offset, uint32_t trait) {
            time = read_f32(bytes, offset);

            switch (trait) {
            case 1: // kTraitBool
                value = read_u8(bytes, offset);
                break;
            case 2: // kTraitFloat
                value = read_f32(bytes, offset);
                break;
            case 8: // kTraitAction
                value = read_u8(bytes, offset);
                break;
            default:
                __debugbreak();
            }

            interpolation = read_str(bytes, offset);
            easing = read_str(bytes, offset);
        }
    };

    struct Trait final {
        std::string traitName;
        uint32_t unknown0;
        uint32_t param;
        int32_t subObjectParamShareIdx;
        uint32_t trait;
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

        void read(std::vector<char>& bytes, size_t& offset) {
            traitName = read_str(bytes, offset);
            unknown0 = read_u32(bytes, offset);
            param = read_u32(bytes, offset);
            subObjectParamShareIdx = read_s32(bytes, offset);
            trait = read_u32(bytes, offset);
            datapoints.resize(read_u32(bytes, offset));

            for (int iDatapoint = 0; iDatapoint < datapoints.size(); ++iDatapoint) {
                datapoints[iDatapoint].read(bytes, offset, trait);
            }

            editorDatapoints.resize(read_u32(bytes, offset));

            for (int iDatapoint = 0; iDatapoint < editorDatapoints.size(); ++iDatapoint) {
                editorDatapoints[iDatapoint].read(bytes, offset, trait);
            }

            unknown1 = read_u32(bytes, offset);
            unknown2 = read_u32(bytes, offset);
            unknown3 = read_u32(bytes, offset);
            unknown4 = read_u32(bytes, offset);
            unknown5 = read_u32(bytes, offset);
            intensity0 = read_str(bytes, offset);
            intensity1 = read_str(bytes, offset);
            unknown6 = read_u8(bytes, offset);
            unknown7 = read_u8(bytes, offset);
            unknown8 = read_u32(bytes, offset);
            unknown9 = read_f32(bytes, offset);
            unknown10 = read_f32(bytes, offset);
            unknown11 = read_f32(bytes, offset);
            unknown12 = read_f32(bytes, offset);
            unknown13 = read_f32(bytes, offset);
            unknown14 = read_u8(bytes, offset);
            unknown15 = read_u8(bytes, offset);
            unknown16 = read_u8(bytes, offset);
        }
    };

    uint32_t hash;
    uint32_t unknown0;
    uint32_t hash2;
    std::string timeUnit;
    uint32_t hash3;
    std::vector<Trait> traits;

    void read(std::vector<char>& bytes, size_t& offset) {
        hash = read_u32(bytes, offset);
        unknown0 = read_u32(bytes, offset);
        hash2 = read_u32(bytes, offset);
        timeUnit = read_str(bytes, offset);
        hash3 = read_u32(bytes, offset);
        traits.resize(read_u32(bytes, offset));

        for (int iTrait = 0; iTrait < traits.size(); ++iTrait) {
            traits[iTrait].read(bytes, offset);
        }

        // Leaf footer, seems to be variable length??
        // Can ignore for now
    }
};

struct ObjlibLevel final {
    std::string name;
    std::vector<ObjLibLeafDef> leafs;
};

std::vector<ObjlibLevel> gLevels;

void read_all_leafs(std::optional<std::filesystem::path> const& aThumperPath) {
    std::array<std::string, 10> levels = {
        "Alevels/title_screen.objlib",
        "Alevels/demo.objlib",
        "Alevels/level2/level_2a.objlib",
        "Alevels/level3/level_3a.objlib",
        "Alevels/level4/level_4a.objlib",
        "Alevels/level5/level_5a.objlib",
        "Alevels/level6/level_6.objlib",
        "Alevels/level7/level_7a.objlib",
        "Alevels/level8/level_8a.objlib",
        "Alevels/level9/level_9a.objlib",
    };

    struct ObjLibLibraryImports final {
        uint32_t unknown00;
        std::string importName;
    };

    struct ObjLibObjectDeclaration final {
        uint32_t type;
        std::string name;

        // Not in objlib, computed at runtime
        size_t offset = 0;
        bool found = false;
    };

    struct ObjLibObjectImport final {
        uint32_t type;
        std::string objectName;
        uint32_t unknown0;
        std::string libraryName;
    };

    struct ParsedData {
        uint32_t fileType; // Assumes 8
        uint32_t objlibType;
        uint32_t unknown00;
        uint32_t unknown01;
        uint32_t unknown02;
        uint32_t unknown03;
        std::vector<ObjLibLibraryImports> libraryImports;
        std::string originName;
        std::vector<ObjLibObjectImport> objectImports;
        std::vector<ObjLibObjectDeclaration> objectDeclarations;
    };

    for (auto const& level : levels) {
        ObjlibLevel levelView;

        std::string path = std::format("{}/cache/{:x}.pc", path_to_string(aThumperPath.value()), hash32(level));
        auto bytes = read_path_binary(path);
        size_t offset = 0;

        ParsedData data;

        data.fileType = read_u32(bytes, offset);
        data.objlibType = read_u32(bytes, offset);
        data.unknown00 = read_u32(bytes, offset);
        data.unknown01 = read_u32(bytes, offset);
        data.unknown02 = read_u32(bytes, offset);
        data.unknown03 = read_u32(bytes, offset);
        data.libraryImports.resize(read_u32(bytes, offset));

        for (int i = 0; i < data.libraryImports.size(); ++i) {
            data.libraryImports[i].unknown00 = read_u32(bytes, offset);
            data.libraryImports[i].importName = read_str(bytes, offset);
        }

        data.originName = read_str(bytes, offset);
        data.objectImports.resize(read_u32(bytes, offset));

        levelView.name = data.originName;

        for (int i = 0; i < data.objectImports.size(); ++i) {
            data.objectImports[i].type = read_u32(bytes, offset);
            data.objectImports[i].objectName = read_str(bytes, offset);
            data.objectImports[i].unknown0 = read_u32(bytes, offset);
            data.objectImports[i].libraryName = read_str(bytes, offset);
        }

        data.objectDeclarations.resize(read_u32(bytes, offset));

        for (int i = 0; i < data.objectDeclarations.size(); ++i) {
            data.objectDeclarations[i].type = read_u32(bytes, offset);
            data.objectDeclarations[i].name = read_str(bytes, offset);
        }

        for (int i = 0; i < data.objectDeclarations.size(); ++i) {
            if (data.objectDeclarations[i].type == 0xce7e85f6) { // Leaf
                // Match leaf header
                std::array<char, 16> header{ 0x22, 0x00, 0x00, 0x00, 0x21, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, };
                auto it = std::search(bytes.begin() + offset, bytes.end(), header.begin(), header.end());

                // Found it
                if (it != bytes.end()) {
                    offset += std::distance(bytes.begin() + offset, it);
                    data.objectDeclarations[i].offset = offset;
                    data.objectDeclarations[i].found = true;

                    //printf("%s @ %s:0x%X\n", data.objectDeclarations[i].name.c_str(), data.originName.c_str(), offset);
                    offset += header.size(); // advance past header

                    ObjLibLeafDef leaf;
                    leaf.read(bytes, offset);
                    leaf.leafname = data.objectDeclarations[i].name;
                    leaf.offset = data.objectDeclarations[i].offset;

                    levelView.leafs.push_back(std::move(leaf));
                }
            }
#if 0
            if (data.objectDeclarations[i].type == 0x7aa8f390) { // Samp
                // Match leaf header
                std::array<char, 12> header{ 0x0C, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00 };
                auto it = std::search(bytes.begin() + offset, bytes.end(), header.begin(), header.end());

                // Found it
                if (it != bytes.end()) {
                    offset += std::distance(bytes.begin() + offset, it);
                    data.objectDeclarations[i].offset = offset;
                    data.objectDeclarations[i].found = true;

                    //printf("%s @ 0x%X\n", data.objectDeclarations[i].name.c_str(), offset);
                    offset += header.size(); // advance past header
                }
            }

            if (data.objectDeclarations[i].type == 0xbcd17473) { // Lvl
                // Match leaf header
                std::array<char, 16> header{ 0x33, 0x00, 0x00, 0x00, 0x21, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00 };
                auto it = std::search(bytes.begin() + offset, bytes.end(), header.begin(), header.end());

                // Found it
                if (it != bytes.end()) {
                    offset += std::distance(bytes.begin() + offset, it);
                    data.objectDeclarations[i].offset = offset;
                    data.objectDeclarations[i].found = true;

                    //printf("%s @ 0x%X\n", data.objectDeclarations[i].name.c_str(), offset);
                    offset += header.size(); // advance past header
                }
            }

            if (data.objectDeclarations[i].type == 0xaa63a508) { // Gate
                // Match leaf header
                std::array<char, 12> header{ 0x1A, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00 };
                auto it = std::search(bytes.begin() + offset, bytes.end(), header.begin(), header.end());

                // Found it
                if (it != bytes.end()) {
                    offset += std::distance(bytes.begin() + offset, it);
                    data.objectDeclarations[i].offset = offset;
                    data.objectDeclarations[i].found = true;

                    //printf("%s @ 0x%X\n", data.objectDeclarations[i].name.c_str(), offset);
                    offset += header.size(); // advance past header
                }
            }

            if (data.objectDeclarations[i].type == 0x86621b1e) { // Flow
                // Match leaf header
                std::array<char, 12> header{ 0x16, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00 };
                auto it = std::search(bytes.begin() + offset, bytes.end(), header.begin(), header.end());

                // Found it
                if (it != bytes.end()) {
                    offset += std::distance(bytes.begin() + offset, it);
                    data.objectDeclarations[i].offset = offset;
                    data.objectDeclarations[i].found = true;

                    //printf("%s @ 0x%X\n", data.objectDeclarations[i].name.c_str(), offset);
                    offset += header.size(); // advance past header
                }
            }

            if (data.objectDeclarations[i].type == 0x4890a3f6) { // Path
                // Match leaf header
                std::array<char, 12> header{ 0x29, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00 };
                auto it = std::search(bytes.begin() + offset, bytes.end(), header.begin(), header.end());

                // Found it
                if (it != bytes.end()) {
                    offset += std::distance(bytes.begin() + offset, it);
                    data.objectDeclarations[i].offset = offset;
                    data.objectDeclarations[i].found = true;

                    //printf("%s @ 0x%X\n", data.objectDeclarations[i].name.c_str(), offset);
                    offset += header.size(); // advance past header
                }
            }

            if (data.objectDeclarations[i].type == 0x8f86650f) { // Cam
                // Match leaf header
                std::array<char, 12> header{ 0x06, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00 };
                auto it = std::search(bytes.begin() + offset, bytes.end(), header.begin(), header.end());

                // Found it
                if (it != bytes.end()) {
                    offset += std::distance(bytes.begin() + offset, it);
                    data.objectDeclarations[i].offset = offset;
                    data.objectDeclarations[i].found = true;

                    //printf("%s @ 0x%X\n", data.objectDeclarations[i].name.c_str(), offset);
                    offset += header.size(); // advance past header
                }
            }

            if (data.objectDeclarations[i].type == 0xbf69f115) { // Mesh
                // Match leaf header
                std::array<char, 12> header{ 0x0F, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00 };
                auto it = std::search(bytes.begin() + offset, bytes.end(), header.begin(), header.end());

                // Found it
                if (it != bytes.end()) {
                    offset += std::distance(bytes.begin() + offset, it);
                    data.objectDeclarations[i].offset = offset;
                    data.objectDeclarations[i].found = true;

                    //printf("%s @ 0x%X\n", data.objectDeclarations[i].name.c_str(), offset);
                    offset += header.size(); // advance past header
                }
            }

            if (data.objectDeclarations[i].type == 0x490780b9) { // Master
                // Match leaf header
                std::array<char, 12> header{ 0x0C, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00 };
                auto it = std::search(bytes.begin() + offset, bytes.end(), header.begin(), header.end());

                // Found it
                if (it != bytes.end()) {
                    offset += std::distance(bytes.begin() + offset, it);
                    data.objectDeclarations[i].offset = offset;
                    data.objectDeclarations[i].found = true;

                    //printf("%s @ 0x%X\n", data.objectDeclarations[i].name.c_str(), offset);
                    offset += header.size(); // advance past header
                }
            }

            if (data.objectDeclarations[i].type == 0x5232f8f9) { // Anim
                // Match leaf header
                std::array<char, 12> header{ 0x21, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00 };
                //std::array<char, 16> header{ 0x04, 0x00, 0x00, 0x00, 0x21, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00 };
                auto it = std::search(bytes.begin() + offset, bytes.end(), header.begin(), header.end());

                // Found it
                if (it != bytes.end()) {
                    offset += std::distance(bytes.begin() + offset, it);
                    data.objectDeclarations[i].offset = offset;
                    data.objectDeclarations[i].found = true;

                    // printf("%s @ 0x%X\n", data.objectDeclarations[i].name.c_str(), offset);
                    offset += header.size(); // advance past header
                }
            }
#endif
        }

#if 0
        int notFoundCount = 0;

        printf("%s", "Could not find offsets for the following object declatations:\n");
        for (int i = 0; i < data.objectDeclarations.size(); ++i) {
            if (!data.objectDeclarations[i].found) {
                notFoundCount++;

                if (data.objectDeclarations[i].type != 0xce7e85f6) continue;


                printf("%s [%d]\n", data.objectDeclarations[i].name.c_str(), i);
            }
        }

        printf("----- Found %d / %d declaration offsets -----\n", data.objectDeclarations.size() - notFoundCount, data.objectDeclarations.size());
#endif

        gLevels.push_back(std::move(levelView));
    }
}

void Application::init() {
    // Read configs
    try {
        YAML::Node rootNode = YAML::LoadFile("config.yaml");
        std::string str = rootNode["path"].as<std::string>("");

        if (!str.empty()) {
            mThumperPath = std::filesystem::path(str);
        }
    }
    catch (YAML::BadFile const&) {
        // Do nothing, use defaults
        tinyfd_messageBox("ERROR", "Config file not found! \nYou will need to re-select your Thumper executable.", "ok", "error", 1);
    }

    // If invalid, request path
    if (!mThumperPath) {
        mThumperPath = select_directory_save();

        if (!mThumperPath) {
            mRunning = false;
            return;
        }
    }

    mWindow = tcle::Window({
        .width = 1280,
        .height = 720,
        .title = "Thumper Mod Loader v2.0.0.0",

        .maximized = true,
        .visible = false,
    });

    // Set window icon
    {
        tcle::Image icon32("thumper_modding_tool_32.png");
        GLFWimage image{
            .width = icon32.width(),
            .height = icon32.height(),
            .pixels = icon32.pixels(),
        };

        glfwSetWindowIcon(mWindow, 1, &image);
    }

    glfwShowWindow(mWindow);

    glfwMakeContextCurrent(mWindow);

    gladLoadGL(&glfwGetProcAddress);

    mAudioEngine.init();
    ma_engine_play_sound(&mAudioEngine.mEngine, "UIBoot.ogg", nullptr);

    // Load larger icon, seen in the about panel
    {
        tcle::Image image("thumper_modding_tool.png");
        glCreateTextures(GL_TEXTURE_2D, 1, &mIconTexture);
        glTextureStorage2D(mIconTexture, 1, GL_RGBA8, image.width(), image.height());
        glTextureSubImage2D(mIconTexture, 0, 0, 0, image.width(), image.height(), GL_RGBA, GL_UNSIGNED_BYTE, image.pixels());
        glTextureParameteri(mIconTexture, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    }

    // Load difficulty texture
    for (int i = 0; i < mDiffTextures.size(); ++i) {
        std::string path = std::format("difficulty_icons/d{}.png", i);

        tcle::Image image(path.c_str());
        glCreateTextures(GL_TEXTURE_2D, 1, &mDiffTextures[i]);
        glTextureStorage2D(mDiffTextures[i], 1, GL_RGBA8, image.width(), image.height());
        glTextureSubImage2D(mDiffTextures[i], 0, 0, 0, image.width(), image.height(), GL_RGBA, GL_UNSIGNED_BYTE, image.pixels());
        glTextureParameteri(mDiffTextures[i], GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    }

    imgui_init(mWindow);

    mFontMono = ImGui::GetIO().Fonts->AddFontFromFileTTF("fonts/NotoSansMono-Regular.ttf", 18.0f);

    read_all_leafs(mThumperPath);

    for (auto& entry : std::filesystem::directory_iterator("levels")) {
        if (!entry.is_directory()) continue;
        std::filesystem::path path = entry.path() / "LEVEL DETAILS.txt";
        if (!std::filesystem::exists(path)) continue;

        YAML::Node node = YAML::LoadFile(path_to_string(path));

        mLevels.emplace_back(
            node["level_name"].as<std::string>(""),
            node["difficulty"].as<std::string>(""),
            node["description"].as<std::string>(""),
            node["author"].as<std::string>("")
        );
    }

    
}

void Application::uninit() {
    mRunning = false;

    for (auto& texture : mDiffTextures) {
        glDeleteTextures(1, &texture);
    }

    glDeleteTextures(1, &mIconTexture);

    imgui_uninit();

    glfwMakeContextCurrent(nullptr);

    mWindow = {};
    mAudioEngine.uninit();
}

void Application::run() {
    init();

    while (mRunning) {
        glfwPollEvents();

        if (glfwWindowShouldClose(mWindow)) mRunning = false;

#ifdef TE_WINDOWS
        ImGui::GetIO().ConfigDebugIsDebuggerPresent = ::IsDebuggerPresent();
#endif

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::DockSpaceOverViewport();

        update();

        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(mWindow, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
            GLFWwindow* backup_current_context = glfwGetCurrentContext();
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
            glfwMakeContextCurrent(backup_current_context);
        }

        glfwSwapBuffers(mWindow);
    }

    uninit();
}

void Application::update() {
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Exit", ImGui::GetKeyChordName(ImGuiMod_Alt | ImGuiKey_F4))) {
                mRunning = false;
            }

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("View")) {
            ImGui::MenuItem("Dear ImGui Demo", nullptr, &mShowDearImGuiDemo);

            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
    }

    if (ImGui::Begin("Debug")) {
        std::string path = path_to_string(mThumperPath.value());
        ImGui::LabelText("Thumper Path", "%s", path.c_str());
    }
    ImGui::End();

    hash_panel(mShowHashPanel);
    about_panel(mIconTexture, mShowAboutPanel);

    tcle::gui_diff_table(mShowDifficultyExplanation, mDiffTextures);

    if (ImGui::Begin("Level Leafs")) {
        for (auto& level : gLevels) {
            if (ImGui::CollapsingHeader(level.name.c_str())) {
                for (auto& leaf : level.leafs) {
                    ImGui::Button(leaf.leafname.c_str());
                    ImGui::SetItemTooltip("Offset 0x%X", leaf.offset);
                }
            }
        }
    }
    ImGui::End();
                
    
#if 0
        auto hash_to_str = [](uint32_t hash) -> char const* {
            switch (hash) {
            case 0x5232f8f9: return ".anim Objects";
            case 0x7dd6b7d8: return ".bend Objects";
            case 0x570e17fa: return ".bind Objects";
            case 0x8f86650f: return ".cam Objects";
            case 0xadb02913: return ".ch Objects";
            case 0x4945e860: return ".cond Objects";
            case 0xac1abb2c: return ".dch Objects";
            case 0x9ce604da: return ".dec Objects";
            case 0xacc2033e: return ".dsp Objects";
            case 0xeae6beee: return ".ent Objects";
            case 0x3bbcc4ec: return ".env Objects";
            case 0x86621b1e: return ".flow Objects";
            case 0x6222e06f: return ".flt Objects";
            case 0x993811f5: return ".flt Objects";
            case 0xc2fd0a11: return ".gameplay Objects";
            case 0xaa63a508: return ".gate Objects";
            case 0xc2aaec43: return ".grp Objects";
            case 0xce7e85f6: return ".leaf Objects";
            case 0x711a2715: return ".light Objects";
            case 0xbcd17473: return ".lvl Objects";
            case 0x490780b9: return ".master Objects";
            case 0x1a5812f6: return ".mastering Objects";
            case 0x7ba5c8e0: return ".mat Objects";
            case 0xbf69f115: return ".mesh Objects";
            case 0x1ba51443: return " GFX.objlib Objects";
            case 0xb0954548: return " Sequin.objlib Objects";
            case 0x9d1c6219: return " Obj.objlib Objects";
            case 0x0b374d9e: return " Level.objlib Objects";
            case 0xe674624f: return " Avatar.objlib Objects";
            case 0x4890a3f6: return ".path Objects";
            case 0x745dd78b: return ".playspace Objects";
            case 0x230da622: return ".pulse Objects";
            case 0x7aa8f390: return ".samp Objects";
            case 0xd3058b5d: return ".sdraw / .drawer Objects";
            case 0xcac934cf: return ".sh Objects";
            case 0xd897d5db: return ".spn Objects";
            case 0xd955fdc6: return ".st Objects";
            case 0xe7b3aadb: return ".steer Objects";
            case 0x96ba8a70: return ".tex Objects";
            case 0x799c45a7: return ".vib Objects";
            case 0x4f37349d: return ".vr_settings Objects";
            case 0x7d9db5ef: return ".xfm / .xfmer Objects";
            default: return "?";
            }
        };
    
        ImGui::LabelText("File Type", "%d", data.fileType);
        ImGui::LabelText("ObjLib Type", "%08X (%s)", data.objlibType, hash_to_str(data.objlibType));
        ImGui::LabelText("Unknown 00", "%u", data.unknown00);
        ImGui::LabelText("Unknown 01", "%u", data.unknown01);
        ImGui::LabelText("Unknown 02", "%u", data.unknown02);
        ImGui::LabelText("Unknown 03", "%u", data.unknown03);
        ImGui::LabelText("Library Import Count", "%u", data.libraryImports.size());

        for (int i = 0; i < data.libraryImports.size(); ++i) {
            auto& libraryImport = data.libraryImports[i];

            ImGui::PushID(i);
            ImGui::LabelText("Unknown 00", "%u", libraryImport.unknown00);
            ImGui::LabelText("Import Name", "%s", libraryImport.importName.c_str());
            ImGui::PopID();
        }

        ImGui::LabelText("File name origin", "%s", data.originName.c_str());
        ImGui::LabelText("Object Import Count", "%u", data.objectImports.size());

        for (int i = 0; i < data.objectImports.size(); ++i) {
            auto& objImport = data.objectImports[i];

            ImGui::PushID(i);
            ImGui::LabelText("Type", "%08X (%s)", objImport.type, hash_to_str(objImport.type));
            ImGui::LabelText("Object Name", "%s", objImport.objectName.c_str());
            ImGui::LabelText("Unknown", "%u", objImport.unknown0);
            ImGui::LabelText("Library Name", "%s", objImport.libraryName.c_str());
            ImGui::PopID();
        }

        ImGui::LabelText("Object Declarations", "%u", data.objectDeclarations.size());

        ImGui::PushID("object declarations");
        for (int i = 0; i < data.objectDeclarations.size(); ++i) {
            auto& objDeclaration = data.objectDeclarations[i];

            
            ImGui::PushID(i);
            ImGui::LabelText("Type", "%08X (%s) [%d]", objDeclaration.type, hash_to_str(objDeclaration.type), i);

            if(objDeclaration.found)
                ImGui::LabelText("Name", "%s (0x%X)", objDeclaration.name.c_str(), objDeclaration.offset);
            else
                ImGui::LabelText("Name", "%s", objDeclaration.name.c_str());

            ImGui::PopID();
        }
        ImGui::PopID();

    }
    ImGui::End();
#endif

    if (ImGui::Begin("Thumper Level Editor v0.0.0.1", nullptr, ImGuiWindowFlags_MenuBar))
    {
        if (ImGui::BeginMenuBar())
        {
            if (ImGui::BeginMenu("File"))
            {
                if (ImGui::MenuItem("Save All", "CTRL+Q"))
                {
                    //Save all changes made to the objlib
                }

                if (ImGui::MenuItem("Open Level...", "CTRL+P"))
                {
                    //Open an objlib / level
                }

                if (ImGui::MenuItem("Recent Levels...", "CTRL+R"))
                {
                    //Open list of recently opened levels, will initially be none on first startup
                }

                ImGui::Separator();

                if (ImGui::MenuItem("Open current level in explorer", "CTRL+E"))
                {
                    //Open the currently opened level / objlib in file explorer
                }



                if (ImGui::BeginMenu("Leaf..."))
                {
                    if (ImGui::MenuItem("New", "CTRL+N"))
                    {
                        //Create a new, empty leaf file
                    }

                    if (ImGui::MenuItem("Open", "CTRL+O"))
                    {
                        //Open an existing leaf file
                    }

                    if (ImGui::MenuItem("Save", "CTRL+S"))
                    {
                        //Save the currently open leaf file
                    }

                    if (ImGui::MenuItem("Save As...", "CTRL+A"))
                    {
                        //Save the currently open leaf file as a different name / separate file
                    }

                    if (ImGui::MenuItem("Open Template", "CTRL+T"))
                    {
                        //Open a pre-made template of a leaf file
                    }

                    ImGui::EndMenu();
                }



                if (ImGui::BeginMenu("LVL..."))
                {
                    if (ImGui::MenuItem("New", "ALT+N"))
                    {
                        //Create a new, empty lvl file
                    }

                    if (ImGui::MenuItem("Open", "ALT+O"))
                    {
                        //Open an existing lvl file
                    }

                    if (ImGui::MenuItem("Save", "ALT+S"))
                    {
                        //Save the currently open lvl file
                    }

                    if (ImGui::MenuItem("Save As...", "ALT+A"))
                    {
                        //Save the currently open lvl file as a different name / separate file
                    }

                    ImGui::EndMenu();
                }



                if (ImGui::BeginMenu("Gate..."))
                {
                    if (ImGui::MenuItem("New", "CTRL+SHIFT+N"))
                    {
                        //Create a new, empty gate file
                    }

                    if (ImGui::MenuItem("Open", "CTRL+SHIFT+O"))
                    {
                        //Open an existing gate file
                    }

                    if (ImGui::MenuItem("Save", "CTRL+SHIFT+S"))
                    {
                        //Save the currently open gate file
                    }

                    if (ImGui::MenuItem("Save As...", "CTRL+SHIFT+A"))
                    {
                        //Save the currently open gate file as a different name / separate file
                    }

                    ImGui::EndMenu();
                }



                if (ImGui::BeginMenu("Master..."))
                {
                    if (ImGui::MenuItem("New", "CTRL+ALT+N"))
                    {
                        //Create a new, empty master sequence file
                    }

                    if (ImGui::MenuItem("Open", "CTRL+ALT+O"))
                    {
                        //Open an existing master sequence file
                    }

                    if (ImGui::MenuItem("Save", "CTRL+ALT+S"))
                    {
                        //Save the currently open master sequence file
                    }

                    if (ImGui::MenuItem("Save As...", "CTRL+ALT+A"))
                    {
                        //Save the currently open master sequence file as a different name / separate file
                    }

                    ImGui::EndMenu();
                }

                if (ImGui::BeginMenu("Sample..."))
                {
                    if (ImGui::MenuItem("New", "ALT+SHIFT+N"))
                    {
                        //Create a new, empty .samp container file
                    }

                    if (ImGui::MenuItem("Open", "ALT+SHIFT+O"))
                    {
                        //Open an existing lvl .samp container file
                    }

                    if (ImGui::MenuItem("Save", "ALT+SHIFT+S"))
                    {
                        //Save the currently open .samp container file
                    }

                    if (ImGui::MenuItem("Save As...", "ALT+SHIFT+A"))
                    {
                        //Save the currently open .samp container file as a different name / separate file
                    }

                    ImGui::EndMenu();
                }

                ImGui::Separator();

                if (ImGui::MenuItem("New Level Folder", "CTRL+L"))       // Fill background of button light green to highlight
                {
                    //Create a new project folder that contains all the files of the custom level.
                }

                if (ImGui::MenuItem("Edit Level Details"))       // Fill background of button light grey to highlight
                {
                    //Create a new details file that stores info about the level - description, authors, bpm etc...
                }

                if (ImGui::MenuItem("Regenerate Default Files"))       // Fill background of button light grey to highlight.        
                {
                    //No comment, I actually don't know what this does
                }

                if (ImGui::BeginMenu("Template files..."))
                {
                    if (ImGui::MenuItem("Open folder"))
                    {
                        //Open the folder that contains leaf template files
                    }

                    if (ImGui::MenuItem("Regenerate files"))
                    {
                        //Probably regenerates the default template files if they got deleted by accident
                    }

                    ImGui::EndMenu();
                }

                ImGui::Separator();

                if (ImGui::MenuItem("Exit"))
                {
                    //Probably not needed anymore, but originally closes the entire editor program. Could probably re-use this to close the currently open level.
                }

                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Edit"))
            {

                if (ImGui::MenuItem("Preferences..."))
                {
                    /*

                    This opens a separate window that allows you to:
                    - change the default colors of the editor
                        - menu color
                        - master sequence color
                        - lvl editor color
                        - sample editor color
                        - gate editor color
                        - leaf editor color
                        - active panel color
                        - background color
                        - default track object selected (would probably be removed later) and its param
                    - audio
                        - mute application audio
                    - keybinds
                        - every keybind. leaf new, leaf open, template.. everything. Includes a search function.

                    */
                }

                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("View"))
            {
                ImGui::MenuItem("Leaf Editor", nullptr, &mLeafEditorShown);
                ImGui::MenuItem("Lvl Editor", nullptr, &mLvlEditorShown);
                ImGui::MenuItem("Gate Editor", nullptr, &mGateEditorShown);
                ImGui::MenuItem("Master Editor", nullptr, &mMasterEditorShown);
                ImGui::MenuItem("Working Folder", nullptr, &mWorkingFolderShown);
                ImGui::MenuItem("Sample Editor", nullptr, &mSampleEditorShown);

                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Window"))
            {
                //No clue what this is.

                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Help"))
            {
                //Fill this in later because I cannot be bothered

                ImGui::EndMenu();
            }

            ImGui::EndMenuBar();
        }
    }
    ImGui::End();

    if (ImGui::Begin("Thumper Mod Loader v2.0.0.0", nullptr, ImGuiWindowFlags_MenuBar)) {

        if (ImGui::BeginMenuBar()) {

            if (ImGui::BeginMenu("Options")) {
                if (ImGui::MenuItem("Change Game Dir")) {
                    if (auto path = select_directory_save()) mThumperPath = path;
                }


                ImGui::MenuItem("Hash Panel", nullptr, &mShowHashPanel);
                ImGui::MenuItem("[!!!] Reset Settings [!!!]", nullptr, nullptr, false);

                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Help")) {
                ImGui::MenuItem("About...", nullptr, &mShowAboutPanel);

                if (ImGui::MenuItem("Discord Server", nullptr, nullptr, ImGui::GetCurrentContext()->PlatformIO.Platform_OpenInShellFn)) {
                    ImGui::GetCurrentContext()->PlatformIO.Platform_OpenInShellFn(ImGui::GetCurrentContext(), "https://discord.com/invite/gTQbquY");
                }

                if (ImGui::MenuItem("Github", nullptr, nullptr, ImGui::GetCurrentContext()->PlatformIO.Platform_OpenInShellFn)) {
                    ImGui::GetCurrentContext()->PlatformIO.Platform_OpenInShellFn(ImGui::GetCurrentContext(), "https://github.com/CocoaMix86/Thumper-Custom-Level-Editor");
                }

                if (ImGui::MenuItem("Donate & Tip (ko-fi)", nullptr, nullptr, ImGui::GetCurrentContext()->PlatformIO.Platform_OpenInShellFn)) {
                    ImGui::GetCurrentContext()->PlatformIO.Platform_OpenInShellFn(ImGui::GetCurrentContext(), "https://ko-fi.com/cocoamix");
                }

                ImGui::EndMenu();
            }

            ImGui::EndMenuBar();
        }


        ImGui::TextUnformatted("Mod Mode");
        ImGui::SameLine();

        if (mModMode) {
            ImGui::PushStyleColor(ImGuiCol_Button, { 154.0f / 255.0f, 205.0f / 255.0f , 50.0f / 255.0f, 1.0f });
            if (ImGui::SmallButton("is ON")) mModMode ^= true;
        }
        else {
            ImGui::PushStyleColor(ImGuiCol_Button, { 64.0f / 255.0f,0,0,1 });
            if (ImGui::SmallButton("is OFF")) mModMode ^= true;
        }
        ImGui::PopStyleColor();


        ImGui::BeginDisabled(!mModMode);
        ImGui::Button("Update Levels");
        ImGui::EndDisabled();
        ImGui::SetItemTooltip("%s", "Update Thumper with these levels and splash screen.\nAdding or removing levels requires a re-launch of the game.");

        ImGui::SeparatorText("Levels");

        if (ImGui::BeginTable("ModeLoaderLevlTable", 4, ImGuiTableFlags_BordersInner)) {

            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::TextUnformatted("Level Name");

            ImGui::TableNextColumn();
            ImGui::TextUnformatted("Difficulty");
            ImGui::SameLine();
            if (ImGui::SmallButton("?"))
                mShowDifficultyExplanation = true;

            ImGui::TableNextColumn();
            ImGui::TextUnformatted("Description");

            ImGui::TableNextColumn();
            ImGui::TextUnformatted("Author");

            for (auto const& level : mLevels) {
                ImGui::TableNextRow();

                ImGui::TableNextColumn();
                ImGui::TextUnformatted(level.name.c_str());

                ImGui::TableNextColumn();
                ImGui::TextUnformatted(level.difficulty.c_str());

                ImGui::TableNextColumn();
                ImGui::TextUnformatted(level.description.c_str());

                ImGui::TableNextColumn();
                ImGui::TextUnformatted(level.author.c_str());
            }

            ImGui::EndTable();
        }
    }
    ImGui::End();

    if (mShowDearImGuiDemo) ImGui::ShowDemoWindow(&mShowDearImGuiDemo);
}

int main(int argc, char** argv) {
    Application app;
    app.run();
    return EXIT_SUCCESS;
}