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

#include "XcodeProjUnifier.hpp"
#include "shared/str_utils.h"
#include "shared/file_utils.h"
#include <shared/utils/Path.h>
#include "SXcodeSources.h"

void XcodeProjUnifier::printInfosToFile(std::string name) {
    shared::StrBuf buf;
    if (false) {
        Impl::debugDump(buf);
        std::string projFileName_out = name + ".dump.txt";
        saveContentWithCheck(projFileName_out.c_str(), buf);
        buf.clear();
    }
    if (false) {
        _plist.write(0, buf);
        std::string projFileName_out = name + ".plist.txt";
        saveContentWithCheck(projFileName_out.c_str(), buf);
        buf.clear();
    }
    if (false) {
        Impl::write(buf);
        //printf("%s", buf.c_str());

        std::string projFileName_out = name + ".out";
        saveContentWithCheck(projFileName_out.c_str(), buf);
        buf.clear();
    }
}

bool XcodeProjUnifier::makeXcodeproj(const char* proj_path, const char* proj_name) {
    std::string content;
    shared::Path projFilePath(proj_path, proj_name);
    shared::Path projFileName(projFilePath.c_str(), "project.pbxproj");
    if (!loadContent(projFileName.c_str(), content) || content.length() == 0) {
        LOG_E("Unable to load %s\n", projFileName.c_str());
        return false;
    }

    if (!Impl::inlineParse((char*)content.c_str())) {
        LOG_E("Invalid project\n");
        return false;
    }

    std::set<std::string> allFiles;

    Impl::getAllFiles(allFiles);

    printInfosToFile(projFileName.string() + ".1");

    for (size_t i = 0; i < targetCount(); ++i) {
        SXcodeSources srcs;
        srcs.setUnified(_unified);
        auto target = Impl::target(i);
        const char* targetName = target->stringByKey("name");
        if (!targetName) {
            return false;
        }
        srcs.setAllFiles(&allFiles);
        LOG_W_ONLY(printf("========== %s ==========\n", targetName));
        if (!srcs.loadList(proj_path, targetName)) {
            LOG_W("Skip target %s\n", targetName);
            continue;
        }
        NeXTSTEP::Array* build_files = Impl::targetGetBuildFiles(target);
        if (!build_files) {
            return false;
        }

        // cleanup build files
        {
            bool skipInc = false;
            for (auto iter = build_files->begin(); iter != build_files->end(); skipInc = skipInc ? false : (++iter, false)) {
                if (iter->isString()) {
                    pbxproj::ProjectItem buildFile(_objects->objectByKey(iter->string()));
                    if (!buildFile.isa("PBXBuildFile")) {
                        return false;
                    }
                    NeXTSTEP::Object* fileRef = _objects->objectByKey(buildFile->stringByKey("fileRef"));
                    if (!fileRef) {
                        return false;
                    }
                    skipInc = true;
                    _pathmap.erase(iter->string());
                    removeItem(iter->string());
                    iter = build_files->erase(iter);
                }
            }
        }

        NeXTSTEP::Object* target_group = Impl::targetGetUnifiedRoot(targetName);
        if (!target_group) {
            return false;
        }
        {
            NeXTSTEP::Array* target_children = target_group->arrayByKey("children");
            if (!target_children) {
                return false;
            }

            // cleanup target group
            for (auto iter = target_children->begin(); iter != target_children->end(); ++iter) {
                if (iter->isString()) {
                    _pathmap.erase(iter->string());
                    removeItem(iter->string());
                }
            }
            target_children->clear();
        }

        {
            const std::string begin_unified = std::string(SXcodeSources::Unified_Path) + "/";
            for (auto iter = srcs.files().begin(); iter != srcs.files().end(); ++iter) {
                const auto& path = *iter;
                std::string build_key;
                std::string file_key;
                std::string path_dir, path_file;
                {
                    auto file_pos = path.rfind("/");
                    path_file = path.substr(file_pos + 1);
                    if (std::string::npos != file_pos) {
                        path_dir = path.substr(0, file_pos);
                    }
                    build_key = Impl::KeyForBuildFile(path_dir.c_str(), path_file.c_str(), targetName);
                    file_key = Impl::KeyForGroupOrFile(path_dir.c_str(), path_file.c_str());
                }

                if (isBeginWith(path, begin_unified)) {
                    if (!getOrNewChildFile(target_group, path_file.c_str())) {
                        return false;
                    }
                } else {
                    // find org fileRef and file_key from path
                    const pbxproj::PBXPath* pathInfo = Impl::findGroupOrFilePath(path.c_str());
                    if (!pathInfo) {
                        return false;
                    }
                    file_key = pathInfo->key;
                }

                NeXTSTEP::Object* buildFile = new NeXTSTEP::Object();
                buildFile->set("isa", "PBXBuildFile");
                buildFile->set("fileRef", file_key.c_str());

                addItem(new NeXTSTEP::KeyValue(build_key.c_str(), buildFile));

                build_files->push_back(NeXTSTEP::Value::NewString(build_key.c_str()));
            }
        }
        if (_stats) {
            srcs.printStats();
        }
    }

    printInfosToFile(projFileName.string() + ".2");

    shared::StrBuf buf;
    Impl::write(buf);
    return saveContentWithCheck(projFileName.c_str(), buf);
}
