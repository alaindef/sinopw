// notes
//  ${workspaceFolder} the path of the workspace folder that contains the tasks.json file

#ifdef __APPLE__
#else
#include <windows.h>
#include <GL/gl.h>
#include <GL/freeglut.h>
#endif

#include <iostream>
#include <thread>
#include <filesystem>
#include <png.h>
#include "Renderer.h"
#include "Texture.h"
#include "FlightSimulator.h"
#include "FSData.h"

#include "utils.h"
#include "vartable.h"
#include "tokengenerator.h"
#include "rpngenerator.h"
#include "calculator.h"
#include "runprogram.h"

using namespace std;
CFlightSimulator gFlightSimulator;

void InitFunction(std::vector<std::string>);
void createProgram();
void Renderfunction();
void dialog();

static int reportlevel = 0; // >0 means that tokengen and RPNgen output will be printed
// provisional globals
// string scriptDir = "scripts/";
// string pingDir   = "pings/";
string scriptDir = "./scripts/";//"/home/alaindef/projects/vscode/sinop/scripts/";
string pingDir = "./pings/";//"/home/alaindef/projects/vscode/sinop/pings/";
unsigned printResult = 0;
unsigned printVarTable = 0;
VarTable VARIABLES;
vector<Token> tokenList;
vector<RPNToken> tokensRPN;
vector<vector<RPNToken> > RPNList;

CFSData gFSData;

int main(int argc, char *argv[]) {
    cout << "--------------------------------------- teut --------------------------------" << endl;
    // read and print the source program
    string name;
    name = scriptDir + "source240702.sin";
    // name = scriptDir + "source240623.sin";
    // name = scriptDir + "sourceTest.sin";
    // name = scriptDir + "source3.sin";
    // name = scriptDir + "source-meters-only.sin";
    // name = scriptDir + "sourceifthen.sin";
    // name = scriptDir + "sourcewhile.sin";
    cout << "name is " << name << endl;
    if (filesystem::exists(filesystem::path(name))) cout << "script file good" << endl;
    else {
        cout << "script file bad" << endl;
        return 0;
    }

    auto src = readProgram(name);

    thread dialogThread(dialog);

    gFSData.InitData();
    gFSData.CreateSocket();

    // generate the program as a vector of RPN tokenlists
    // VARIABLES.printVarTable();
    if (printResult + printVarTable == 0)
        CRenderer::InitSetStart(argc, argv,
            InitFunction,
            src,
            Renderfunction); // no rendering when testing

    dialogThread.join();

    return EXIT_SUCCESS;
}


void InitFunction(vector<string> script) {
    // this has to occur after init of openGL, so cannot be in vartable.h
    for (int i = 0; i < script.size(); i++) {
        if (script[i][0] == '#')
            continue;
        tokenList = makeTokenList(script[i], keywords, VARIABLES, reportlevel);
// skip USE statement. no RPN needed. makeTokenList handles png to texture
        if (tokenList[1].opcode == OC::USE) continue;
        tokensRPN = makeRPN(tokenList, reportlevel);
        // VARIABLES.printVarTable();
        RPNList.push_back(makeRPN(tokenList, reportlevel)); // comment out if testing only tokens
    }

    CTexture texi = {};
}

void Renderfunction() {
    // cout << "alti = " << VARIABLES.getValue(0) << endl;
    exec(RPNList, VARIABLES);
}

void dialog() {
    string ch = "";
    long unsigned int choice = 0;
    while (true) {
        cout << "\nchoice (1 to switch data source, 0 to exit) ==> ";
        cin >> ch;
        cin.ignore();
        if (isNumeric(ch))
        // if (ch != "")
        {
            choice = stoi(ch);
            cout << "choice = " << choice << endl;
            switch (choice) {
                case 0:
                    gFSData.CloseSocket();
                    glutLeaveMainLoop();
                    return;
                case 1:
                    VARIABLES.gUseXPData = 1 - (VARIABLES.gUseXPData > 0);
                    cout << "we will switch to the other data source " << VARIABLES.gUseXPData << endl;

                    break;
                default:
                    cout << "you typed another interesting number. go on!" << endl;
                    break;
            }
        } else if (ch == "r") // not used. reportLevel not propagated or so
        {
            reportlevel = 1 - (reportlevel > 0);
            cout << "reportlevel now is " << reportlevel << endl;
        } else
            cout << "no  number. retry" << endl;
    }
};
