#pragma once
#ifndef RUNPROGRAM_H
#define RUNPROGRAM_H
#include <functional>
#include "rpngenerator.h"

// we put the input token list in a struct, with access operators
struct Lines
{
    vector<vector<RPNToken>> RPNList;
    int cursor = 0;
    vector<RPNToken> pop()
    {
        if (cursor >= RPNList.size())
            throw invalid_argument("symbols missing in struct Lines!");
        return RPNList[cursor++];
    };
    vector<RPNToken> get(int shift)
    {
        int newCursor = cursor + shift;
        if ((newCursor < 0) || (newCursor > RPNList.size()))
            // throw invalid_argument("out of reach");
            std::cout << "trying to get item outside the list" << endl;
        return RPNList[newCursor];
    }
    bool done()
    {
        return (cursor >= RPNList.size());
    }
};


vector<string> readProgram(std::string filename);
void exec(vector<vector<RPNToken> > &RPNProgram, VarTable &vartabel); 

#endif // RUNPROGRAM_H
