#pragma once
#ifndef CALCULATOR_H
#define CALCULATOR_H
#include <functional>
#include "rpngenerator.h"


    float calc(std::vector<RPNToken> &tokenlist, VarTable &vartabel);
    float calcandprint(std::vector<RPNToken> &tokenlist, VarTable& vartabel);

#endif // CALCULATOR_H
