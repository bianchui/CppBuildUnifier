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

#include "namehash.h"

// ascii
//0123456789ABCDEF0123456789ABCDEF
// !"#$%&'()*+,-./0123456789:;<=>?
//@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\]^_
//`abcdefghijklmnopqrstuvwxyz{|}~

// encoded, order by mac file system
//012345678  9ABCDEF0   123  456789ABCDEF
//  _-,;!?.'"()[]{}@*:\/&#%`^+<=>|~$01234

//0123456789ABCDEF0123456789ABCDEF
//56789abcdefghijklmnopqrstuvwxyz


static char g_code[256] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,

    //   0   1   2   3   4   5   6   7   8   9   A   B   C   D   E   F
    //       !   "   #   $   %   &   '   (   )   *   +   ,   -   .   /
    /**/ 1,  6,  9, 18, 26, 19, 17,  9,  9, 10, 16, 20,  4,  3,  8, 17,
    //   0   1   2   3   4   5   6   7   8   9   A   B   C   D   E   F
    //   0   1   2   3   4   5   6   7   8   9   :   ;   <   =   >   ?
    /**/27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 17,  5, 21, 22, 23,  7,

    //   0   1   2   3   4   5   6   7   8   9   A   B   C   D   E   F
    //   @   A   B   C   D   E   F   G   H   I   J   K   L   M   N   O
    /**/15, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51,
    //   0   1   2   3   4   5   6   7   8   9   A   B   C   D   E   F
    //   P   Q   R   S   T   U   V   W   X   Y   Z   [   \   ]   ^   _
    /**/52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 11, 17, 12, 20,  2,

    //   0   1   2   3   4   5   6   7   8   9   A   B   C   D   E   F
    //   `   a   b   c   d   e   f   g   h   i   j   k   l   m   n   o
    /**/20, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51,
    //   0   1   2   3   4   5   6   7   8   9   A   B   C   D   E   F
    //   p   q   r   s   t   u   v   w   x   y   z   {   |   }   ~
    /**/52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 13, 24, 14, 25, 63,

    63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63,
    63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63,
    63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63,
    63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63,

    63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63,
    63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63,
    63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63,
    63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63,
};

static unsigned char gethashchar(char c) {
    unsigned char uc = c;
    if (uc >= 'A' && uc <= 'Z') {
        uc += 32;
    }
    return uc;
}

// seed = 31 131 1313 13131 131313 etc..
uint32_t HashPath(const char* path, uint32_t seed) {
    uint32_t hash = 0;
    while (*path) {
        hash = hash * seed + gethashchar(*path++);
    }
    return hash;
}

uint32_t HashString(const char* str, uint32_t seed) {
    uint32_t hash = 0;
    while (*str) {
        hash = hash * seed + (*str++);
    }
    return hash;
}

std::string encode(const char* path) {
    char buf[32];

    while (*path) {
        ++path;
    }

    return buf;
}

