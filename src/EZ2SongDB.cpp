#include "EZ2SongDB.h"
#include <memoryapi.h>

  
//EZ2SongDb::SongList songList;
using namespace EZ2SongDb;

const char fnexKEY[] = "\xF3\x1A\x83\x3B\xAA\xD9\x46\x5B\x71\x3D\x16\xB9\xF2\x8D\x1F\x80\xD0\x39\x64\xAC\x30\xB6\x84\x40\xBF\xC0\x8A\x31\x12\xB1\x39\x19";
const char fnexKEY2[] = "\x7D\x5D\x4F\x97\x5B\x29\xC4\x08\xB1\x77\x46\xA4\x40\x98\x8E\xE9\xCA\x43\xFE\x25\x9F\x28\x0D\x9E\x7B\x07\xDE\x84\xC0\x25\x45\x9C";


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

bool EZ2SongDb::openFile(LPCTSTR inputFile, EZ2SongDb::SongList* songList, bool decrypt) {

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
        if (!CipherBinData(lpAddress, fileSize, fnexKEY, fnexKEY2)) {
            return false;
        };
    }               

    if (lpAddress == NULL) {
        songList->numSongs = -1;
    }

    memcpy(&songList->gameMode, (BYTE*)lpAddress + gameModeOffset, sizeof(uint8_t));
    memcpy(&songList->numSongs, (BYTE*)lpAddress + numSongOffset, sizeof(uint16_t));
    memcpy(&songList->songListOffset, (BYTE*)lpAddress + playlistNameSizeOffset, sizeof(uint32_t));
    memcpy(&songList->categoriesOffset, (BYTE*)lpAddress + songlistSizeOffset, sizeof(uint32_t));


    //populate song list
    for (int i = 0; i < songList->numSongs; i++) {
        int offset = 0x56 * i;
        memcpy(&songList->songs[i].name, (BYTE*)lpAddress + songlistOffset + offset, sizeof(char[songNameSize]));
        memcpy(&songList->songs[i].gameVersion, (BYTE*)lpAddress + gameVerOffset + offset, sizeof(uint16_t));

        for (int j = 0; j < numCharts; j++) {
            int chartOffset = 0x09 * j;

            memcpy(&songList->songs[i].charts[j].level, (BYTE*)lpAddress + nmOffset + chartOffset + offset, sizeof(uint8_t));

            memcpy(&songList->songs[i].charts[j].minBPM, (BYTE*)lpAddress + nmMinBpm + chartOffset + offset, sizeof(float));

            memcpy(&songList->songs[i].charts[j].maxBPM, (BYTE*)lpAddress + nmMaxBpm + chartOffset + offset, sizeof(float));
        }
    }


    //populate categories
    int offset = 0;
    for (int i = 0; i < NUM_CATEGORIES; i++) {
        memcpy(&songList->categories[i].numSongs, (BYTE*)lpAddress + songList->categoriesOffset + offset, sizeof(uint16_t));
        offset += 2;
        for (int j = 0; j < songList->categories[i].numSongs; j++) {
            char *name = (char*)malloc(16);
            memcpy(name, (BYTE*)lpAddress + songList->categoriesOffset+ offset, sizeof(char[16]));
            songList->categories[i].songNames[j] = name;
            offset += (0x10);
        }
    }

    return true;
}

bool EZ2SongDb::SaveFile(LPCTSTR savePath, EZ2SongDb::SongList *songList, bool encrypt) {

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
        memcpy((BYTE*)lpAddress + gameVerOffset + offset, &songList->songs[i].gameVersion, sizeof(&songList->songs[i].gameVersion));

        for (int j = 0; j < numCharts; j++) {
            int chartOffset = 0x09 * j;

            memcpy((BYTE*)lpAddress + nmOffset + chartOffset + offset, &songList->songs[i].charts[j].level, sizeof(uint8_t));

            memcpy((BYTE*)lpAddress + nmMinBpm + chartOffset + offset, &songList->songs[i].charts[j].minBPM, sizeof(float));

            memcpy((BYTE*)lpAddress + nmMaxBpm + chartOffset + offset, &songList->songs[i].charts[j].maxBPM, sizeof(float));
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
        CipherBinData(lpAddress, calculatedFileSize, fnexKEY, fnexKEY2);
    }

    //Finally, save the file..
    FILE* file = fopen(savePath, "wb");
    if (file == NULL) {
        perror("Error opening file");
        return false;
    }
    size_t bytes_written = fwrite(lpAddress, 1, calculatedFileSize, file); // write the memory range to the file
    if (bytes_written != calculatedFileSize) {
        perror("Error writing to file");
    }

    fclose(file);
    return true;
}

bool EZ2SongDb::shiftSongDown(SongList* songList, int category, int songIndex)
{
    if (songIndex < songList->categories[category].numSongs-1) {
        char* temp = songList->categories[category].songNames[songIndex + 1];
        songList->categories[category].songNames[songIndex + 1] = songList->categories[category].songNames[songIndex];
        songList->categories[category].songNames[songIndex] = temp;
    }

    return false;
}

bool EZ2SongDb::shiftSongUp(SongList* songList, int category, int songIndex)
{
    if (songIndex > 0) {
        char* temp = songList->categories[category].songNames[songIndex-1];
        songList->categories[category].songNames[songIndex - 1] = songList->categories[category].songNames[songIndex];
        songList->categories[category].songNames[songIndex] = temp;
    }

    return false;
}


