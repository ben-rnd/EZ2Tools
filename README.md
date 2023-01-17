# EZ2 SongDB Editor
A tool for editing the EZ2AC song.bin files.



Documentation:


# NT:
INI Decrypt: 00412e02
EZI: 00410837
EZ:00410607

# FNEX:
00410BB4 - EZ
00410E04 - EZI
004134E0,0041374C - INI


Songs whill not appear unless Stage.ini is decrypted and ALLSONGS var is edited, just like the old days. This is usefull if you want to add songs to only one game mode. making the following edits will break any gamemode song.bin with an unmodified song.bin. You can pad the bin file with blank songs to fix this.
A lot of the offsets in the exe for parsing the song details are hardcoded, the following offsets need to be adjusted when adding new songs: 

1) Game crashes while parsing song.bin unless:
00469deb: -> CMP EDX,dword ptr [ECX + 0x<9278 + 0x56 * numAddtionalSongs>] (0x56 * numSongs)
00469dc2: -> to MOV dword ptr [EDI + 0x<9278 + 0x56 * numAddtionalSongs>] (0x56 * numSongs)
00469df8: -> to LEA EAX,[EDI + 0x<927C + 0x56 * numAddtionalSongs>] (0x4 + 0x56 * numSongs) 
00469dfe: -> to LEA ECX,[EDI + 0x<9338 + 0x56 * numAddtionalSongs>] (0xC0 + 0x56 * numSongs)

The following may restrict how many songs we can add. These are hardcoded chunks of storage for the song list. unsure how much extra  space we have, reducing size of the "ALL" category may buy some space.
2) Game crashes upon loading stage (and scanning catergories) unless: 
00435090: -> 0x3789BD4 + 0x56 * numAddtionalSongs 
00435110: -> 0x3789c90 + 0x56 * numAddtionalSongs 
Fortunately the devs werent complete demons and the code references the .bin file for grabbing the no. of songs in a category.

3) Game crashes on exit unless:
00469f98: -> CMP ESI,dword ptr [ECX + 0x<9278 +  0x56 * numAddtionalSongs>] (0x56 * numSongs)
00469f45: -> CMP dword ptr [ECX + 0x<9278 +  0x56 * numAddtionalSongs>],EDX (0x56 * numSongs)

CV2 Mode completely breaks, still WIP to work out how the above offsets are impacting the game.

# File format info

.ezi = track file that just contains id, tone(?) and name of wav file. Has been encrypted since 7th with its own rainbow table. 

.ez = song pattern file, 2 versions, v06 and v08. v06 used up until AEIC, then EC (or EV?) changed to v08, 08 type seems to have changed over time and can support .ezi files. Has been encrypted since 7th with its own rainbow table.

.ini = bunch of data relevant to whatever folder its in, every .ini is encrypted the same. Has been encrypted since 2nd with its own rainbow table.

.ssf/.ezw = same file type. just a wav with header data stripped/changed. 
.amb = there is a old and new type of amb, seems they changed the way some data was stored. just a BMP with some header data changed/stripped.

the above files are easily decrypted using the "Ez2Decrypt" tool. Later versions require the tool to be modfied to work correctly, but easy enough to do if you know what to look for ;)

.bin = new file type since EV. Contains song data such as file name, label, bpm, and levels. has 3 revisions. 00 for EV, 01 for NT and TT, 03 for FN and FNEX. 01 introduced BPM data. 03 no longer includes label data and introduced the "categories" feature. encrypted using XOR subsitution cipher with 2 keys. Easily decrypted with this tool ;)






