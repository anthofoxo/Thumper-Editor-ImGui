#include "te_diff_table.hpp"

#include <imgui.h>

namespace aurora {
	void gui_diff_table(bool& aOpen, std::array<GLuint, 8> const& aDiffTextures) {
        constexpr ImVec4 kBlack{ 0.0f, 0.0f, 0.0f, 1.0f };
        constexpr ImVec4 kWhite{ 1.0f, 1.0f, 1.0f, 1.0f };
        constexpr ImVec2 kIconSize{ 64.0f, 64.0f };

        if (!aOpen) return;

        if (!ImGui::Begin("Difficulty Explanation", &aOpen, ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::End();
            return;
        }
       
        if (!ImGui::BeginTable("Difficulty Table", 3, ImGuiTableFlags_BordersInner | ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_NoSavedSettings)) {
            ImGui::EndTable();
            ImGui::End();
            return;
        }

        //Set up the header columns
        ImGui::TableNextRow();
        ImGui::PushStyleColor(ImGuiCol_Text, kWhite);
        ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, 0xFF808080);
        ImGui::TableNextColumn();
        ImGui::TextUnformatted("Difficulty\nRating");
        ImGui::TableNextColumn();
        ImGui::TextUnformatted("Description");
        ImGui::TableNextColumn();
        ImGui::TextUnformatted("Icon");

        ImGui::PushStyleColor(ImGuiCol_Text, kBlack);

        //D0
        ImGui::TableNextRow();
        ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, 0xFFDBEFE2);
        ImGui::TableNextColumn();
        ImGui::TextUnformatted("D0");
        ImGui::TableNextColumn();
        ImGui::TextUnformatted("EASY. Equivalent to early game");
        ImGui::TableNextColumn();
        ImGui::Image((ImTextureID)(uintptr_t)aDiffTextures[0], kIconSize);
        ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, 0xFF808080);

        //D1
        ImGui::TableNextRow();
        ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, 0xFF8ECFA9);
        ImGui::TableNextColumn();
        ImGui::TextUnformatted("D1");
        ImGui::TableNextColumn();
        ImGui::TextUnformatted("NORMAL. Ranging from mid-game to Level 9.");
        ImGui::TableNextColumn();
        ImGui::Image((ImTextureID)(uintptr_t)aDiffTextures[1], kIconSize);
        ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, 0xFF808080);

        //D2
        ImGui::TableNextRow();
        ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, 0xFF99E7FF);
        ImGui::TableNextColumn();
        ImGui::TextUnformatted("D2");
        ImGui::TableNextColumn();
        ImGui::TextUnformatted("INTERMEDIATE. Ranging from 9+ to harder than base game.");
        ImGui::TableNextColumn();
        ImGui::Image((ImTextureID)(uintptr_t)aDiffTextures[2], kIconSize);
        ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, 0xFF808080);

        //D3
        ImGui::TableNextRow();
        ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, 0xFFADCAFA);
        ImGui::TableNextColumn();
        ImGui::TextUnformatted("D2");
        ImGui::TableNextColumn();
        ImGui::TextUnformatted("HARD. Much harder than the base game, hindered sight-reading.");
        ImGui::TableNextColumn();
        ImGui::Image((ImTextureID)(uintptr_t)aDiffTextures[3], kIconSize);
        ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, 0xFF808080);

        //D4
        ImGui::TableNextRow();
        ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, 0xFFC2C1FF);
        ImGui::TableNextColumn();
        ImGui::TextUnformatted("D4");
        ImGui::TableNextColumn();
        ImGui::TextUnformatted("DAUNTING. Further-hindered sight-reading, some input intensity.");
        ImGui::TableNextColumn();
        ImGui::Image((ImTextureID)(uintptr_t)aDiffTextures[4], kIconSize);
        ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, 0xFF808080);

        //D5
        ImGui::TableNextRow();
        ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, 0xFF7072FF);
        ImGui::TableNextColumn();
        ImGui::TextUnformatted("D5");

        ImGui::PushStyleColor(ImGuiCol_Text, kWhite);

        ImGui::TableNextColumn();
        ImGui::TextUnformatted("EXTREME. Mostly input intensive, fast tempo, hindered sight reading.");
        ImGui::TableNextColumn();
        ImGui::Image((ImTextureID)(uintptr_t)aDiffTextures[5], kIconSize);
        ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, 0xFF808080);

        //D6
        ImGui::TableNextRow();
        ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, 0xFF1013FD);
        ImGui::TableNextColumn();
        ImGui::PushStyleColor(ImGuiCol_Text, kBlack);
        ImGui::TextUnformatted("D6");
        ImGui::TableNextColumn();
        ImGui::PushStyleColor(ImGuiCol_Text, kWhite);
        ImGui::TextUnformatted("PAINFUL. Incredibly input intensive, barely sight readable.");
        ImGui::TableNextColumn();
        ImGui::Image((ImTextureID)(uintptr_t)aDiffTextures[6], kIconSize);
        ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, 0xFF808080);

        //D7
        ImGui::TableNextRow();
        ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, 0xFF000000);
        ImGui::TableNextColumn();
        ImGui::TextUnformatted("D7");
        ImGui::TableNextColumn();
        ImGui::TextUnformatted("EXCRUCIATING. Unreadable, lightning fast, crazily input intensive, relying only on muscle memory through practice.");
        ImGui::TableNextColumn();
        ImGui::Image((ImTextureID)(uintptr_t)aDiffTextures[7], kIconSize);
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
        ImGui::End();
	}
}