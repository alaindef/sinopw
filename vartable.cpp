#include "vartable.h"
#include <iostream>
#include <iomanip>
#include <sstream>

#include "FlightSimulator.h"
#include "FSData.h"

extern CFlightSimulator gFlightSimulator;
extern CFSData gFSData;

using namespace std;

int VarTable::getIndex(string name)
{
    for (int i = 0; i < vartab.size(); i++)
        if (name == vartab[i].name)
            return i;
    vartab.push_back({name, 0}); // new entry in table
    return vartab.size() - 1;
}

float VarTable::getValue(int index)
{
    int fsVars = 6; // variables with VarTable index lower than this come from gFlightSimulator
    float val;
    if (gUseXPData)
        switch (index)
        {
        case 0:
            val = gFSData.GetAltitude();
            break;
        case 1:
            val = gFSData.GetBankAngle();
            break;
        case 2:
            val = gFSData.GetPitchangle();
            break;
        case 3:
            val = gFSData.GetHeading();
            break;
        case 4:
            val = gFSData.GetRollrate();
            break;
        case 5:
            val = gFSData.GetGroundspeed();
            break;
        case 6:
            val = gFSData.GetVerticalspeed();
            break;
        default:
            val = vartab[index].value;
            break;
        }
    else if (index < fsVars)
        val = *gFlightSimulator.GetAddressOfNamedVariableFloat(vartab[index].name);
    else
        val = vartab[index].value;
    return val;
}

int VarTable::setVar(string name, float value)
{
    for (int i = 0; i < vartab.size(); i++)
        if (name == vartab[i].name)
        {
            vartab[i].value = value;
            return i;
        }
    vartab.push_back({name, value}); // new entry in table
    return vartab.size() - 1;
}

int VarTable::setVar(int index, float value)
{
    vartab[index].value = value;
    return index;
}

void VarTable::printVarTable()
{
    cout << "var:   ";
    for (const auto &element : vartab)
        cout  << fixed << setprecision(2) << setw(7) << element.name.substr(0, 6);
        // cout << element.name.substr(0, 4) << "\t\t";
    cout << "\nvar:   ";
    for (const auto &element : vartab)
    {
        // printf("%.2f\t", element.value);
        cout << fixed << setprecision(2) << setw(7) <<  element.value;
    }
    cout << endl;
}
