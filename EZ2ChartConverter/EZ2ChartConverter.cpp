// EZ2ChartConverter.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#define _CRT_SECURE_NO_WARNINGS
#include <filesystem>
#include <iostream>
#include <cstdint>
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <memoryapi.h>
#include <string>

using namespace std;


//new note data length 0xD
//old length 0x0B
struct Note {
    UINT32 position;
    UINT8 NoteType = 0x01; //0x1= note, 0x2=volume , if 0x2, next byte is volume.
    UINT16 noteValue;
    UINT8 velocity;
    UINT8 panning;
    UINT8 unknown;
    UINT16 noteLength; //usuall 0x06
    UINT8 unknown2 = 0x0; //new byte for EV, dunno.
};

struct oldNote {
    UINT32 position;
    UINT8 NoteType = 0x01; //0x1= note, 0x2=volume , if 0x2, next byte is volume.
    UINT8 keySoundIDOrVolume;
    UINT8 velocity;
    UINT8 panning;
    UINT16 noteLength; //usuall 0x06
};

struct Lane {
    char laneName[0x40];
    UINT32 laneTicks;
    UINT32 sizeOfNoteData;
    Note notes[1000];
    int numNotes; //iterated at parsing 
};

struct Chart {
    UINT8 EZFFVer = 0x08;
    UINT16 ticksPerMeasure;
    float BPM;
    UINT32 totalTicks;
    float OtherBPM; // no clue.. lol
    UINT16 numLanes; //max 64 (0x40)
    Lane lanes[64];
    int NewSize;
};



bool saveMemoryRangeToFile(LPCTSTR filePath, LPVOID address, int size) {

    FILE* file = _wfopen(filePath, L"wb");
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


bool endsWith(std::wstring const& str, std::wstring const& suffix) {
    if (str.length() < suffix.length()) {
        return false;
    }
    return str.compare(str.length() - suffix.length(), suffix.length(), suffix) == 0;
}

std::wstring ExePath() {
    TCHAR buffer[MAX_PATH] = { 0 };
    GetModuleFileName(NULL, buffer, MAX_PATH);
    std::string::size_type pos = std::wstring(buffer).find_last_of(L"\\/");
    return std::wstring(buffer).substr(0, pos);
}


void addAntiTamperByteAndSave(LPWSTR ezFileName) {

    if(endsWith(ezFileName, L".ez")){
        wstring fileName = ExePath().append(L"\\");
        fileName.append(ezFileName);
        HANDLE hFile = CreateFileW(fileName.c_str() , GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hFile == INVALID_HANDLE_VALUE) {
            DWORD dwLastError = GetLastError();
            return;
        }

        DWORD fileSize = GetFileSize(hFile, NULL);
        LPVOID lpAddress = VirtualAlloc(NULL, fileSize, MEM_COMMIT, PAGE_READWRITE);
        if (!ReadFile(hFile, lpAddress, fileSize, &fileSize, NULL)) {
            return;
        }

        CloseHandle(hFile);

        int i = 0;
        while (i < fileSize) {
            DWORD offset1 = 0x1F8;
            DWORD offset2 = 0x400;
            DWORD offset3 = 0x5F0;


            BYTE* currByte;

            if (offset1 + i < fileSize) {
                currByte = (BYTE*)lpAddress + offset1 + i;
                *currByte = *currByte + 0xf9;
            }
            
            if (offset2+ i < fileSize) {
                currByte = (BYTE*)lpAddress + offset2 + i;
                *currByte = *currByte + 0xf9;
            }

            if (offset3 + i < fileSize) {
                currByte = (BYTE*)lpAddress + offset3 + i;
                *currByte = *currByte + 0xf9;
            }

            i += 0x600;

        }

        saveMemoryRangeToFile(fileName.c_str(), lpAddress, fileSize);
    }

}

Chart chartToConvert;


void ConvertToNew() {
    LPVOID lpAddress = VirtualAlloc(NULL, chartToConvert.NewSize, MEM_COMMIT, PAGE_READWRITE);

    if (lpAddress != 0) {
        char headerText[] = "EZFF";
        UINT8 newVer = 0x8;
        memcpy((BYTE*)lpAddress, &headerText, sizeof(headerText));
        memcpy((BYTE*)lpAddress + 0x5, &newVer, sizeof(UINT8));


        memcpy((BYTE*)lpAddress + 0x86, &chartToConvert.ticksPerMeasure, sizeof(UINT16));
        memcpy((BYTE*)lpAddress + 0x88, &chartToConvert.BPM, sizeof(float));
        memcpy((BYTE*)lpAddress + 0x8C, &chartToConvert.numLanes, sizeof(UINT16));
        memcpy((BYTE*)lpAddress + 0x8E, &chartToConvert.totalTicks, sizeof(UINT32));
        memcpy((BYTE*)lpAddress + 0x92, &chartToConvert.OtherBPM, sizeof(float));



        //start Lanes
        int offset = 0x96;
        for (UINT16 x = 0; x < chartToConvert.numLanes; x++) {
            char laneText[] = "EZTR";
            memcpy((BYTE*)lpAddress + offset, &laneText, sizeof(laneText));
            memcpy((BYTE*)lpAddress + offset + 6, &chartToConvert.lanes[x].laneName, sizeof(char[0x40]));
            memcpy((BYTE*)lpAddress + offset + 0x46, &chartToConvert.lanes[x].laneTicks, sizeof(UINT32));

            UINT32 newSizeNoteData = chartToConvert.lanes[x].numNotes * 0xD;
            memcpy((BYTE*)lpAddress + offset + 0x4A, &newSizeNoteData, sizeof(UINT32));

            UINT32 y = 0;
            int counter = 0;
            while (y < newSizeNoteData) {
                UINT32 noteOffset = y + 0x4E;
                memcpy((BYTE*)lpAddress + offset + noteOffset, &chartToConvert.lanes[x].notes[counter].position, sizeof(UINT32));
                memcpy((BYTE*)lpAddress + offset + noteOffset + 0x4, &chartToConvert.lanes[x].notes[counter].NoteType, sizeof(UINT8));
                if (chartToConvert.lanes[x].notes[counter].NoteType == 0x3) {
                    memcpy((BYTE*)lpAddress + offset + noteOffset + 0x5, &chartToConvert.lanes[x].notes[counter].noteValue, sizeof(UINT8));
                    memcpy((BYTE*)lpAddress + offset + noteOffset + 0x6, &chartToConvert.lanes[x].notes[counter].velocity, sizeof(UINT8));
                    memcpy((BYTE*)lpAddress + offset + noteOffset + 0x7, &chartToConvert.lanes[x].notes[counter].panning, sizeof(UINT8));
                    memcpy((BYTE*)lpAddress + offset + noteOffset + 0x8, &chartToConvert.lanes[x].notes[counter].unknown, sizeof(UINT8));
                    memcpy((BYTE*)lpAddress + offset + noteOffset + 0x9, &chartToConvert.lanes[x].notes[counter].noteLength, sizeof(UINT16));
                }else{
                    memcpy((BYTE*)lpAddress + offset + noteOffset + 0x5, &chartToConvert.lanes[x].notes[counter].noteValue, sizeof(UINT16));
                    memcpy((BYTE*)lpAddress + offset + noteOffset + 0x7, &chartToConvert.lanes[x].notes[counter].velocity, sizeof(UINT8));
                    memcpy((BYTE*)lpAddress + offset + noteOffset + 0x8, &chartToConvert.lanes[x].notes[counter].panning, sizeof(UINT8));
                    memcpy((BYTE*)lpAddress + offset + noteOffset + 0x9, &chartToConvert.lanes[x].notes[counter].unknown, sizeof(UINT8));
                    memcpy((BYTE*)lpAddress + offset + noteOffset + 0xA, &chartToConvert.lanes[x].notes[counter].noteLength, sizeof(UINT16));
                    memcpy((BYTE*)lpAddress + offset + noteOffset + 0xC, &chartToConvert.lanes[x].notes[counter].unknown2, sizeof(UINT8));
                }



                counter++;
                y = y + 0xD;
            };

            offset = offset + 0x4E + newSizeNoteData;
        }
    }

    saveMemoryRangeToFile(L"C:\\Users\\randa\\source\\repos\\ez2songdbeditor\\x64\\Debug\\test-yes-yes.ez", lpAddress, chartToConvert.NewSize);

}


void OpenOld(LPWSTR ezFileName) {

    if (endsWith(ezFileName, L".ez")) {
        //wstring fileName = ExePath().append(L"\\");
        //fileName.append(ezFileName);
        HANDLE hFile = CreateFileW(ezFileName, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hFile == INVALID_HANDLE_VALUE) {
            DWORD dwLastError = GetLastError();
            return;
        }

        DWORD fileSize = GetFileSize(hFile, NULL);
        LPVOID lpAddress = VirtualAlloc(NULL, fileSize, MEM_COMMIT, PAGE_READWRITE);
        if (!ReadFile(hFile, lpAddress, fileSize, &fileSize, NULL)) {
            return;
        }

        CloseHandle(hFile);


        int newFileSize = 0;


        memcpy(&chartToConvert.ticksPerMeasure, (BYTE*)lpAddress + 0x86, sizeof(UINT16));
        memcpy(&chartToConvert.BPM, (BYTE*)lpAddress + 0x88, sizeof(float));
        memcpy(&chartToConvert.numLanes, (BYTE*)lpAddress + 0x8C, sizeof(UINT16));
        memcpy(&chartToConvert.totalTicks, (BYTE*)lpAddress + 0x8E, sizeof(UINT32));
        memcpy(&chartToConvert.OtherBPM, (BYTE*)lpAddress + 0x92, sizeof(float));
        
        //start Lanes
        int offset = 0x96;
        newFileSize += 0x96;
        for (UINT16 x = 0; x < chartToConvert.numLanes; x++) {
            memcpy(&chartToConvert.lanes[x].laneName, (BYTE*)lpAddress + offset + 6, sizeof(char[0x40]));
            memcpy(&chartToConvert.lanes[x].laneTicks, (BYTE*)lpAddress + offset + 0x46, sizeof(UINT32));
            memcpy(&chartToConvert.lanes[x].sizeOfNoteData, (BYTE*)lpAddress + offset + 0x4A, sizeof(UINT32));
            
            //we're converting from OLD remember thissss!!! 
            UINT32 y = 0;
            int counter = 0;
            newFileSize += 0x4E;
            while(y < chartToConvert.lanes[x].sizeOfNoteData){
                UINT32 noteOffset = y + 0x4E;
                memcpy(&chartToConvert.lanes[x].notes[counter].position, (BYTE*)lpAddress + offset + noteOffset, sizeof(UINT32));
                memcpy(&chartToConvert.lanes[x].notes[counter].NoteType, (BYTE*)lpAddress + offset + noteOffset + 0x4, sizeof(UINT8));
                memcpy(&chartToConvert.lanes[x].notes[counter].noteValue, (BYTE*)lpAddress + offset + noteOffset + 0x5, sizeof(UINT8));
                memcpy(&chartToConvert.lanes[x].notes[counter].velocity, (BYTE*)lpAddress + offset + noteOffset + 0x6, sizeof(UINT8));
                memcpy(&chartToConvert.lanes[x].notes[counter].panning, (BYTE*)lpAddress + offset + noteOffset + 0x7, sizeof(UINT8));
                memcpy(&chartToConvert.lanes[x].notes[counter].unknown, (BYTE*)lpAddress + offset + noteOffset + 0x8, sizeof(UINT8));
                memcpy(&chartToConvert.lanes[x].notes[counter].noteLength, (BYTE*)lpAddress + offset + noteOffset + 0x9, sizeof(UINT16));


                counter++;
                y = y + 0xB;

                newFileSize += 0xD;
            };

            chartToConvert.lanes[x].numNotes = counter;

            offset = offset + 0x4E + chartToConvert.lanes[x].sizeOfNoteData;
        }

        chartToConvert.NewSize = newFileSize;

        ConvertToNew();
    }

}




static void iterate_dir(std::wstring dir) {
    WIN32_FIND_DATA  fd;
    HANDLE hFind;
    std::wstring fn_ws;
    std::wstring fn;
    int pos = 0;
    int count_bg = 0;
    int count_fg = 0;
    //std::string dir_bkp = dir;
    std::wstring dir_sub;

    std::wstring str_wide_char_for_any = L"*.*";
    std::wstring str_folder_node = L"..";

    if (dir.length() - dir.find_last_of(L"\\") > 1) //dir ends without "\\"
        dir += L"\\";

    dir += str_wide_char_for_any;
    std::wstring dir_wstr = std::wstring(dir.begin(), dir.end());
    LPCWSTR dir_wc = dir_wstr.c_str();

    hFind = FindFirstFile(dir_wc, &fd);
    if (hFind == INVALID_HANDLE_VALUE) {
        FindClose(hFind);
        return;
    }
    while (true) {
        if (!FindNextFile(hFind, &fd)) {
            break;
        }
        if (fd.dwFileAttributes & FILE_ATTRIBUTE_ARCHIVE) {
            fn_ws = std::wstring(fd.cFileName);
            fn = std::wstring(fn_ws.begin(), fn_ws.end());
            if (endsWith(fn, L".ez")){
                printf("Converting: %ls\n", fn.c_str());
                addAntiTamperByteAndSave((LPWSTR)fn.c_str());
            }
        }
    }
    FindClose(hFind);
    return;
}

int main(int argc, char** argv)
{
    bool addTamper = false;
    getchar();

    if (argc != 2) {
        iterate_dir(ExePath());

        cout << "Press enter to close..";
        getchar();
        return -1;
    }

    wchar_t wtext[MAX_PATH];
    mbstowcs(wtext, argv[1], strlen(argv[1]) + 1);//Plus null
    LPWSTR ezFileName = wtext;
    OpenOld(ezFileName);


    //addAntiTamperByteAndSave(ezFileName);

}
