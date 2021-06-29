// Copyright (c) 2018-2021 bianchui https://github.com/bianchui
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#ifndef __shared_str_utils_h__
#define __shared_str_utils_h__
#include <string>
#include <string.h>
#include <algorithm>

// trim from start (in place)
inline void ltrim(std::string &s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](int ch) {
        return !std::isspace(ch);
    }));
}

// trim from end (in place)
inline void rtrim(std::string &s) {
    s.erase(std::find_if(s.rbegin(), s.rend(), [](int ch) {
        return !std::isspace(ch);
    }).base(), s.end());
}

// trim from both ends (in place)
inline void trim(std::string &s) {
    ltrim(s);
    rtrim(s);
}

inline void lowercase(std::string& s) {
    std::transform(s.begin(), s.end(), s.begin(), ::tolower);
}

inline bool stricasecmp(const std::string& a, const std::string& b) {
    return ::strcasecmp(a.c_str(), b.c_str()) < 0;
}

inline bool isBeginWith(const std::string& src, const std::string& begin) {
    return src.length() > begin.length() && strncmp(src.c_str(), begin.c_str(), begin.length()) == 0;
}

inline bool isEndOf(const std::string& src, const std::string& end) {
    return src.length() > end.length() && strcmp(src.c_str() + src.length() - end.length(), end.c_str()) == 0;
}

#endif//__shared_str_utils_h__
