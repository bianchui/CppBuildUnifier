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

#include "shared/SSources.h"
#include <sstream>
#include <string>
#include <vector>
//#include <regex>
#include <stdio.h>
#include <unistd.h>
#include <shared/utils/Path.h>
#include "Android.mk.h"
#include "shared/str_utils.h"
#include "shared/file_utils.h"

struct SAndroidSources : SSources {

    static constexpr const char* Unified_Path = "@unified_build/android/";
    static constexpr const char* Unified_RelativeRoot = "../../";

public:
    SAndroidSources() {
        _unified_Path = Unified_Path;
        _unified_RelativeRoot = Unified_RelativeRoot;

        addExt(".c");
        addExt(".cpp");
        addExt(".cxx");
    }

    bool load(const char* root);
    bool makeAndroidMK() const;
};

bool SAndroidSources::load(const char* root) {
    if (!SSources::load(root, "Android.list")) {
        LOG_E("Build %s failed!\n", root);
        return false;
    }

    return makeAndroidMK();
}

bool SAndroidSources::makeAndroidMK() const {
    std::string content;
    std::string mkFileName = _root + "Android.mk";
    if (!loadContent(mkFileName.c_str(), content) || content.length() == 0) {
        LOG_E("Unable to load %s\n", mkFileName.c_str());
        return false;
    }
    
    std::ostringstream os;
    for (auto iter = _files.begin(); iter != _files.end(); ++iter) {
        os << "    " << *iter << " \\\n";
    }
    
    std::string replaced = AndroidMk_replace_LOCAL_SRC_FILES(content, os.str());
    if (replaced.length() == 0) {
        LOG_E("Parsing %s error\n", mkFileName.c_str());
        return false;
    }

    return saveContentWithCheck(mkFileName.c_str(), replaced);
}

void help(const char* cmd) {
    printf("Android.mk unifier, v2020.1207, Copyright 2018-2022 bianchui@github.com .\n\n");
    printf("usage:\n%s [-no] [dir]\n\n", cmd);
    printf("  -no       disable unifier\n");
}

int main(const int argc, const char * argv[]) {
    const char* cwd = getcwd(NULL, 0);
    const char* srcdir = NULL;
    bool stats = false;
    SAndroidSources srcs;
    for (int i = 1; i < argc; ++i) {
        if (argv[i][0] == '-') {
            if (0 == strcasecmp(argv[i] + 1, "h") || 0 == strcasecmp(argv[i] + 1, "help")) {
                help(argv[0]);
                return 0;
            } else if (0 == strcasecmp(argv[i] + 1, "no")) {
                srcs.setUnified(false);
            } else if (ELogLevel::ParseLogLevel(argv[i] + 1)) {

            } else if (0 == strcasecmp(argv[i] + 1, "stat")) {
                stats = true;
            }
        } else {
            srcdir = argv[i];
        }
    }
    if (srcdir == NULL) {
        srcdir = cwd;
    }
    shared::Path path(cwd, srcdir);
    if (!srcs.load(path.c_str())) {
        return 1;
    }
    if (stats || ELogLevel::GetLogLevel() >= ELogLevel::INFO) {
        srcs.printStats();
    }
    return 0;
}
