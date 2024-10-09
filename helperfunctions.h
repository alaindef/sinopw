#pragma once

#include <windows.h>
#include <algorithm> 
#include <cctype>
#include <locale>
#include <string>

// trim from start (in place)
inline void ltrim(std::string &s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
        return !std::isspace(ch);
    }));
}

// trim from end (in place)
inline void rtrim(std::string &s) {
    s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
        return !std::isspace(ch);
    }).base(), s.end());
}

// trim from both ends (in place)
inline void trim(std::string &s) {
    rtrim(s);
    ltrim(s);
}

inline bool IsNumeric(const std::string& s)
{
    if (s.length() == 0) return false;
    for (int i = 0; i < s.length(); ++i)
    {
        if ((s[i] >= '0' && s[i] <= '9')
        || s[i] == ' '
        || s[i] == '.'
        || (s[i] == '-' && i == 0)) 
            continue;
        return false;
    }
    return true;
}

/// Returns the number of ticks since an undefined time (usually system startup).
inline uint64_t GetTickCountMs()
{
    /*struct timespec ts;

    clock_gettime(CLOCK_MONOTONIC, &ts);

    return (uint64_t)(ts.tv_nsec / 1000000) + ((uint64_t)ts.tv_sec * 1000ull);*/

    return GetTickCount64();
}