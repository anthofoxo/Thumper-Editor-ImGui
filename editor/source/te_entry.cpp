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
        tinyfd_messageBox("ERROR", "Config file not found! \nYou will need to re-select your Thumper executable.", "ok", "error", 1);
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

    std::array<GLuint, 8> diffTextures{};

    // Load difficulty texture
    for (int i = 0; i < diffTextures.size(); ++i) {
        std::string path = std::format("difficulty_icons/d{}.png", i);

        tcle::Image image(path.c_str());
        glCreateTextures(GL_TEXTURE_2D, 1, &diffTextures[i]);
        glTextureStorage2D(diffTextures[i], 1, GL_RGBA8, image.width(), image.height());
        glTextureSubImage2D(diffTextures[i], 0, 0, 0, image.width(), image.height(), GL_RGBA, GL_UNSIGNED_BYTE, image.pixels());
        glTextureParameteri(diffTextures[i], GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    }

    imgui_init(window);

    bool showHashPanel = false;
    bool showAboutPanel = false;
    bool showDifficultyExplanation = false;
    bool modMode = false;
    bool showDearImGuiDemo = false;
    bool secretEnabled = false;

    //bools to control what editor panels are open. 
    bool leafEditorShown = false;
    bool lvlEditorShown = false;
    bool gateEditorShown = false;
    bool masterEditorShown = false;
    bool workingFolderShown = false;
    bool sampleEditorShown = false;

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

        if (ImGui::Begin("Debug")) {
            std::string path = path_to_string(thumperPath.value());
            ImGui::LabelText("Thumper Path", "%s", path.c_str());
        }
        ImGui::End();

        hash_panel(showHashPanel);
        about_panel(iconTexture, showAboutPanel);

        if (showDifficultyExplanation) {
            if (ImGui::Begin("Difficulty Explanation", &showDifficultyExplanation, ImGuiWindowFlags_AlwaysAutoResize)) {
                if(ImGui::BeginTable("Difficulty Table", 3, ImGuiTableFlags_BordersInner | ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_NoSavedSettings)) {
                    //Set up the header columns
                    ImGui::TableNextRow();
                    ImGui::PushStyleColor(ImGuiCol_Text, { 1.0f, 1.0f, 1.0f, 1.0f });
                    ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, 0xFF808080);
                    ImGui::TableNextColumn();
                    ImGui::TextUnformatted("Difficulty\nRating");
                    ImGui::TableNextColumn();
                    ImGui::TextUnformatted("Description");
                    ImGui::TableNextColumn();
                    ImGui::TextUnformatted("Icon");

                    ImGui::PushStyleColor(ImGuiCol_Text, { 0.0f, 0.0f, 0.0f, 1.0f });

                    //D0
                    ImGui::TableNextRow();
                    ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, 0xFFDBEFE2);
                    ImGui::TableNextColumn();
                    ImGui::TextUnformatted("D0");
                    ImGui::TableNextColumn();
                    ImGui::TextUnformatted("EASY. Equivalent to early game");
                    ImGui::TableNextColumn();
                    ImGui::Image((ImTextureID)(uintptr_t)diffTextures[0], { 64.0f, 64.0f });
                    ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, 0xFF808080);

                    //D1
                    ImGui::TableNextRow();
                    ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, 0xFF8ECFA9);
                    ImGui::TableNextColumn();
                    ImGui::TextUnformatted("D1");
                    ImGui::TableNextColumn();
                    ImGui::TextUnformatted("NORMAL. Ranging from mid-game to Level 9.");
                    ImGui::TableNextColumn();
                    ImGui::Image((ImTextureID)(uintptr_t)diffTextures[1], { 64.0f, 64.0f });
                    ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, 0xFF808080);

                    //D2
                    ImGui::TableNextRow();
                    ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, 0xFF99E7FF);
                    ImGui::TableNextColumn();
                    ImGui::TextUnformatted("D2");
                    ImGui::TableNextColumn();
                    ImGui::TextUnformatted("INTERMEDIATE. Ranging from 9+ to harder than base game.");
                    ImGui::TableNextColumn();
                    ImGui::Image((ImTextureID)(uintptr_t)diffTextures[2], { 64.0f, 64.0f });
                    ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, 0xFF808080);

                    //D3
                    ImGui::TableNextRow();
                    ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, 0xFFADCAFA);
                    ImGui::TableNextColumn();
                    ImGui::TextUnformatted("D2");
                    ImGui::TableNextColumn();
                    ImGui::TextUnformatted("HARD. Much harder than the base game, hindered sight-reading.");
                    ImGui::TableNextColumn();
                    ImGui::Image((ImTextureID)(uintptr_t)diffTextures[3], { 64.0f, 64.0f });
                    ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, 0xFF808080);

                    //D4
                    ImGui::TableNextRow();
                    ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, 0xFFC2C1FF);
                    ImGui::TableNextColumn();
                    ImGui::TextUnformatted("D4");
                    ImGui::TableNextColumn();
                    ImGui::TextUnformatted("DAUNTING. Further-hindered sight-reading, some input intensity.");
                    ImGui::TableNextColumn();
                    ImGui::Image((ImTextureID)(uintptr_t)diffTextures[4], { 64.0f, 64.0f });
                    ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, 0xFF808080);

                    //D5
                    ImGui::TableNextRow();
                    ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, 0xFF7072FF);
                    ImGui::TableNextColumn();
                    ImGui::TextUnformatted("D5");

                    ImGui::PushStyleColor(ImGuiCol_Text, { 1.0f, 1.0f, 1.0f, 1.0f });

                    ImGui::TableNextColumn();
                    ImGui::TextUnformatted("EXTREME. Mostly input intensive, fast tempo, hindered sight reading.");
                    ImGui::TableNextColumn();
                    ImGui::Image((ImTextureID)(uintptr_t)diffTextures[5], { 64.0f, 64.0f });
                    ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, 0xFF808080);

                    //D6
                    ImGui::TableNextRow();
                    ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, 0xFF1013FD);
                    ImGui::TableNextColumn();
                    ImGui::PushStyleColor(ImGuiCol_Text, { 0.0f, 0.0f, 0.0f, 1.0f });
                    ImGui::TextUnformatted("D6");
                    ImGui::TableNextColumn();
                    ImGui::PushStyleColor(ImGuiCol_Text, { 1.0f, 1.0f, 1.0f, 1.0f });
                    ImGui::TextUnformatted("PAINFUL. Incredibly input intensive, barely sight readable.");
                    ImGui::TableNextColumn();
                    ImGui::Image((ImTextureID)(uintptr_t)diffTextures[6], { 64.0f, 64.0f });
                    ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, 0xFF808080);

                    //D7
                    ImGui::TableNextRow();
                    ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, 0xFF000000);
                    ImGui::TableNextColumn();
                    ImGui::TextUnformatted("D7");
                    ImGui::TableNextColumn();
                    ImGui::TextUnformatted("EXCRUCIATING. Unreadable, lightning fast, crazily input intensive, relying only on muscle memory through practice.");
                    ImGui::TableNextColumn();
                    ImGui::Image((ImTextureID)(uintptr_t)diffTextures[7], { 64.0f, 64.0f });
                    ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, 0xFF808080);

                    // No checkpoints
                    ImGui::TableNextRow();
                    ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, 0xFF262626);
                    ImGui::TableNextColumn();
                    ImGui::TextUnformatted("#");
                    ImGui::TableNextColumn();
                    ImGui::TextUnformatted("Indicates that the level has no checkpoints.");
                    ImGui::TableNextColumn();

                    // No checkpoints
                    ImGui::TableNextRow();
                    ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, 0xFF262626);
                    ImGui::TableNextColumn();
                    ImGui::TableNextColumn();
                    ImGui::TextUnformatted("Example: D4#");
                    ImGui::TableNextColumn();

                    ImGui::PopStyleColor(5);

                    ImGui::EndTable();
                }
                
                
            }
            ImGui::End();
            
        }
        
        if(ImGui::Begin("Thumper Level Editor v0.0.0.1", nullptr, ImGuiWindowFlags_MenuBar))
        {
            if(ImGui::BeginMenuBar())
            {
                if(ImGui::BeginMenu("File"))
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
                        if(ImGui::MenuItem("New", "ALT+N"))
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

                    if(ImGui::MenuItem("New Level Folder", "CTRL+L"))       // Fill background of button light green to highlight
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

                    if(ImGui::MenuItem("Exit"))
                    {
                        //Probably not needed anymore, but originally closes the entire editor program. Could probably re-use this to close the currently open level.
                    }












                    ImGui::EndMenu();
                }

                if(ImGui::BeginMenu("Edit"))
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
                    ImGui::MenuItem("Leaf Editor", NULL, &leafEditorShown);
                    ImGui::MenuItem("Lvl Editor", NULL, &lvlEditorShown);
                    ImGui::MenuItem("Gate Editor", NULL, &gateEditorShown);
                    ImGui::MenuItem("Master Editor", NULL, &masterEditorShown);
                    ImGui::MenuItem("Working Folder", NULL, &workingFolderShown);
                    ImGui::MenuItem("Sample Editor", NULL, &sampleEditorShown);

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
                        if (auto path = select_directory_save()) thumperPath = path;
                    }

                    ImGui::MenuItem("Enable Sekrit", nullptr, &secretEnabled, true);

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

    for (auto& texture : diffTextures) {
        glDeleteTextures(1, &texture);
    }

    glDeleteTextures(1, &iconTexture);

    imgui_uninit();

	glfwMakeContextCurrent(nullptr);
	
    window = {};

    audioEngine.uninit();

	return EXIT_SUCCESS;
}