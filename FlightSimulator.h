#pragma once
#include <cstdint>
#include <string>
#include <map>

enum class ENamedVariable : int
{
    FlapPosition,
    Altitude,
    EngineRPM,
    NoNamedVariable
};

struct NamedVariableQuery
{
    NamedVariableQuery(uint32_t var) : Variable(var) {}
    NamedVariableQuery(ENamedVariable var) : Variable((uint32_t)var) {}
    NamedVariableQuery(uint32_t var, int index) : Variable(var), Index(index) {}
    NamedVariableQuery(ENamedVariable var, int index) : Variable((uint32_t)var), Index(index) {}
    NamedVariableQuery() : Variable((uint32_t)ENamedVariable::NoNamedVariable) {}

    // the INTEGER representation of the ENamedVariable enumeration
    uint32_t Variable;
    // the index in case the variable represents an array (-1 means no array)
    int Index = -1;    

    bool operator<(const NamedVariableQuery& r) const {
        if (Variable < r.Variable) return true;
        if (Variable > r.Variable) return false;
        if (Index < r.Index) return true;
        if (Index > r.Index) return false;
        return false;
    }

    bool operator==(const NamedVariableQuery& r) const {
        if (Variable != r.Variable) return false;
        if (Index != r.Index) return false;
        return true;
    }
};

class CFlightSimulator
{
    public:
        void Update();
        float GetFlapPosition();
        float GetAltitude();
        float GetEngineRPM(int enginenr);
    
    NamedVariableQuery ParseNamedVariable(std::string identifier);
    float GetNamedVariableValue(NamedVariableQuery variable);
    float* GetAddressOfNamedVariableFloat(std::string identifier);

    private:
    uint64_t mTicksOnLastUpdate = 0;
    float mAltitude = 0;
    float mFlapPosition = 0;
    float mEngineRPM[4] = {0,0,0,0};    

    void UpdateNamedVariableFloats();
    std::map<NamedVariableQuery, float> mNamedVariableSources;
};