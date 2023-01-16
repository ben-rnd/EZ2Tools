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
#define gameVerOffset 0x40
#define nmOffset 0x42
#define nmMinBpm 0x43
#define nmMaxBpm 0x47


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

    //number of playlists is hardcoded in exe so only expanding the number of songs in a playlist is possible.
    //Playlist Bytes:
    //BB 00 31 31 73 74 61 72 67 61 7A 65 72 00 00 00 00 00
    //BB 00 = number of songs in playlist, 151,
    //31 31 73 74 61 72 67 61 7A 65 72 00 00 00 00 00 = name of song, 11stargazer
    //repeat 150 more times for rest of playlist
    //number of playlist seems to be hardcoded?
    struct Category {
        uint16_t numSongs = 0;
        char *songNames[MAX_SONGS];
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

    bool openFile(LPCTSTR inputFile, SongList* songList, bool decrypt = true);
    bool SaveFile(LPCTSTR savePath, SongList *songList, bool encrypt = true);
    bool shiftSongDown(SongList* songList, int category, int songIndex);
    bool shiftSongUp(SongList* songList, int category, int songIndex);



    //-------------
    //TEXT Data
    //-------------
    static char* GameMode_names[] =
    {   "5K ONLY",
        "5K RUBY",
        "5K STANDARD",
        "7K STANDARD",
        "10K MANIAC",
        "14K MANIAC",
        "EZ2CATCH",
        "TURNTABLE"
    };

    static char* GameVersion_names[] =
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

    static char* Catergory_Names[NUM_CATEGORIES] =
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


