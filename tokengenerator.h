#pragma once
#ifndef tokengenerator_H
#define tokengenerator_H

#include "vartable.h"
#include <string>
#include <vector>
#include <map>

enum class OC {
    NUM,
    VAR,
    STR,
    MUL,
    DIV,
    MOD,
    REM,
    ADD,
    SUB,
    PAS,
    CHS,
    LT,
    LE,

    GT,
    GE,
    EQ,
    NE,
    ASS,
    QU,
    COL,
    PAR_L,
    PAR_R,
    COMMA,
    USE,

    FIRSTFUN,
    getms,
    pushm,
    popm,
    trnsm,
    rotm,
    drawm,
    alti,
    qIf,
    qElse,
    fIq,
    qWhile,
    elihWq,
    endp,
    NIL,
};

// pretty print opcodes
const std::string ppOC[43] = {
    "NUM  ", "VAR  ", "STR  ", "MUL  ", "DIV  ", "MOD  ", "REM ", "ADD ", "SUB  ", "PAS ",
    "CHS  ", "LT   ", "LE   ", "GT   ", "GE   ", "EQ   ", "NE  ", "ASS  ","QU   ", "COL  ",
    "PAR_L", "PAR_R", "COMMA", "USE  ", "FIRST", "getms", "pushm","popm ","trnsm", "rotm ",
    "drawm", "alti ", "IF   ", "ELSE ", "FI   ", "WHILE", "elihWq", "Finito", "NIL  "
};

struct Token {
    OC opcode;
    int arity;
    int precedence;
    float value;
    string description;
};

const map<std::string, Token> keywords = {
    // symbol {opcode, arity, precedence, value}
    {"NUM", {OC::NUM, 0, 0, 0}},
    {"VAR", {OC::VAR, 0, 0, 0}},
    {"STR", {OC::STR, 0, 0, 0}},
    {"*", {OC::MUL, 2, 3, 0}},
    {"%", {OC::MOD, 2, 3, 0}},
    {"/", {OC::DIV, 2, 3, 0}},
    {"REM", {OC::REM, 2, 3, 0}},
    {"+", {OC::ADD, 2, 4, 0}},
    {"-", {OC::SUB, 2, 4, 0}},
    {"<", {OC::LT, 2, 6, 0}},
    {"<=", {OC::LE, 2, 6, 0}},
    {">", {OC::GT, 2, 6, 0}},
    {">=", {OC::GE, 2, 6, 0}},

    {"==", {OC::EQ, 2, 7, 0}},
    {"!=", {OC::NE, 2, 7, 0}},
    {"?", {OC::QU, -1, 13, 0}},
    {":", {OC::COL, 3, 13, 0}},
    {"=", {OC::ASS, 2, 14, 0}},
    {"(", {OC::PAR_L, -1, -1, 0}},
    {")", {OC::PAR_R, -1, -1, 0}},
    {",", {OC::COMMA, -1, -1, 0}},
    {"USE", {OC::USE, 2, -1, 0}},
    {"FIRSTFUN", {OC::FIRSTFUN, 0, 0, 0}},
    {"getms", {OC::getms, 0, 0, 0}},
    {"pushm", {OC::pushm, 0, 0, 0}},
    {"popm", {OC::popm, 0, 0, 0}},
    {"trnsm", {OC::trnsm, 0, 0, 0}},
    {"rotm", {OC::rotm, 0, 0, 0}},
    {"drawm", {OC::drawm, 0, 0, 0}},
    {"alti", {OC::alti, 0, 0, 15}},
    {"if", {OC::qIf, 0, 0, 15}},
    {"else", {OC::qElse, 0, 0, 15}},
    {"fi", {OC::fIq, 0, 0, 15}},
    {"while", {OC::qWhile, 0, 0, 15}},
    {"elihw", {OC::elihWq, 0, 0, 15}},
    {"endp", {OC::endp, 0, 0, 15}},
    {"NIL", {OC::NIL, -1, 0, 0}}
};

string pp(Token &token, map<std::string, Token> &keys);

vector<Token> makeTokenList(string textIn,
                            const map<std::string, Token> &keywords,
                            VarTable &vartabel, int report);

#endif // tokengenerator_H
