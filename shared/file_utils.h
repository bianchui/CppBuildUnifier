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

#ifndef __shared_file_utils_h__
#define __shared_file_utils_h__
#include <string>
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <shared/utils/StrBuf.h>

inline bool loadContent(const char* name, std::string& data) {
    FILE* f = fopen(name, "rb");
    if (f) {
        fseek(f, 0, SEEK_END);
        size_t len = ftell(f);
        if (len) {
            fseek(f, 0, SEEK_SET);
            data.resize(len);
            size_t w = fread(&data[0], 1, len, f);
            data.resize(w);
        } else {
            data.clear();
        }
        fclose(f);
    }
    return f;
}

inline size_t fileSize(const char* name) {
    size_t len = 0;
    FILE* f = fopen(name, "rb");
    if (f) {
        fseek(f, 0, SEEK_END);
        len = ftell(f);
        fclose(f);
    }
    return len;
}

inline bool saveContent(const char* name, const std::string& data) {
    FILE* f = fopen(name, "wb");
    if (f) {
        if (data.length()) {
            fwrite(&data[0], 1, data.length(), f);
        }
        fclose(f);
    }
    return !!f;
}

inline bool saveContent(const char* name, const shared::StrBuf& data) {
    FILE* f = fopen(name, "wb");
    if (f) {
        if (data.length()) {
            fwrite(data.data(), 1, data.length(), f);
        }
        fclose(f);
    }
    return !!f;
}

inline bool saveContentWithCheck(const char* name, const std::string& data) {
    std::string olddata;
    if (loadContent(name, olddata) && data == olddata) {
        return true;
    }
    return saveContent(name, data);
}

inline bool saveContentWithCheck(const char* name, const shared::StrBuf& data) {
    std::string olddata;
    if (loadContent(name, olddata) && data.same(olddata)) {
        return true;
    }
    return saveContent(name, data);
}

inline bool isDir(const char* path) {
    struct stat info;
    if (stat(path, &info) < 0) {
        perror(path);
        return 0;
    }
    return S_ISDIR(info.st_mode);
}

inline std::string getDirName(const char* dir) {
    std::string dir2(dir);
    size_t len = dir2.length();
    if (len && dir2.back() == '/') {
        --len;
    }
    size_t i = dir2.find_last_of("\\/") + 1;
    return dir2.substr(i, len - i);
}

inline std::string getClearFileName(const char* name) {
    std::string ret(name);
    std::replace(ret.begin(), ret.end(), '.', '_');
    std::replace(ret.begin(), ret.end(), ' ', '_');
    std::replace(ret.begin(), ret.end(), '/', '_');
    return ret;
}

inline const char* fileext(const char* name, const char* def = NULL) {
    const char* pt = strrchr(name, '.');
    const char* sp = strrchr(name, '/');
    return (pt && sp < pt) ? pt : def;
}

inline void CreateDirs(const std::string& path, const std::string& fileName) {
    size_t index = 0;
    for (;;) {
        index = fileName.find('/', index);
        if (index == std::string::npos) {
            break;
        }
        mkdir((path + fileName.substr(0, index)).c_str(), S_IRWXU | S_IRWXG | S_IRWXO);
        ++index;
    }
}

#endif//__shared_file_utils_h__
