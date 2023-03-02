// bmp2abm-deluxe.cpp : This file contains the 'main' function. Program execution begins and ends there.
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

void replace_file_extension(LPWSTR filename, LPCWSTR new_extension) {
    LPWSTR extension_pos = wcsrchr(filename, L'.');
    if (extension_pos != NULL) {
        // Found the file extension, replace it with the new one
        wcscpy(extension_pos, new_extension);
    }
    // Else: No file extension found, do nothing
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

void convertToAbm(LPWSTR ezFileName) {
    if (endsWith(ezFileName, L".bmp")) {
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

        UINT8 emptyBit = 0x0;
        UINT16 AW = 0x5741;
        memcpy((BYTE*)lpAddress, &AW, sizeof(UINT16));
        memcpy((BYTE*)lpAddress + 0x2, (BYTE*)lpAddress + 0x1c, sizeof(UINT16));
        memcpy((BYTE*)lpAddress + 0x3, &emptyBit, sizeof(UINT8));
        memcpy((BYTE*)lpAddress + 0x4, &emptyBit, sizeof(UINT8));
        memcpy((BYTE*)lpAddress + 0x5, &emptyBit, sizeof(UINT8));
        memcpy((BYTE*)lpAddress + 0x6, &emptyBit, sizeof(UINT8));
        memcpy((BYTE*)lpAddress + 0x7, &emptyBit, sizeof(UINT8));
        memcpy((BYTE*)lpAddress + 0x9, &emptyBit, sizeof(UINT8));


        memcpy((BYTE*)lpAddress + 0x6, (BYTE*)lpAddress + 0x12, sizeof(UINT16));
        memcpy((BYTE*)lpAddress + 0x8, (BYTE*)lpAddress + 0x16, sizeof(UINT16));

        DWORD* dataAtOffset = (DWORD*)((BYTE*)lpAddress + 0xA); 
        DWORD originalValue = *dataAtOffset;  
        *dataAtOffset = *dataAtOffset ^ 0x109a; 

        dataAtOffset = (DWORD*)((BYTE*)lpAddress + 0x12);
        originalValue = *dataAtOffset;
        *dataAtOffset = *dataAtOffset ^ 0xcfa1;

        dataAtOffset = (DWORD*)((BYTE*)lpAddress + 0x16);
        *dataAtOffset = *dataAtOffset ^ 0x51ae;

        dataAtOffset = (DWORD*)((BYTE*)lpAddress + 0x1c);
        *dataAtOffset = *dataAtOffset ^ 0xb18f;

        replace_file_extension(ezFileName, L".abm");
        saveMemoryRangeToFile(ezFileName, lpAddress, fileSize);
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
            if (endsWith(fn, L".bmp")) {
                printf("Converting: %ls\n", fn.c_str());
                convertToAbm((LPWSTR)fn.c_str());
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
    printf("Converting: %ls\n", ezFileName);
    convertToAbm(ezFileName);


    //addAntiTamperByteAndSave(ezFileName);

}
