#include <iostream>
#include <memory>
#include "calculator.h"
#include "FlightSimulator.h"
#include <windows.h>
#include <GL/gl.h>
#include <GL/freeglut.h>

#include "helperfunctions.h"
#include "Renderer.h"
#include <png.h>
#include "Texture.h"
#include "RenderMath.h"
#include <cmath>

extern CFlightSimulator gFlightSimulator;

float calc(std::vector<RPNToken> &tokenlist, VarTable &vartabel, int &cursor)
{
    if (tokenlist.size() == 0)
        return 0.f;

    vector<float> stack;

    float res = 0;
    int resin = 0;
    float p[10]; // parameters of an operator

    cursor = 0;
    int pnr; // param number
    int arit;
    int lopctr = 0;

    Rect recta = {0, 0, 256, 196};

    RPNToken last;

    while (cursor < tokenlist.size())
    {
        last = tokenlist[cursor];

        arit = last.arity; // use arity to fill up params! or p[]
        for (pnr = arit; pnr > 0; pnr--)
        {
            p[pnr] = stack.back();
            stack.pop_back();
        }

        switch (last.opcode)
        {
        case OC::NUM:
            res = last.value;
            stack.push_back(last.value);
            break;
        case OC::VAR:
            res = vartabel.getValue(last.value);
            stack.push_back(vartabel.getValue((int)last.value));
            break;
        case OC::MUL:
            res = p[1] * p[2];
            stack.push_back(res);
            break;
        case OC::DIV:
            res = p[1] / p[2];
            stack.push_back(res);
            break;
        case OC::MOD:
            resin = (int)p[1] % (int)p[2];
            res = (float)resin;
            stack.push_back(res);
            break;
        case OC::REM:
            res = p[1] - std::floor(p[1] / p[2]) * p[2];
            stack.push_back(res);
            break;
        case OC::ADD:
            res = p[1] + p[2];
            stack.push_back(res);
            break;
        case OC::SUB:
            res = p[1] - p[2];
            stack.push_back(res);
            break;
        case OC::PAS:
            res = p[1];
            stack.push_back(res);
            break;
        case OC::CHS:
            res = -p[1];
            stack.push_back(res);
            break;
        case OC::LT:
            res = p[1] < p[2];
            stack.push_back(res);
            break;
        case OC::LE:
            res = p[1] <= p[2];
            stack.push_back(res);
            break;
        case OC::GT:
            res = p[1] > p[2];
            stack.push_back(res);
            break;
        case OC::GE:
            res = p[1] >= p[2];
            stack.push_back(res);
            break;
        case OC::EQ:
            res = p[1] == p[2];
            stack.push_back(res);
            break;
        case OC::NE:
            res = p[1] != p[2];
            stack.push_back(res);
            break;
        case OC::ASS:
            vartabel.setVar(p[1], p[2]);
            break;
        case OC::COL:
            res = p[1] ? p[2] : p[3];
            stack.push_back(res);
            break; 
        case OC::getms:
            res = GetTickCountMs();
            stack.push_back(res);
            break;
        case OC::pushm:
            glPushMatrix();
            break;
        case OC::popm:
            glPopMatrix();
            break;
        case OC::trnsm:
            glTranslatef(p[1], p[2], 0);
            break;
        case OC::rotm:
            glRotatef(p[1], p[2], p[3], p[4]);
            break;
        case OC::drawm:
            recta = {p[2], p[3], p[4], p[5]};
            CRenderer::DrawImage(&vartabel.tex[(int)p[1]], &recta, 0xFFFFFFFF);
            break;

        default:
            break;
        }
        cursor++;
    }
    return res;
}

float calc(std::vector<RPNToken> &tokenlist, VarTable &vartabel)
{
    int cursor = tokenlist.size();
    return calc(tokenlist, vartabel, cursor);
}

float calcandprint(std::vector<RPNToken> &tokenlist, VarTable &vartabel)
{
    int cursor = tokenlist.size();
    float result = calc(tokenlist, vartabel, cursor);
    return result;
}
