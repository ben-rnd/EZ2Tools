#pragma once
#include <cstdint>
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>


#define songNameSize 0x30
#define playlistSongNameSize 0x10
#define songStructSize 0x56

#define MAX_SONGS 1000
#define numCharts 4

//headerOffsets
#define gameModeOffset 0x05
#define numSongOffset 0x06
#define playlistNameSizeOffset 0x08
#define songlistSizeOffset 0x0C
#define songlistOffset 0x10

//songOffsets
#define gameVerOffset 0x30
#define nmOffset 0x32
#define nmMinBpm 0x33
#define nmMaxBpm 0x37


namespace EZ2SongDb {
    #define NUM_CATEGORIES 47

    //The beginnings of custom charts in ez2dj FNEX...
    //the following is required for the song to be fully functional: 
    //1. ENCRYPTED song data in sound folder (ezi, ez, ini) (obviously)
    //2. background data in bg folder (.scr)
    //3. 20 second song preview .ssf file under system/preview. 
    //4. disc image (amb)
    //5. small disk image (amb)
    //6. name card for song list (amb)
    // EXE needs to be modified to account for new number of songs - progress currently incoplete and stuck. Game crashes while trying to parse the playlists.
    
    //example bytes of the for a song and its charts, a song can have a max of 4 charts, normal, hard, super hard and EX
    //62 61 62 79 64 61 6E 63 65 <snip>00 00 04 00 00 00 00 00 00 DE 42 07 00 00 00 00 00 00 DE 42 00 00 00 00 00 00 00 DE 42 00 00 00 00 00 00 00 DE 42
    //62 61 62 79 64 61 6E 63 65 = song name, babydance
    //00 00 = game version, 1st trax
    //04 = chart difficulty, if 0, difficulty is disabled.
    //00 00 00 00 = minimum bpm (not used for baby dance...)
    //00 00 00 DE 42 = max BPM, 111
    struct Chart {
        uint8_t level = 0; //1-18
        float minBPM = 0; //not used if song doesnt change, Only changes whats shown under song name during song select
        float maxBPM = 0;
    };

    struct Song {
        char name[songNameSize] = "";
        uint16_t gameVersion = 0; //0-18, Only changes whats shown under song name during song select
        Chart charts[numCharts];
    };

    static const struct Song EmptySong;

    //number of playlists is hardcoded in exe so only expanding the number of songs in a playlist is possible.
    //Playlist Bytes:
    //BB 00 31 31 73 74 61 72 67 61 7A 65 72 00 00 00 00 00
    //BB 00 = number of songs in playlist, 151,
    //31 31 73 74 61 72 67 61 7A 65 72 00 00 00 00 00 = name of song, 11stargazer
    //repeat 150 more times for rest of playlist
    //number of playlist seems to be hardcoded?
    struct Category {
        uint16_t numSongs = 0;
        char songNames[MAX_SONGS][16];
    };

    //example bytes of the EZSL file (song.bin) header for streetmix
    //45 5A 53 4C 03 02 B5 01 10 00 00 00 DE 92 00 00
    //45 5A 53 4C = E  Z  S  L, EZ song List? 
    //03 = No clue
    //02 = Game Mode, 02 = street mix
    //B5 01 = 436, number songs
    //10 00 00 00 = maybe the playlist songname length?
    //DE 92 00 00 = size of the song list, 0x56 * number of songs + 0x10
    struct SongList {
        uint8_t EZSLByte = 0x03;
        uint8_t gameMode = 0; //0= 5key only, 1= ruby, 2= street, etc. I presume.
        uint16_t numSongs= 0;
        uint32_t songListOffset = 0; //unsure..
        uint32_t categoriesOffset = 0; // numSong * 0x56 + 0x10
        Song songs[MAX_SONGS];
        Category categories[NUM_CATEGORIES]; 
    };

   // static const struct SongList EmptySongList;


    bool openFile(LPCTSTR inputFile, SongList* songList, int gameVer, bool saveDecrypt, bool decrypt = true);
    bool SaveFile(LPCTSTR savePath, SongList *songList, int gameVer, bool encrypt = true);
    bool sortByBPM(SongList* songList);
    bool sortByGameVer(SongList* songList);
    bool shiftSongDown(SongList* songList, int category, int songIndex);
    bool shiftSongUp(SongList* songList, int category, int songIndex);
    bool PatchEXE(LPCTSTR exePath, SongList* songList, int currGame);
    
    //----------------
    ///Decryption Keys
    //----------------
    static enum Games {
        EV,
        NT,
        TT,
        FN,
        FNEX,
        Last = FNEX
    };

    static const char* Games_names[]{
        "EVOLVE",
        "Night Traveler",
        "Time Traveler",
        "Final",
        "Final:EX"
    };

    static const char* keys1[] =
    {
        "\x79\x81\x26\x19\x57\x85\x0A\x13\x9B\xF8\x05\x02\x24\x80\x62\x65\xF5\xA4\xDA\x92\xE4\xD9\xBB\x6E\x84\xDB\x44\xDC\xBF\x5A\xFE\x4E", //EV 
        "\x3e\x20\x82\x9b\x72\x19\xc7\x46\x26\x4b\x71\x58\xe0\x40\xe0\x71\x1a\x41\x8c\x89\x80\x49\xc7\x2e\x4a\xa3\x0e\x6b\x8d\x68\x77\x72", //NT
        "\x3e\x20\x82\x9b\x72\x19\xc7\x46\x26\x4b\x71\x58\xe0\x40\xe0\x71\x1a\x41\x8c\x89\x80\x49\xc7\x2e\x4a\xa3\x0e\x6b\x8d\x68\x77\x72", //TT
        "\xe3\xb3\x46\xf8\xff\x5c\xe3\x1b\xee\x64\xfa\x91\xb4\xf7\x44\x66\x45\x30\x30\x31\xfb\x2a\x10\xb7\x95\xc7\x10\xac\x5b\x34\x27\x9e", //FN
        "\xF3\x1A\x83\x3B\xAA\xD9\x46\x5B\x71\x3D\x16\xB9\xF2\x8D\x1F\x80\xD0\x39\x64\xAC\x30\xB6\x84\x40\xBF\xC0\x8A\x31\x12\xB1\x39\x19"  //FNEX
    };
    
    static const char* keys2[] =
    {
        "\xE1\x2D\x2B\x32\xAE\x82\x62\x82\xAA\xA8\xA2\xCA\xA1\xFC\xAE\x8E\x25\xDB\x6A\x06\x49\xA6\xC8\x59\x1C\x78\x21\xB4\xD2\x8F\x68\x29", //EV
        "\xf2\xab\x57\x29\x3b\x49\x51\x7d\x22\x05\xa9\xe0\x57\x9e\x54\x25\xb1\xd0\x9c\xe3\x1a\x21\x59\xa2\x24\x3c\xb8\x31\x0c\xfa\x0a\xeb", //NT
        "\xf2\xab\x57\x29\x3b\x49\x51\x7d\x22\x05\xa9\xe0\x57\x9e\x54\x25\xb1\xd0\x9c\xe3\x1a\x21\x59\xa2\x24\x3c\xb8\x31\x0c\xfa\x0a\xeb", //TT
        "\xca\x8a\x4a\x05\xff\xde\x41\x52\x6a\x68\x6c\xc6\x0c\x82\x1e\xe5\x9e\x33\x78\xd6\x7a\x8e\x33\x66\x39\xb8\xe5\xde\xd9\xef\xf0\x08", //FN
        "\x7D\x5D\x4F\x97\x5B\x29\xC4\x08\xB1\x77\x46\xA4\x40\x98\x8E\xE9\xCA\x43\xFE\x25\x9F\x28\x0D\x9E\x7B\x07\xDE\x84\xC0\x25\x45\x9C"  //FNEX
    };


    //-------------
    //TEXT Data
    //-------------
    static const char* GameMode_names[] =
    {   "5K ONLY (5keymix)",
        "5K RUBY (rubymix)",
        "5K STANDARD (streetmix)",
        "7K STANDARD (7streetmix)",
        "10K MANIAC (clubmix)",
        "14K MANIAC (spacemix)",
        "EZ2CATCH",
        "TURNTABLE"
    };

    static const char* FNGameVersion_names[] =
    {   "1st",
        "1st SE",
        "2nd",
        "3rd",
        "4th",
        "Plt",
        "6th",
        "7th",
        "1.5-2.0",
        "CV",
        "3S",
        "BE",
        "AEIC",
        "EC",
        "EV",
        "NT",
        "TT",
        "FN",
        "FNEX"
    };

    static const char* PreFNGameVersion_names[] =
    {   "1st",
        "1st SE",
        "2nd",
        "3rd",
        "4th",
        "Plt",
        "6th",
        "7th",
        "CV",
        "3S",
        "BE",
        "AEIC",
        "EC",
        "EV",
        "NT",
        "TT",
        "FN"
    };

    static const char* PreFnCatergory_Names[11]{
        "Option1",
        "Option2",
        "Option3",
        "Option4",
        "Option5",
        "Option6",
        "Option7",
        "Option8",
        "Option9",
        "Option10",
        "Option11",
    };



    static const char* FnCatergory_Names[NUM_CATEGORIES] =
    {   "HOT",
        "NEW",
        "ALL",
        "1st",
        "1st SE",
        "2nd",
        "3rd",
        "4th",
        "Plt",
        "6th",
        "7th",
        "1.5-2.0",
        "CV",
        "3S-BE",
        "AEIC",
        "EC",
        "EV",
        "NT",
        "TT",
        "LV1",
        "LV2",
        "LV3",
        "LV4",
        "LV5",
        "LV6",
        "LV7",
        "LV8",
        "LV9",
        "LV10",
        "LV11",
        "LV12",
        "LV13",
        "LV14",
        "LV15",
        "LV16",
        "LV17",
        "LV18",
        "ABC",
        "DEF",
        "GHI",
        "JKL",
        "MNO",
        "PQR",
        "STU",
        "VWX",
        "YZ+",
        "OTH"
    };
};


