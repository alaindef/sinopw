#ifndef VARTABLE_H
#define VARTABLE_H
#include <string>
#include <vector>
#include "Renderer.h"

using namespace std;

struct VarTable
{
    struct VarValue
    {
        string name;
        float value;
    };

    vector<VarValue> vartab = {{"Altitude", 300},
                               {"BankAngle", 0},
                               {"PitchAngle", 0},
                               {"Heading", 0},
                               {"RollRate", 0},
                               {"GroundSpeed", 0},
                               {"VerticalSpeed", 0}};

    int getIndex(string name);
    // look for name in the table and return the storage index
    // if not found create a new entry and return the new index

    float getValue(int index);
    // get value at index. error if index > size of vartable
    // if index < 3 returns value from flightsimulator

    int setVar(string name, float value);
    // if name found modify the value, if not store value in new entry. returns index

    int setVar(int index, float value);
    // returns index

    void printVarTable();

    // I think png's should be 128x128 pixels
    vector<string> pings = {"adfscrambled128.png", "spriteTest.png", "nin.png", "alt_HM.png", "needle.png",
    "G1000_1_scale_rotating128.png", "carthesius128.png", "VOR_13convert.png", "VOR_11convert.png", "needlesmall.png"};
    // for the warnings on VOR pngs, see https://stackoverflow.com/questions/22745076/libpng-warning-iccp-known-incorrect-srgb-profile

    vector<CTexture> tex;

    bool gUseXPData = true;
};
 
#endif // VARTABLE_H
