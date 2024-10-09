#pragma once
#ifndef UTILS_H
#define UTILS_H


inline bool isNumeric(const std::string& s)
{
    if (s.length() == 0) return false;
    for (int32_t i = 0; i < s.length(); ++i)
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

#endif // UTILS_H