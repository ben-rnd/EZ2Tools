#include "GUI.h"
#include "EZ2SongDB.h"
#include "nfd.h"
#include <stdio.h>
#define GL_SILENCE_DEPRECATION
#if defined(IMGUI_IMPL_OPENGL_ES2)
#include <GLES2/gl2.h>
#endif
#include <GLFW/glfw3.h> // Will drag system OpenGL headers
#include <string>
#include "IconsFontAwesome4.h"


using namespace GUI;

// --------------------------------------------------------------------
// Data and globals
// --------------------------------------------------------------------
EZ2SongDb::SongList songList;
LPCTSTR currFilePath;


// --------------------------------------------------------------------
// Local Function Declaration
// --------------------------------------------------------------------
void showSongListView();
void showCategoryView();

/// <summary>
/// Song List
/// </summary>
/// <param name="window"></param>
/// <returns></returns>
int GUI::RenderUI(GLFWwindow* window)
{
    static bool use_work_area = true;
    static ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings;

    if (ImGui::BeginMainMenuBar())
    {
            
            if (ImGui::MenuItem("Open", "CTRL+O")) {
                nfdchar_t* outPath = NULL;
                nfdresult_t result = NFD_OpenDialog(NULL, NULL, &outPath);
                if (result == NFD_OKAY) {
                    currFilePath = outPath;
                    EZ2SongDb::openFile((LPCTSTR)outPath, &songList);
                }
            }
            if (ImGui::MenuItem("Save", "CTRL+S")) { EZ2SongDb::SaveFile("songtest.bin", & songList); }
            if (ImGui::MenuItem("SaveAs", "CTRL+SHIFT+S")) {
                nfdchar_t* outPath = NULL;
                nfdresult_t result = NFD_SaveDialog(NULL, NULL, &outPath);
                if (result == NFD_OKAY) {
                    EZ2SongDb::SaveFile(outPath, &songList);
                    currFilePath = outPath;
                }
            }

        ImGui::EndMainMenuBar();
    }

    // We demonstrate using the full viewport area or the work area (without menu-bars, task-bars etc.)
    // Based on your use case you may want one of the other.
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(use_work_area ? viewport->WorkPos : viewport->Pos);
    ImGui::SetNextWindowSize(use_work_area ? viewport->WorkSize : viewport->Size);


    if (ImGui::Begin("Menu", (bool*)true, flags))
    {
        if (songList.numSongs > 0) {
            ImGui::Text("Number of Songs: %d", songList.numSongs);
            ImGui::SameLine();
            ImGui::Text("Game Mode: %s", EZ2SongDb::GameMode_names[songList.gameMode]);


            if (ImGui::BeginTabBar("##Tabs", ImGuiTabBarFlags_None))
            {
                //Begin settings tab
                if (ImGui::BeginTabItem("Song List"))
                {
                    showSongListView();
                    ImGui::EndTabItem();
                }

                //Begin Buttons Tab
                if (ImGui::BeginTabItem("Categories"))
                {
                    showCategoryView();
                    ImGui::EndTabItem();
                }

                ImGui::EndTabBar();
            }
        }
    }
    ImGui::Text("");
    ImGui::SameLine(ImGui::GetWindowWidth() - 170);
    ImGui::Text("Made by kasaski - 2023");
    ImGui::End();

    return 0;
}

void showSongListView()
{
    ImGui::BeginChild("Songs", { 0, ImGui::GetWindowHeight() - 75 }, false, ImGuiWindowFlags_HorizontalScrollbar);

    static ImGuiTableFlags flags =
        ImGuiTableFlags_Resizable | ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersOuter
        | ImGuiTableFlags_BordersV | ImGuiTableFlags_NoBordersInBody | ImGuiTableFlags_ScrollY;

    if (ImGui::BeginTable("songTable", 10, flags, { 0, ImGui::GetWindowHeight() - 25 }) )
    {
        ImGui::TableSetupColumn("Index", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoResize , 35.0f);
        ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoResize, 120.0f);
        ImGui::TableSetupColumn("Version");
        ImGui::TableSetupColumn("Min BPM");
        ImGui::TableSetupColumn("Max BPM");
        ImGui::TableSetupColumn("NM");
        ImGui::TableSetupColumn("SHD");
        ImGui::TableSetupColumn("HD");
        ImGui::TableSetupColumn("EX");
        ImGui::TableSetupScrollFreeze(0, 1); // Make row always visible
        ImGui::TableHeadersRow();
        ImGui::TableNextColumn();


        for (int i = 0; i < songList.numSongs; i++) {
            /*std::string songLabel = "##songName" + std::to_string(i + 1);
            ImGui::PushItemWidth(-1);
            ImGui::InputText(songLabel.c_str(), songList.songs[i].name, sizeof(char[16]));
            ImGui::PopItemWidth();*/
            ImGui::Text("%d", i+1);
            ImGui::TableNextColumn();
            ImGui::Text("%s", songList.songs[i].name);
            ImGui::TableNextColumn();
            ImGui::Text("%s", EZ2SongDb::GameVersion_names[songList.songs[i].gameVersion]);
            ImGui::TableNextColumn();
            ImGui::Text("%.5g", songList.songs[i].charts[0].minBPM);
            ImGui::TableNextColumn();
            ImGui::Text("%.5g", songList.songs[i].charts[0].maxBPM);
            ImGui::TableNextColumn();

            ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(100, 149, 237, 255));
            ImGui::Text("%d", songList.songs[i].charts[0].level);
            ImGui::PopStyleColor();
            ImGui::TableNextColumn();

            ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(250, 128, 114, 255));
            ImGui::Text("%d", songList.songs[i].charts[1].level);
            ImGui::PopStyleColor();
            ImGui::TableNextColumn();

            ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(238, 130, 238, 255));
            ImGui::Text("%d", songList.songs[i].charts[2].level);
            ImGui::PopStyleColor();
            ImGui::TableNextColumn();

            ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(147, 112, 219, 255));
            ImGui::Text("%d", songList.songs[i].charts[3].level);
            ImGui::PopStyleColor();
            ImGui::TableNextColumn();
            ImGui::Button(ICON_FA_PENCIL);
            ImGui::SameLine();
            ImGui::Button(ICON_FA_TRASH);
            ImGui::TableNextColumn();
        }

        ImGui::EndTable();
    }
    ImGui::Button("Add Song");

    ImGui::EndChild();
}

void showCategoryView() {

    ImGui::BeginChild("catergory", { 0, ImGui::GetWindowHeight() - 75 }, false, ImGuiWindowFlags_HorizontalScrollbar);
    static int selectedCatergory = 0;
    if (ImGui::BeginCombo("##combo", EZ2SongDb::Catergory_Names[selectedCatergory])) // The second parameter is the label previewed before opening the combo.
    {
        for (int n = 0; n < NUM_CATEGORIES; n++)
        {
            bool is_selected = (selectedCatergory == n); // You can store your selection however you want, outside or inside your objects
            if (ImGui::Selectable(EZ2SongDb::Catergory_Names[n], is_selected)) {
                selectedCatergory = n;
            }
            if (is_selected)
                ImGui::SetItemDefaultFocus();   // You may set the initial focus when opening the combo (scrolling + for keyboard navigation support)
        }
        ImGui::EndCombo();
    }
    ImGui::SameLine();
    ImGui::Text("(Songs: %d)", songList.categories[selectedCatergory].numSongs);

    static ImGuiTableFlags flags =
        ImGuiTableFlags_Resizable | ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersOuter
        | ImGuiTableFlags_BordersV | ImGuiTableFlags_NoBordersInBody | ImGuiTableFlags_ScrollY;

    if (ImGui::BeginTable("songTable", 2, flags, { 0, ImGui::GetWindowHeight() - 55 }))
    {
        ImGui::TableSetupColumn("Index", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoResize, 50.0f);
        ImGui::TableSetupColumn("Name");
        ImGui::TableHeadersRow();
        ImGui::TableNextColumn();

        for (int i = 0; i < songList.categories[selectedCatergory].numSongs; i++) {
            ImGui::Text("%d", i+1);
            ImGui::TableNextColumn();
            ImGui::Text(songList.categories[selectedCatergory].songNames[i]);
            ImGui::SameLine(ImGui::GetWindowWidth() - 170);

            std::string shiftUpLabel = ICON_FA_ARROW_UP"##" + std::to_string(selectedCatergory) + std::to_string(i + 1);
            if (ImGui::Button(shiftUpLabel.c_str())) {
                EZ2SongDb::shiftSongUp(&songList, selectedCatergory, i);
            }
            ImGui::SameLine();
            std::string shiftDownLabel = ICON_FA_ARROW_DOWN"##" + std::to_string(selectedCatergory) + std::to_string(i + 1);
            if (ImGui::Button(shiftDownLabel.c_str())) {
                EZ2SongDb::shiftSongDown(&songList, selectedCatergory, i);
            };
            ImGui::SameLine();
            ImGui::Button(ICON_FA_TRASH);
            ImGui::TableNextColumn();
        }

        ImGui::EndTable();
    }
    ImGui::Button("Add Song");
    ImGui::SameLine();



    ImGui::EndChild();

}
