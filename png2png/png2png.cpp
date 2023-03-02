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

static int pngKey[128] = { 6, 0x3d, 0x7d, 0x23,  0x10, 0xfe,  0xd,  0xec,  0xe8,  0x10,  0xd4,  0x4c,  0xb8,  0xc5,  0x7e,  0x2e,  0xf,  1,  2,  0x21,  0x24,  4,  0x20,  0x12,  0x17,  0x1a,  0xe4,  0x1a,  0x7c,  0x57,  0xde,  0xe,  5,  0xb,  2,  3,  0x3d,  0x29,  0x23,  0x13,  0xd5,  6,  0x1c,  0x56,  0x18,  0x57,  2,  0x54,  0x41,  1,  2,  0x1f,  0x56,  4,  3,  0xd8,  0x27,  6,  0x3a,  6,  4,  7,  2,  4,  5,  0x3d,  0x19,  0x23,  6,  0x36,  0x1f,  0x24,  0xd3,  6,  0x52,  6,  0x54,  0x61,  0x1a,  0x2e,  0xf,  1,  2,  0x21,  0x24,  0x36,  0x1f,  0x11,  0x1f,  0x7e,  0x1c,  6,  0x18,  0x57,  0x16,  4,  5,  0xb,  2,  3,  0xa1,  0x19b,  0x87,  0x74,  0xd,  0x3d,  0x1c,  0x56,  0x18,  0x57,  2,  0x54,  0x41,  1,  2,  0x1f,  0x56,  0xe,  3,  0xa6,  0x23,  0x10,  0x3a,  6,  4,  7,  2,  4 };


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

void convertToPng(LPWSTR ezFileName) {
    if (endsWith(ezFileName, L".png")) {

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


        int counter = 0;
        for (int i = 0; i < 0x100; i++ ) {
            char* dataAtOffset = (char*)((BYTE*)lpAddress + i);
            *dataAtOffset = *dataAtOffset - pngKey[counter];
            counter = counter + 1;
            if (counter >= sizeof(pngKey)/sizeof(pngKey)[0]) {
                counter = 0;
            }
        }

        replace_file_extension(ezFileName, L".bng");
        saveMemoryRangeToFile(ezFileName, lpAddress, fileSize);

        VirtualFree(lpAddress, fileSize, MEM_RELEASE);

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
            if(!endsWith(fd.cFileName, L"png2png.exe")){
                fn = std::wstring(fn_ws.begin(), fn_ws.end());
                if (endsWith(fn, L".png")) {
                    printf("Converting: %ls\n", fn.c_str());
                    convertToPng((LPWSTR)fn.c_str());
                }
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
    printf("Converting: %ls\n", ezFileName);
    convertToPng(ezFileName);


    //addAntiTamperByteAndSave(ezFileName);

}


