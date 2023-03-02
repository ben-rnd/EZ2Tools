
//
#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>
#include <regex>
#include <windows.h>

using namespace std;

// Convert note and octave to integer note number
int getNoteNumber(const std::string& note, int octave) {
    static const std::unordered_map<std::string, int> noteMap = {
        {"C", 0}, {"C#", 1}, {"D", 2}, {"D#", 3}, {"E", 4}, {"F", 5},
        {"F#", 6}, {"G", 7}, {"G#", 8}, {"A", 9}, {"A#", 10}, {"B", 11}
    };
    int noteNum = noteMap.at(note);
    return octave * 12 + noteNum;
}

// Convert integer note number to note and octave string
std::string getNoteName(int noteNumber) {
    static const std::vector<std::string> noteNames = {
        "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"
    };
    int octave = noteNumber / 12;
    int note = noteNumber % 12;
    return noteNames[note] + std::to_string(octave);
}

int main(int argc, char** argv) {

    wchar_t wtext[MAX_PATH];
    mbstowcs(wtext, argv[1], strlen(argv[1]) + 1);//Plus null
    LPWSTR eziFileName = wtext;

    // Open input file
    ifstream inputFile(eziFileName);
    if (!inputFile.is_open()) {
        cerr << "Failed to open input file." << endl;
        return 1;
    }

    // Open temporary output file
    ofstream tempFile("temp.ezi");
    if (!tempFile.is_open()) {
        cerr << "Failed to open temporary output file." << endl;
        return 1;
    }

    std::string line;
    int lineNumber = 1;

    std::regex note_regex("([A-G]#?)([0-9]+)");

    while (std::getline(inputFile, line)) {
        std::istringstream iss(line);
        std::string note, velocity, fileName;
        iss >> note >> velocity >> fileName;

        smatch match;

        if (regex_search(line, match, note_regex)) {
            string note = match.str(1);
            int octave = stoi(match.str(2));

            int noteNumber = getNoteNumber(note, octave);
            tempFile << noteNumber << " " << velocity << " " << fileName << std::endl;
            lineNumber++;
        }

    }

    inputFile.close();
    tempFile.close();

    // Replace input file with temporary file
    if (_wremove(eziFileName) != 0) {
        cerr << "Failed to delete input file." << endl;
        return 1;
    }
    if (_wrename(L"temp.ezi", eziFileName) != 0) {
        cerr << "Failed to rename temporary file." << endl;
        return 1;
    }

    return 0;
}