#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <iomanip>
#include "runprogram.h"
#include "rpngenerator.h"
#include "calculator.h"

using namespace std;

vector<string> readProgram(std::string filename)
{
    // Open the file in read mode
    ifstream file(filename);

    // Check if the file opened successfully
    if (!file.is_open())
    {
        cerr << "Error opening file: " << filename << endl;
        return {""};
    }

    vector<string> lines;

    string line;
    while (getline(file, line))
    {
        if (line[0] != '#')
            lines.push_back(line);
    }

    file.close();
    return lines;
}

void prog(vector<RPNToken> &line, Lines &lineList, VarTable &vartabel, int condition);

void ifstatement(vector<RPNToken> &line, Lines &lineList, VarTable &vartabel, int condition)
{
    float res = 0.0;
    line.erase(line.begin());
    // if statement can only be "if <condition>" in 1 line !
    res = calc(line, vartabel); // calculates the value of the condition
    if (res > 0.1)
    {
        prog(line, lineList, vartabel, condition);
        if (line.front().opcode == OC::qElse)
            prog(line, lineList, vartabel, false);
    }
    else
    {
        prog(line, lineList, vartabel, false);
        if (line.front().opcode == OC::qElse)
            prog(line, lineList, vartabel, condition);
    }
    if (line.front().opcode == OC::qElse)
        prog(line, lineList, vartabel, condition);
};

void whilestatement(vector<RPNToken> &line, Lines &lineList, VarTable &vartabel, int lineNbr)
{
    float res = 0.0;
    line.erase(line.begin());
    // while statement can only be "if <condition>" in 1 line !
    res = calc(line, vartabel); // calculates the value of the condition
    if (res > 0.1)
        prog(line, lineList, vartabel, lineNbr);
};

void prog(vector<RPNToken> &line, Lines &lineList, VarTable &vartabel, int condition)
// for a "WHILE" statement condition will be used as linenumber (the start of the loop)
{
    extern unsigned printResult;
    extern unsigned printVarTable;
    float res = 0.0;
    while (true)
    {
        line = lineList.pop();
        while (line.size() == 0) line = lineList.pop();  
        int lineNbr = lineList.cursor - 1;
        OC opcode = line.front().opcode;

        string run = condition ? "    " : "SKIP"; // test only --if statement

        switch (opcode)
        {
        case OC::qIf:
            ifstatement(line, lineList, vartabel, condition);
            break;
        case OC::qElse:
            return;
        case OC::fIq:
            return;
        case OC::qWhile:
            whilestatement(line, lineList, vartabel, lineNbr);
            break;
        case OC::elihWq:
            lineList.cursor = condition + 1;
            return;
        case OC::endp:
            return;
        default:
            if (condition)
                // res = calc(line, vartabel);
                res = calcandprint(line, vartabel);
            if (printResult)
                cout << "\nat line " << lineList.cursor << " result = " << res << endl;
            if (printVarTable)
                vartabel.printVarTable();
            break;
        }
    }
    prog(line, lineList, vartabel, condition);
}

void exec(vector<vector<RPNToken> > &RPNProgram, VarTable &vartabel)
{
    Lines lineList;
    lineList.RPNList = RPNProgram;
    vector<RPNToken> line = {{}};
    prog(line, lineList, vartabel, 1);
    return;
}
