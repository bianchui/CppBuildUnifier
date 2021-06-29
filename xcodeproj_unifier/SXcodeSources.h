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

#ifndef xcodeproj_h__
#define xcodeproj_h__
#include "shared/SSources.h"

struct SXcodeSources : SSources {
    static constexpr const char* Unified_Path = "@unified_targets";
    static constexpr const char* Unified_RelativeRoot = "../../";

public:
    SXcodeSources() {
        _unified_RelativeRoot = Unified_RelativeRoot;

        addExt(".c");
        addExt(".cpp");
        addExt(".cxx");
        addExt(".m");
        addExt(".mm");
    }

    void setAllFiles(const std::set<std::string>* allFiles) {
        _allFiles = allFiles;
    }

    bool loadList(const char* dir, const char* target) {
        std::string src_list = target;
        src_list.append(".list");

        _unified_Path = Unified_Path;
        _unified_Path.append(1, '/');
        _unified_Path.append(target);
        _unified_Path.append(1, '/');

        return load(dir, src_list.c_str());
    }

    const std::vector<std::string>& files() const {
        return _files;
    };
};

#endif//xcodeproj_h__
