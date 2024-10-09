#include "FlightSimulator.h"
#include <map>
#include <string>
#include <math.h>
#include "helperfunctions.h"

using namespace std;

void CFlightSimulator::Update()
{
    uint64_t elapsedTicks = GetTickCountMs() - mTicksOnLastUpdate;
    mTicksOnLastUpdate = GetTickCountMs();
    static float period = 5000; //5000ms
    // float a = ((mTicksOnLastUpdate % (uint64_t)period) / period) * 6.28;
    float a = ((mTicksOnLastUpdate) / period) * 6.28;
    float s = sinf(a); // varies between -1 and +1
    float p = sinf(a/4.0); // varies between -1 and +1

    mAltitude = 100 + 100 * s;
    // mFlapPosition = 0.5 + (p / 2.f);
    mFlapPosition = 200 + 100 * p ;
    mEngineRPM[0] = 2000 + 500 * p;
    mEngineRPM[1] = 2000 + 500 * p;

    UpdateNamedVariableFloats();
}

NamedVariableQuery CFlightSimulator::ParseNamedVariable(std::string identifier)
{
    NamedVariableQuery query;

    static map<string, ENamedVariable> mapNameToKey = {
        {"EngineRPM", ENamedVariable::EngineRPM },
        {"Altitude", ENamedVariable::Altitude },
        {"FlapPosition", ENamedVariable::FlapPosition },
    };

    // if it's an internal variable it should be in the map
    map<string, ENamedVariable>::iterator it = mapNameToKey.find(identifier);
    if (it != mapNameToKey.end())
    {
        query.Variable = (uint32_t)(*it).second;
        return query;
    }

    return NamedVariableQuery(ENamedVariable::NoNamedVariable);
}

float CFlightSimulator::GetNamedVariableValue(NamedVariableQuery variable)
{
    switch ((ENamedVariable)variable.Variable)
    {
        case ENamedVariable::Altitude:
            return mAltitude;
        break;
        case ENamedVariable::EngineRPM:
            if (variable.Index > 0 && variable.Index < 4) return mEngineRPM[variable.Index];
            else return mEngineRPM[0];
        break;
        case ENamedVariable::FlapPosition:
            return mFlapPosition;
        break;
        default:
            break;
    }
    return 0.0f;
}

float* CFlightSimulator::GetAddressOfNamedVariableFloat(std::string identifier)
{
    NamedVariableQuery query = ParseNamedVariable(identifier);
    // if not existing: create <query, float> pair. so te actual float instance exists; and return a const pointer to the float
    float& floatref = mNamedVariableSources[query];
    return &floatref;
}

void CFlightSimulator::UpdateNamedVariableFloats()
{
    for (auto& [query, f] : mNamedVariableSources)
        f = GetNamedVariableValue(query);
}
