#pragma once
#include <stdint.h>

struct Rect
{
    float left;
    float top;
    float right;
    float bottom;
};
inline float RectWidth(const Rect& r){
    return r.right - r.left;
}
inline float RectHeight(const Rect& r){
    return r.bottom - r.top;
}
struct Point
{
    float x;
    float y;
};

typedef uint32_t Color;