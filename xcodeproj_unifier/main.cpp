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
#include "XcodeProjUnifier.hpp"
#include <stdio.h>
#include <stdint.h>
#include <shared/utils/Path.h>
#include <unistd.h>
#include "shared/str_utils.h"
#include "shared/file_utils.h"
#include <dirent.h>

bool isProjectFile(const char* file) {
    const char* ext = fileext(file, ".");
    return strcmp(ext, ".xcodeproj") == 0;
}

bool findProjects(const char* path, std::vector<std::string>& projects) {
    DIR* dir = opendir(path);
    if (!dir) {
        LOG_E_ONLY(perror(path));
        return false;
    }
    struct stat info;
    struct dirent* item;
    while ((item = readdir(dir)) != NULL) {
        if (item->d_name[0] == '.') {
            continue;
        }
        if (!isProjectFile(item->d_name)) {
            continue;
        }
        if (stat(shared::Path(path, item->d_name).c_str(), &info) < 0) {
            LOG_E_ONLY(perror(item->d_name));
            continue;
        }
        if (S_ISDIR(info.st_mode)) {
            projects.push_back(item->d_name);
        }
    }
    closedir(dir);
    return true;
}

void help(const char* cmd) {
    printf("xcode project unifier, v2022.0914, Copyright 2018-2022 bianchui@github.com .\n");
    printf("usage:\n%s [-no] [-project projname] [dir]\n", cmd);
    printf("  -no       disable unifier\n");
}

int main(const int argc, const char * argv[]) {
    const char* cwd = getcwd(NULL, 0);
    const char* srcdir = NULL;
    const char* projName = NULL;
    XcodeProjUnifier unifier;
    for (int i = 1; i < argc; ++i) {
        if (argv[i][0] == '-') {
            if (0 == strcasecmp(argv[i] + 1, "h") || 0 == strcasecmp(argv[i] + 1, "help")) {
                help(argv[0]);
                return 0;
            } else if (0 == strcasecmp(argv[i] + 1, "no")) {
                unifier._unified = false;
            } else if (0 == strcasecmp(argv[i] + 1, "project")) {
                if (i + 1 < argc) {
                    projName = argv[++i];
                }
            } else if (ELogLevel::ParseLogLevel(argv[i] + 1)) {

            } else if (0 == strcasecmp(argv[i] + 1, "stat")) {
                unifier._stats = true;
            }
        } else {
            srcdir = argv[i];
        }
    }

    if (ELogLevel::GetLogLevel() >= ELogLevel::INFO) {
        unifier._stats = true;
    }

    if (srcdir == NULL) {
        srcdir = cwd;
    }
    std::vector<std::string> projects;
    if (projName) {
        projects.push_back(projName);
    }
    shared::Path path(cwd, srcdir);
    if (projects.empty()) {
        findProjects(path.c_str(), projects);
    }
    size_t count = 0;
    for (auto iter = projects.begin(); iter != projects.end(); ++iter) {
        std::string proj_name = *iter;
        std::string xcodeproj = ".xcodeproj";
        if (!isEndOf(proj_name, xcodeproj)) {
            proj_name.append(xcodeproj);
        }

        if (unifier.makeXcodeproj(path.c_str(), proj_name.c_str())) {
            ++count;
        }
    }
    if (!count) {
        LOG_E("No project built\n");
    }

    return 0;
}
