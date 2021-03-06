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
