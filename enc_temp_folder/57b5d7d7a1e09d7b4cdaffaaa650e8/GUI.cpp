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
EZ2SongDb::Song tempEditSong;
int currGame = EZ2SongDb::Games::FNEX;
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
           ImGui::OpenPopup("Open song.bin");
        }

        static int selectedGame = EZ2SongDb::Games::Last;

        if (ImGui::BeginPopupModal("Open song.bin", NULL, ImGuiWindowFlags_AlwaysAutoResize))
        {
            if (ImGui::BeginCombo("Game Version ##combo", EZ2SongDb::Games_names[selectedGame]))
            {
                for (int n = 0; n < EZ2SongDb::Games::Last+1; n++)
                {
                    bool is_selected = (selectedGame == n); 
                    if (ImGui::Selectable(EZ2SongDb::Games_names[n], is_selected)) {
                        selectedGame = n;
                    }
                    if (is_selected)
                        ImGui::SetItemDefaultFocus();
                }
                ImGui::EndCombo();
            }

            static bool saveDecrypt = false;
            ImGui::Checkbox("Save Decrypted File", &saveDecrypt);

            if (ImGui::Button("Open")) {
                nfdchar_t* fileFilter = "bin";
                currGame = selectedGame;
                nfdchar_t* outPath;
                nfdresult_t result = NFD_OpenDialog(fileFilter, NULL, &outPath);
                if (result == NFD_OKAY) {
                    currFilePath = outPath;
                    EZ2SongDb::openFile((LPCTSTR)outPath, &songList, currGame, saveDecrypt);
                    ImGui::CloseCurrentPopup();
                }
            }
            ImGui::SameLine();
            if (ImGui::Button("Cancel")) {
                ImGui::CloseCurrentPopup();
            }

            ImGui::EndPopup();
        }

   
        if (ImGui::MenuItem("Save", "CTRL+S")) { EZ2SongDb::SaveFile("songtest.bin", &songList, currGame, & songList); }
        if (ImGui::MenuItem("SaveAs", "CTRL+SHIFT+S")) {
            nfdchar_t* fileFilter = "bin";
            nfdchar_t* outPath = NULL;
            nfdresult_t result = NFD_SaveDialog(fileFilter, NULL, &outPath);
            if (result == NFD_OKAY) {
                EZ2SongDb::SaveFile(outPath, &songList, currGame);
                currFilePath = outPath;
            }
        }

        if (songList.numSongs> 1 && ImGui::MenuItem("Patch Executable")) {
            nfdchar_t* fileFilter = "exe";
            nfdchar_t* outPath;
            nfdresult_t result = NFD_OpenDialog(fileFilter, NULL, &outPath);
            if (result == NFD_OKAY) {
                currFilePath = outPath;
                EZ2SongDb::PatchEXE((LPCTSTR)outPath, &songList, currGame);
                ImGui::CloseCurrentPopup();
            }
        }

        ImGui::EndMainMenuBar();
    }

    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(use_work_area ? viewport->WorkPos : viewport->Pos);
    ImGui::SetNextWindowSize(use_work_area ? viewport->WorkSize : viewport->Size);


    if (ImGui::Begin("Menu", (bool*)true, flags))
    {
        if (songList.numSongs > 0) {
            ImGui::Text("Game Version: %s |", EZ2SongDb::Games_names[currGame]);
            ImGui::SameLine();
            ImGui::Text("Number of Songs: %d |", songList.numSongs);
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
                if (currGame != EZ2SongDb::Games::EV) {
                    if (ImGui::BeginTabItem("Categories"))
                    {
                        showCategoryView();
                        ImGui::EndTabItem();
                    }
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
            ImGui::Text("%d", i+1);
            ImGui::TableNextColumn();
            ImGui::Text("%s", songList.songs[i].name);
            ImGui::TableNextColumn();
            if (currGame >= EZ2SongDb::Games::FN) {
                ImGui::Text("%s", EZ2SongDb::FNGameVersion_names[songList.songs[i].gameVersion]);
            }else{
                ImGui::Text("%s", EZ2SongDb::PreFNGameVersion_names[songList.songs[i].gameVersion]);
            }
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
            std::string editSongLabel = ICON_FA_PENCIL"##" + std::to_string(i + 1);
            if (ImGui::Button(editSongLabel.c_str())){
                memcpy(&tempEditSong, &songList.songs[i], sizeof(tempEditSong));
                ImGui::OpenPopup(editSongLabel.c_str());

            }
            ImGui::SameLine();
            ImGui::Button(ICON_FA_TRASH);
            ImGui::TableNextColumn();


            //Edit Song Popup
            if (ImGui::BeginPopupModal(editSongLabel.c_str(), NULL, ImGuiWindowFlags_AlwaysAutoResize))
            {
                ImGui::InputText("Name", tempEditSong.name, 16);

                if (ImGui::BeginCombo("Game Version ##combo", EZ2SongDb::FNGameVersion_names[tempEditSong.gameVersion]))
                {
                    for (int n = 0; n < IM_ARRAYSIZE(EZ2SongDb::FNGameVersion_names); n++)
                    {
                        bool is_selected = (tempEditSong.gameVersion == n);
                        if (ImGui::Selectable(EZ2SongDb::FNGameVersion_names[n], is_selected)) {
                            tempEditSong.gameVersion = n;
                        }
                        if (is_selected)
                            ImGui::SetItemDefaultFocus();
                    }
                    ImGui::EndCombo();
                }

                ImGui::InputFloat("Minimum BPM", &tempEditSong.charts[0].minBPM, 10);
                if (tempEditSong.charts[0].minBPM < 0) {
                    tempEditSong.charts[0].minBPM = 0;
                }

                if (tempEditSong.charts[0].minBPM != tempEditSong.charts[1].minBPM) {
                    tempEditSong.charts[1].minBPM = tempEditSong.charts[0].minBPM;
                    tempEditSong.charts[2].minBPM = tempEditSong.charts[0].minBPM;
                    tempEditSong.charts[3].minBPM = tempEditSong.charts[0].minBPM;
                }

                ImGui::InputFloat("Maximum BPM", &tempEditSong.charts[0].maxBPM, 10);

                if (tempEditSong.charts[0].maxBPM > 999) {
                    tempEditSong.charts[0].maxBPM = 999;
                }

                if (tempEditSong.charts[0].maxBPM != tempEditSong.charts[1].maxBPM) {
                    tempEditSong.charts[1].maxBPM = tempEditSong.charts[0].maxBPM;
                    tempEditSong.charts[2].maxBPM = tempEditSong.charts[0].maxBPM;
                    tempEditSong.charts[3].maxBPM = tempEditSong.charts[0].maxBPM;
                }



                ImU8 steps = 1;
                ImGui::InputScalar("Normal Level", ImGuiDataType_U8, &tempEditSong.charts[0].level, &steps, NULL, "%d", { ImGuiInputTextFlags_CharsDecimal });
                if (tempEditSong.charts[0].level > 20) {
                    tempEditSong.charts[0].level = 20;
                }

                ImGui::InputScalar("Hard Level", ImGuiDataType_U8, &tempEditSong.charts[1].level, &steps, NULL, "%d", { ImGuiInputTextFlags_CharsDecimal });
                if (tempEditSong.charts[0].level > 20) {
                    tempEditSong.charts[0].level = 20;
                }

                ImGui::InputScalar("Super Hard Level", ImGuiDataType_U8, &tempEditSong.charts[2].level, &steps, NULL, "%d", { ImGuiInputTextFlags_CharsDecimal });
                if (tempEditSong.charts[0].level > 20) {
                    tempEditSong.charts[0].level = 20;
                }

                ImGui::InputScalar("EX Level", ImGuiDataType_U8, &tempEditSong.charts[3].level, &steps, NULL, "%d", { ImGuiInputTextFlags_CharsDecimal });
                if (tempEditSong.charts[0].level > 20) {
                    tempEditSong.charts[0].level = 20;
                }

                if (ImGui::Button("Save")) {
                    memcpy(&songList.songs[i], &tempEditSong, sizeof(tempEditSong));
                    ImGui::CloseCurrentPopup();
                }

                ImGui::SameLine();
                if (ImGui::Button("Cancel")) {
                    ImGui::CloseCurrentPopup();
                }
            ImGui::EndPopup();
            }
        }

        ImGui::EndTable();
    }


    if (ImGui::Button("Add Song")) {
        songList.numSongs++;
        songList.categoriesOffset += 0x56;
    }

    ImGui::SameLine();
    if (ImGui::Button("Sort By BPM")) {
        EZ2SongDb::sortByBPM(&songList);
    }
    ImGui::SameLine();
    if (ImGui::Button("Sort By Game Version")) {
        EZ2SongDb::sortByGameVer(&songList);
    }


    ImGui::EndChild();
}

void showCategoryView() {

    ImGui::BeginChild("catergory", { 0, ImGui::GetWindowHeight() - 75 }, false, ImGuiWindowFlags_HorizontalScrollbar);
    static int selectedCatergory = 0;
    if (currGame >= EZ2SongDb::Games::FN) {
        if (ImGui::BeginCombo("##combo", EZ2SongDb::FnCatergory_Names[selectedCatergory])) // The second parameter is the label previewed before opening the combo.
        {
            for (int n = 0; n < NUM_CATEGORIES; n++)
            {
                bool is_selected = (selectedCatergory == n); // You can store your selection however you want, outside or inside your objects
                if (ImGui::Selectable(EZ2SongDb::FnCatergory_Names[n], is_selected)) {
                    selectedCatergory = n;
                }
                if (is_selected)
                    ImGui::SetItemDefaultFocus();   // You may set the initial focus when opening the combo (scrolling + for keyboard navigation support)
            }
            ImGui::EndCombo();
        }
    }
    else {
        if (ImGui::BeginCombo("##combo", EZ2SongDb::PreFnCatergory_Names[selectedCatergory])) // The second parameter is the label previewed before opening the combo.
        {
            for (int n = 0; n < 11; n++)
            {
                bool is_selected = (selectedCatergory == n); // You can store your selection however you want, outside or inside your objects
                if (ImGui::Selectable(EZ2SongDb::PreFnCatergory_Names[n], is_selected)) {
                    selectedCatergory = n;
                }
                if (is_selected)
                    ImGui::SetItemDefaultFocus();   // You may set the initial focus when opening the combo (scrolling + for keyboard navigation support)
            }
            ImGui::EndCombo();
        }
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
            std::string editnameLabel= "##Name" + std::to_string(i);
            ImGui::InputText(editnameLabel.c_str(), songList.categories[selectedCatergory].songNames[i], 16);
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
    if (songList.categories[selectedCatergory].numSongs < 436) {
        if (ImGui::Button("Add Song")) {
            ImGui::SetScrollHereY(0.999f);
            songList.categories[selectedCatergory].numSongs++;
            //songList.categories[selectedCatergory].songNames[songList.categories[selectedCatergory].numSongs-1] = newSong;
        };
    }
    ImGui::Text("maximum number Of songs added to category");
    ImGui::SameLine();



    ImGui::EndChild();

}
