#include "EZ2SongDB.h"
#include <memoryapi.h>
#include <string>

  
//EZ2SongDb::SongList songList;
using namespace EZ2SongDb;



bool CipherBinData(LPVOID lpAddress, int dataSize, const char key1[], const char key2[]) {

    int counter = 0;
    int counter2 = 0;
    int counter3 = 0;

    if (dataSize <= 0) return false;

    do {
        BYTE* currByte = (BYTE*)lpAddress;
        currByte += counter;
        int value = (int)lpAddress;
        *currByte = key2[(UINT)(currByte + 1 - lpAddress) & 0x1f] ^ *currByte ^ (BYTE)counter;
        int uVar7 = 0;
        do {
            counter2 = uVar7;
            if (uVar7 == 0) {
                counter2 = 0xc;
            }
            counter3 = uVar7 + 1;
            *currByte = *currByte ^ key1[counter % counter2] ^ key1[uVar7] ^ key2[uVar7];
            if (counter3 == 0) {
                counter3 = 0xc;
            }
            counter2 = uVar7 + 2;
            *currByte = *currByte ^ key1[counter % counter3] ^ key2[uVar7 + 1] ^ key1[uVar7 + 1];
            if (counter2 == 0) {
                counter2 = 0xc;
            }
            counter3 = uVar7 + 3;
            *currByte = *currByte ^ key1[counter % counter2] ^ key2[uVar7 + 2] ^ key1[uVar7 + 2];
            if (counter3 == 0) {
                counter3 = 0xc;
            }
            counter2 = uVar7 + 4;
            *currByte = *currByte ^ key1[counter % counter3] ^ key2[uVar7 + 3] ^ key1[uVar7 + 3];
            uVar7 = counter2;
        } while (counter2 < 0x20);
        counter++;
    } while (counter < dataSize);

    return true;
}

bool saveMemoryRangeToFile(LPCTSTR filePath, LPVOID address, int size) {

    FILE* file = fopen(filePath, "wb");
    if (file == NULL) {
        perror("Error opening file");
        return false;
    }
    size_t bytes_written = fwrite(address, 1, size, file); // write the memory range to the file
    if (bytes_written != size) {
        perror("Error writing to file");
        return false;
    }

    fclose(file);
}

bool EZ2SongDb::openFile(LPCTSTR inputFile, EZ2SongDb::SongList* songList, int gameVer, bool saveDecrypt, bool decrypt) {

    char filePath[MAX_PATH];

    HANDLE hFile = CreateFile(inputFile, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        DWORD dwLastError = GetLastError();
        return false;
    }

    DWORD fileSize = GetFileSize(hFile, NULL);
    LPVOID lpAddress = VirtualAlloc(NULL, fileSize, MEM_COMMIT, PAGE_READWRITE);
    ReadFile(hFile, lpAddress, fileSize, &fileSize, NULL);
    CloseHandle(hFile);

    if (decrypt) {
        if (!CipherBinData(lpAddress, fileSize, keys1[gameVer], keys2[gameVer])) {
            return false;
        };
    }               

    if (lpAddress == NULL) {
        songList->numSongs = -1;
    }


    //save the unencrypted file
    if (saveDecrypt) {
        std::string savepath = std::string(inputFile) + ".dmp";
        saveMemoryRangeToFile(savepath.c_str(), lpAddress, fileSize);
    } //change to flag


    if (gameVer == EV) {
        memcpy(&songList->gameMode, (BYTE*)lpAddress + gameModeOffset, sizeof(uint8_t));
        memcpy(&songList->numSongs, (BYTE*)lpAddress + 0xA, sizeof(uint16_t));
        memcpy(&songList->songListOffset, (BYTE*)lpAddress + 0x6, sizeof(uint32_t));
    }else {
        memcpy(&songList->gameMode, (BYTE*)lpAddress + gameModeOffset, sizeof(uint8_t));
        memcpy(&songList->numSongs, (BYTE*)lpAddress + numSongOffset, sizeof(uint16_t));
        memcpy(&songList->songListOffset, (BYTE*)lpAddress + playlistNameSizeOffset, sizeof(uint32_t));
        memcpy(&songList->categoriesOffset, (BYTE*)lpAddress + songlistSizeOffset, sizeof(uint32_t));
    }
        


    //populate song list
    for (int i = 0; i < songList->numSongs; i++) {
        int offset = 0x56 * i;
        memcpy(&songList->songs[i].name, (BYTE*)lpAddress + songList->songListOffset + offset, sizeof(char[songNameSize]));
        memcpy(&songList->songs[i].gameVersion, (BYTE*)lpAddress + songList->songListOffset + gameVerOffset + offset, sizeof(uint16_t));

        for (int j = 0; j < numCharts; j++) {
            int chartOffset = 0x09 * j;

            memcpy(&songList->songs[i].charts[j].level, (BYTE*)lpAddress + songList->songListOffset + nmOffset + chartOffset + offset, sizeof(uint8_t));

            memcpy(&songList->songs[i].charts[j].minBPM, (BYTE*)lpAddress + songList->songListOffset + nmMinBpm + chartOffset + offset, sizeof(float));

            memcpy(&songList->songs[i].charts[j].maxBPM, (BYTE*)lpAddress + songList->songListOffset + nmMaxBpm + chartOffset + offset, sizeof(float));
        }
    }


    //populate categories
    if (gameVer == FN || gameVer == FNEX) {
        int offset = 0;
        for (int i = 0; i < NUM_CATEGORIES; i++) {
            memcpy(&songList->categories[i].numSongs, (BYTE*)lpAddress + songList->categoriesOffset + offset, sizeof(uint16_t));
            offset += 2;
            for (int j = 0; j < songList->categories[i].numSongs; j++) {
                char* name = (char*)malloc(16);
                memcpy(&songList->categories[i].songNames[j], (BYTE*)lpAddress + songList->categoriesOffset + offset, sizeof(char[16]));
                //songList->categories[i].songNames[j] = name;
                offset += (0x10);
            }
        }
    } else if(gameVer != EV) {
        int offset = 0;
        for (int i = 0; i < 11; i++) {
            memcpy(&songList->categories[i].numSongs, &songList->numSongs, sizeof(uint16_t));
            for (int j = 0; j < songList->categories[i].numSongs; j++) {
                char* name = (char*)malloc(16);
                memcpy(&songList->categories[i].songNames[j], (BYTE*)lpAddress + songList->categoriesOffset + offset, sizeof(char[16]));
                //songList->categories[i].songNames[j] = name;
                offset += (0x10);
            }
        }
    }

    return true;
}

bool EZ2SongDb::SaveFile(LPCTSTR savePath, EZ2SongDb::SongList *songList, int gameVer, bool encrypt) {


    //reinit songlist
    //memcpy(songList, &EmptySongList, sizeof(SongList));
    //Header section = 0x10
    //songlist section = 0x56 * numSongs
    //category section = 0x2+(0x10*numSongs) for each category
    int calculatedFileSize = 0x10 + (0x56 * songList->numSongs);
    for (int i = 0; i < NUM_CATEGORIES; i++) {
        calculatedFileSize += 0x02 + (0x10 * songList->categories[i].numSongs);
    }

    LPVOID lpAddress = VirtualAlloc(NULL, calculatedFileSize, MEM_COMMIT, PAGE_READWRITE);

    //build header always 16 bytes long.
    char headerText[] = "EZSL";
    memcpy((BYTE*)lpAddress, headerText, sizeof(headerText));
    memcpy((BYTE*)lpAddress + 0x4, &songList->EZSLByte, sizeof(uint8_t));
    memcpy((BYTE*)lpAddress + 0x5, &songList->gameMode, sizeof(uint8_t));
    memcpy((BYTE*)lpAddress + 0x6, &songList->numSongs, sizeof(uint16_t));
    memcpy((BYTE*)lpAddress + 0x8, &songList->songListOffset, sizeof(uint32_t));
    memcpy((BYTE*)lpAddress + 0xC, &songList->categoriesOffset, sizeof(uint32_t));

    //Populate songlist data in correct format
    for (int i = 0; i < songList->numSongs; i++) {
        int offset = 0x56 * i;
        memcpy((BYTE*)lpAddress + songList->songListOffset + offset, &songList->songs[i].name, sizeof(char[songNameSize]));
        memcpy((BYTE*)lpAddress + songList->songListOffset + gameVerOffset + offset, &songList->songs[i].gameVersion, sizeof(&songList->songs[i].gameVersion));

        for (int j = 0; j < numCharts; j++) {
            int chartOffset = 0x09 * j;

            memcpy((BYTE*)lpAddress + songList->songListOffset + nmOffset + chartOffset + offset, &songList->songs[i].charts[j].level, sizeof(uint8_t));

            memcpy((BYTE*)lpAddress + songList->songListOffset + nmMinBpm + chartOffset + offset, &songList->songs[i].charts[j].minBPM, sizeof(float));

            memcpy((BYTE*)lpAddress + songList->songListOffset + nmMaxBpm + chartOffset + offset, &songList->songs[i].charts[j].maxBPM, sizeof(float));
        }
    }

    //Populate categories in correct format
    int offset = 0;
    for (int i = 0; i < NUM_CATEGORIES; i++) {
        memcpy((BYTE*)lpAddress + songList->categoriesOffset + offset, &songList->categories[i].numSongs, sizeof(uint16_t));
        offset += 2;
        for (int j = 0; j < songList->categories[i].numSongs; j++) {
            memcpy((BYTE*)lpAddress + songList->categoriesOffset + offset, songList->categories[i].songNames[j], sizeof(char[16]));
            offset += (0x10);
        }
    }

    //re-encrypt the data
    if (encrypt) {
        CipherBinData(lpAddress, calculatedFileSize, keys1[gameVer], keys2[gameVer]);
    }

    //finally, save the data to a file
    saveMemoryRangeToFile(savePath, lpAddress, calculatedFileSize);

    return true;
}

bool EZ2SongDb::sortByBPM(SongList* songList) {
    int i, j;
    Song temp;

    for (i = 0; i < songList->numSongs-1; i++) {
        for (j = 0; j < songList->numSongs - i-1; j++) {
            if (songList->songs[j].charts[0].maxBPM > songList->songs[j + 1].charts[0].maxBPM) {
                temp = songList->songs[j];
                songList->songs[j] = songList->songs[j+1];
                songList->songs[j + 1] = temp;
            }
        }
    }
    return 0;
}

bool EZ2SongDb::sortByGameVer(SongList* songList) {
    int i, j;
    Song temp;

    for (i = 0; i < songList->numSongs - 1; i++) {
        for (j = 0; j < songList->numSongs - i - 1; j++) {
            if (songList->songs[j].gameVersion > songList->songs[j + 1].gameVersion) {
                temp = songList->songs[j];
                songList->songs[j] = songList->songs[j + 1];
                songList->songs[j + 1] = temp;
            }
        }
    }
    return 0;
}


bool EZ2SongDb::shiftSongDown(SongList* songList, int category, int songIndex)
{
    if (songIndex < songList->categories[category].numSongs-1) {
        char* temp = songList->categories[category].songNames[songIndex + 1];
        *songList->categories[category].songNames[songIndex + 1] = *songList->categories[category].songNames[songIndex];
        *songList->categories[category].songNames[songIndex] = *temp;
    }

    return false;
}

bool EZ2SongDb::shiftSongUp(SongList* songList, int category, int songIndex)
{
    if (songIndex > 0) {
        char* temp = songList->categories[category].songNames[songIndex-1];
        *songList->categories[category].songNames[songIndex - 1] = *songList->categories[category].songNames[songIndex];
        *songList->categories[category].songNames[songIndex] = *temp;
    }

    return false;
}

bool EZ2SongDb::PatchEXE(LPCTSTR exePath, SongList* songList, int currGame)
{


    // Open the original exe file
    FILE* exe = fopen(exePath, "r+");
    if (exe == NULL) {
        printf("Error opening exe file: %s\n", exePath);
        return 0;
    }

    uint32_t numSongs = 0x56 * songList->numSongs;
    uint32_t newOffset = 0xEFDF4;
    switch(currGame){
    case FNEX:

        // will possibly need to patch 69E70, this of og song count, but i think its just for category stuff

        //for normal
        fseek(exe, 0x69DC4, SEEK_SET);
        fwrite(&numSongs, sizeof(uint16_t), 1, exe);

        fseek(exe, 0x69DED, SEEK_SET);
        fwrite(&numSongs, sizeof(uint16_t), 1, exe);

        fseek(exe, 0x69F47, SEEK_SET);
        fwrite(&numSongs, sizeof(uint16_t), 1, exe);

        fseek(exe, 0x69F9A, SEEK_SET);
        fwrite(&numSongs, sizeof(uint16_t), 1, exe);

        //FOR CV2
        fseek(exe, 0x69EDA, SEEK_SET);
        fwrite(&numSongs, sizeof(uint16_t), 1, exe);

        fseek(exe, 0x69EFC, SEEK_SET);
        fwrite(&numSongs, sizeof(uint16_t), 1, exe);
            
        //for normals
        numSongs = 0x4 + 0x56 * songList->numSongs;
        fseek(exe, 0x69DFA, SEEK_SET);
        fwrite(&numSongs, sizeof(uint16_t), 1, exe);

        numSongs = 0xC0 + 0x56 * songList->numSongs;
        fseek(exe, 0x69E00, SEEK_SET);
        fwrite(&numSongs, sizeof(uint16_t), 1, exe);


        //00435090: -> 0x3789BD4 + 0x56 * numAddtionalSongs 
        numSongs = 0x3789BD4 +0x56 * (songList->numSongs - 436); //0x010FF435;
        fseek(exe, 0x35093, SEEK_SET);
        fwrite(&numSongs, sizeof(uint32_t), 1, exe);

        //00435110: -> 0x3789c90 + 0x56 * numAddtionalSongs 
        numSongs = 0x3789c90 + 0x56 * (songList->numSongs - 436); //0x0FFF435;
        fseek(exe, 0x35112, SEEK_SET);
        fwrite(&numSongs, sizeof(uint32_t), 1, exe);


        //Increase number of songs parsed during catergory init
        numSongs = songList->numSongs;
        fseek(exe, 0x35F54, SEEK_SET);
        fwrite(&songList->numSongs, sizeof(uint16_t), 1, exe);



        //These offsets completed relocate the data created when parsing song.ini
        //allows us to add more songs :))
        //uint32_t newOffset = 0xEFDF4
        //    0x3515B = 0x5e2bc + newOffset
        //    0x375B9 = 0x5e2bc + newOffset
        //    0x35118 = 0x5e3c8 + newOffset
        //    0x35F80
        //    0x360EF
        //    0x36198;

        newOffset = 0x5e2bc + 0xEFDF4;
        fseek(exe, 0x3515B, SEEK_SET);
        fwrite(&newOffset, sizeof(uint32_t), 1, exe);

        fseek(exe, 0x375B9, SEEK_SET);
        fwrite(&newOffset, sizeof(uint32_t), 1, exe);
        
        newOffset = 0x5e3c8 + 0xEFDF4;
        fseek(exe, 0x35118, SEEK_SET);
        fwrite(&newOffset, sizeof(uint32_t), 1, exe);

        fseek(exe, 0x35F80, SEEK_SET);
        fwrite(&newOffset, sizeof(uint32_t), 1, exe);

        fseek(exe, 0x360EF, SEEK_SET);
        fwrite(&newOffset, sizeof(uint32_t), 1, exe);

        fseek(exe, 0x36198, SEEK_SET);
        fwrite(&newOffset, sizeof(uint32_t), 1, exe);

        newOffset = 0x5E2C4 + 0xEFDF4;
        fseek(exe, 0x30F69, SEEK_SET);
        fwrite(&newOffset, sizeof(uint32_t), 1, exe);


        //Fixes for CV2
        fseek(exe, 0x394BA, SEEK_SET);
        fwrite(&newOffset, sizeof(uint32_t), 1, exe);

        newOffset = 0x5E62C + 0xEFDF4;
        fseek(exe, 0x39447, SEEK_SET);
        fwrite(&newOffset, sizeof(uint32_t), 1, exe);

        newOffset = 0x5E4D0 + 0xEFDF4;
        fseek(exe, 0x3946D, SEEK_SET);
        fwrite(&newOffset, sizeof(uint32_t), 1, exe);

        fseek(exe, 0x39301, SEEK_SET);
        fwrite(&newOffset, sizeof(uint32_t), 1, exe);

        newOffset = -(0x5E29C + 0xEFDF4);
        fseek(exe, 0x362B2, SEEK_SET);
        fwrite(&newOffset, sizeof(uint32_t), 1, exe);

        newOffset = -(0x5E298 + 0xEFDF4);
        fseek(exe, 0x362EF, SEEK_SET);
        fwrite(&newOffset, sizeof(uint32_t), 1, exe);


        break;
    default:
        break;
        //do nothing:
    }
 
    // Close both files
    fclose(exe);


    return 1;
}


