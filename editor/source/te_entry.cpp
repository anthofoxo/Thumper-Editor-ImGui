#ifdef TE_WINDOWS
#   include <Windows.h>
#endif

#include <glad/gl.h>
#include <GLFW/glfw3.h>

#include "te_audio.hpp"
#include "te_window.hpp"
#include "te_image.hpp"

#include <stb_image.h>
#include <miniaudio.h>

#include <tinyfiledialogs.h>
#include <yaml-cpp/yaml.h>

#include <imgui.h>
#include <imgui_internal.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include <misc/cpp/imgui_stdlib.h>

#include <cstdlib> // EXIT_SUCCESS, EXIT_FAILURE
#include <stdio.h>
#include <string>
#include <vector>
#include <filesystem>
#include <fstream>
#include <memory>

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
        if (ImGui::Begin("About Thumper Mod Loader v2.0.0.0", &open)) {
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
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

    ImGui::StyleColorsClassic();

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

int main(int argc, char** argv) {
    // Read configs
    std::optional<std::filesystem::path> thumperPath = std::nullopt;

    try {
        YAML::Node rootNode = YAML::LoadFile("config.yaml");
        std::string str = rootNode["path"].as<std::string>("");

        if (!str.empty()) {
            thumperPath = std::filesystem::path(str);
        }
    }
    catch (YAML::BadFile const&) {
        // Do nothing, use defaults
    }

    // If invalid, request path
    if (!thumperPath) {
        thumperPath = select_directory_save();
        if (!thumperPath) return EXIT_FAILURE;
    }

    tcle::Window window({
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
        
        glfwSetWindowIcon(window, 1, &image);
    }

    glfwShowWindow(window);

	glfwMakeContextCurrent(window);

	gladLoadGL(&glfwGetProcAddress);

    AudioEngine audioEngine;
    audioEngine.init();
    ma_engine_play_sound(&audioEngine.mEngine, "UIBoot.ogg", nullptr);

    // Load larger icon, seen in the about panel
    GLuint iconTexture;
    {
        tcle::Image image("thumper_modding_tool.png");
        glCreateTextures(GL_TEXTURE_2D, 1, &iconTexture);
        glTextureStorage2D(iconTexture, 1, GL_RGBA8, image.width(), image.height());
        glTextureSubImage2D(iconTexture, 0, 0, 0, image.width(), image.height(), GL_RGBA, GL_UNSIGNED_BYTE, image.pixels());
        glTextureParameteri(iconTexture, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    }

    // Difficulty ranking texture, Should be converted to an imgui table
    GLuint diffTexture;
    {
        tcle::Image image("diff.png");
        glCreateTextures(GL_TEXTURE_2D, 1, &diffTexture);
        glTextureStorage2D(diffTexture, 1, GL_RGBA8, image.width(), image.height());
        glTextureSubImage2D(diffTexture, 0, 0, 0, image.width(), image.height(), GL_RGBA, GL_UNSIGNED_BYTE, image.pixels());
        glTextureParameteri(diffTexture, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    }

    imgui_init(window);

    bool showHashPanel = false;
    bool showAboutPanel = false;
    bool showDifficultyExplanation = false;
    bool modMode = false;
    bool showDearImGuiDemo = false;

    std::vector<Level> levels;

    for (auto& entry : std::filesystem::directory_iterator("levels")) {
        if (!entry.is_directory()) continue;
        std::filesystem::path path = entry.path() / "LEVEL DETAILS.txt";
        if (!std::filesystem::exists(path)) continue;
    
        YAML::Node node = YAML::LoadFile(path_to_string(path));

        levels.emplace_back(
            node["level_name"].as<std::string>(""),
            node["difficulty"].as<std::string>(""),
            node["description"].as<std::string>(""),
            node["author"].as<std::string>("")
        );
    }

	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();

#ifdef TE_WINDOWS
        ImGui::GetIO().ConfigDebugIsDebuggerPresent = ::IsDebuggerPresent();
#endif

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        if (ImGui::BeginMainMenuBar()) {
            if (ImGui::BeginMenu("View")) {
                ImGui::MenuItem("Dear ImGui Demo", nullptr, &showDearImGuiDemo);

                ImGui::EndMenu();
            }

            ImGui::EndMainMenuBar();
        }

        hash_panel(showHashPanel);
        about_panel(iconTexture, showAboutPanel);

        if (showDifficultyExplanation) {
            if (ImGui::Begin("Difficulty Explanation", &showDifficultyExplanation)) {
                ImGui::Image((ImTextureID)(uintptr_t)diffTexture, ImGui::GetContentRegionAvail());
            }
            ImGui::End();
        }

        if (ImGui::Begin("Thumper Mod Loader v2.0.0.0", nullptr, ImGuiWindowFlags_MenuBar)) {

            if (ImGui::BeginMenuBar()) {

                if (ImGui::BeginMenu("Options")) {
                    if (ImGui::MenuItem("Change Game Dir")) {
                        if (auto path = select_directory_save()) thumperPath = path;
                    }

                    ImGui::MenuItem("Hash Panel", nullptr, &showHashPanel);
                    ImGui::MenuItem("[!!!] Reset Settings [!!!]", nullptr, nullptr, false);

                    ImGui::EndMenu();
                }

                if (ImGui::BeginMenu("Help")) {
                    ImGui::MenuItem("About...", nullptr, &showAboutPanel);
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

            if (modMode) {
                ImGui::PushStyleColor(ImGuiCol_Button, { 154.0f / 255.0f, 205.0f / 255.0f , 50.0f / 255.0f, 1.0f });
                if (ImGui::SmallButton("is ON")) modMode ^= true;
            }
            else {
                ImGui::PushStyleColor(ImGuiCol_Button, { 64.0f/255.0f,0,0,1 });
                if (ImGui::SmallButton("is OFF")) modMode ^= true;
            }
            ImGui::PopStyleColor();

            
            ImGui::BeginDisabled(!modMode);
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
                    showDifficultyExplanation = true;

                ImGui::TableNextColumn();
                ImGui::TextUnformatted("Description");

                ImGui::TableNextColumn();
                ImGui::TextUnformatted("Author");

                for (auto const& level : levels) {
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

        if(showDearImGuiDemo)
            ImGui::ShowDemoWindow(&showDearImGuiDemo);

        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.3f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
            GLFWwindow* backup_current_context = glfwGetCurrentContext();
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
            glfwMakeContextCurrent(backup_current_context);
        }

		glfwSwapBuffers(window);
	}

    glDeleteTextures(1, &diffTexture);
    glDeleteTextures(1, &iconTexture);

    imgui_uninit();

	glfwMakeContextCurrent(nullptr);
	
    window = {};

    audioEngine.uninit();

	return EXIT_SUCCESS;
}