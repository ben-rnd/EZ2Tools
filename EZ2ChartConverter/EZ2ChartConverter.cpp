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

    if (argc != 2) {
        iterate_dir(ExePath());

        cout << "Press enter to close..";
        getchar();
        return -1;
    }

    wchar_t wtext[MAX_PATH];
    mbstowcs(wtext, argv[1], strlen(argv[1]) + 1);//Plus null
    LPWSTR ezFileName = wtext;

    addAntiTamperByteAndSave(ezFileName);

}
