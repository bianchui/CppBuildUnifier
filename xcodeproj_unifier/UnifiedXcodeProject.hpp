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

#ifndef UnifiedXcodeProject_hpp
#define UnifiedXcodeProject_hpp
#include "xcodeproj/pbxproj_parser.hpp"
#include <set>

class UnifiedXcodeProject : public pbxproj::Project {
public:
    void getAllFiles(std::set<std::string>& files);

    NeXTSTEP::Object* targetGetUnifiedRoot(const char* target);
    NeXTSTEP::Array* targetGetBuildFiles(NeXTSTEP::Object* target);

    NeXTSTEP::Object* getOrNewChildGroupOrFile(NeXTSTEP::Object* parent, const char* name, bool file);
    NeXTSTEP::Object* getOrNewChildGroup(NeXTSTEP::Object* parent, const char* name) {
        return getOrNewChildGroupOrFile(parent, name, false);
    }
    NeXTSTEP::Object* getOrNewChildFile(NeXTSTEP::Object* parent, const char* name) {
        return getOrNewChildGroupOrFile(parent, name, true);
    }

    static std::string KeyForGroupOrFile(const char* path, const char* name);
    static std::string KeyForBuildFile(const char* path, const char* file, const char* target);
};

#endif//UnifiedXcodeProject_hpp
