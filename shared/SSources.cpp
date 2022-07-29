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

#include "SSources.h"
#include <iostream>
#include <sstream>
#include <fstream>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <shared/utils/Path.h>
#include <shared/SharedMacros.h>
#include "str_utils.h"
#include "file_utils.h"

ELogLevel::Enum ELogLevel::g_logLevel = ELogLevel::ERROR;

ELogLevel::Enum ELogLevel::FromString(const char* str) {
    Enum level = ELogLevel::NONE;
    struct LevelString {
        ELogLevel::Enum level;
        const char* str;
    };
    static const LevelString ls[] = {
        {ELogLevel::NONE, "n"},
        {ELogLevel::NONE, "none"},

        {ELogLevel::ERROR, "e"},
        {ELogLevel::ERROR, "error"},

        {ELogLevel::WARNING, "w"},
        {ELogLevel::WARNING, "warn"},
        {ELogLevel::WARNING, "warning"},

        {ELogLevel::INFO, "i"},
        {ELogLevel::INFO, "info"},

        {ELogLevel::LDEBUG, "d"},
        {ELogLevel::LDEBUG, "debug"},

        {ELogLevel::LTRACE, "t"},
        {ELogLevel::LTRACE, "trace"},

        {ELogLevel::VERBOSE, "v"},
        {ELogLevel::VERBOSE, "verbose"},
    };
    for (size_t i = 0; i < _countof(ls); ++i) {
        if (strcasecmp(ls[i].str, str) == 0) {
            level = ls[i].level;
            break;
        }
    }
    return level;
}

template <typename T>
bool contains(const std::vector<T>& vec, const T& item) {
    return std::find(vec.begin(), vec.end(), item) != vec.end();
}

static void removeComment(std::string& str) {
    std::string::size_type it = str.find('#');
    if (it != std::string::npos) {
        str.erase(str.begin() + it, str.end());
    }
}

//SSources
void SSources::addExt(const char* ext) {
    if (ext) {
        if (*ext == '*') {
            ++ext;
        }
        if (*ext == '.') {
            ++ext;
        }
        std::string ext_(ext);
        lowercase(ext_);
        _exts.insert(ext);
    }
}

//.to=.from1,.from2,.from3
//merge '.from1' '.from2' '.from3' to '.to' if '.to' and '.from*' both exist
void SSources::addExtMap(const char* from, const char* to) {
    if (from && to) {
        if (*from == '*') {
            ++from;
        }
        if (*from == '.') {
            ++from;
        }
        if (*to == '*') {
            ++to;
        }
        if (*to == '.') {
            ++to;
        }
        std::string from_(from);
        lowercase(from_);
        std::string to_(to);
        lowercase(to_);
        if (from_ != to_ && _extMap.find(from_) == _extMap.end()) {
            auto& item = _extMap[to_];
            item.insert(from_);
        }
    }
}

bool SSources::load(const char* root, const char* src_list) {
    _root = root;
    if (_root.length() && _root.back() != '/') {
        _root.push_back('/');
    }
    std::ifstream is_list((_root + src_list).c_str());
    return load(is_list, "");
}

bool SSources::load(std::istream& is_list, std::string list_relative) {
    if (!is_list) {
        return false;
    }
    std::string line;
    while (std::getline(is_list, line)) {
        removeComment(line);
        trim(line);
        if (!line.empty()) {
            switch (line[0]) {
                case '-':
                    exclude(line.c_str() + 1, list_relative);
                    break;

                case ':':
                    mergelist(line.c_str() + 1, list_relative);
                    break;
                    
                case '=':
                    addFile(line.c_str() + 1);
                    break;

                case '*':
                case '.':
                    if (line.find('/') == std::string::npos) {
                        const auto pos = line.find('=');
                        if (pos != std::string::npos) {
                            std::string to = line.substr(0, pos);
                            std::string from = line.substr(pos + 1);
                            std::string::size_type i = 0;
                            addExt(to.c_str());
                            while (true) {
                                std::string::size_type nexti = from.find(',', i);
                                std::string ext = from.substr(i, nexti);
                                if (ext.length()) {
                                    addExt(ext.c_str());
                                    addExtMap(ext.c_str(), to.c_str());
                                }
                                if (nexti == std::string::npos) {
                                    break;
                                }
                                i = nexti + 1;
                            }
                        } else {
                            addExt(line.c_str());
                        }
                        break;
                    }
                    if (line[0] == '*') {
                        break;
                    }

                default:
                    if (!addPath(line.c_str(), list_relative)) {
                        return false;
                    }
            }
        }
    }

    return true;
}

bool SSources::mergelist(const char* list, const std::string& relative) {
    shared::Path re_list(relative.c_str(), list);
    shared::Path path(_root.c_str(), re_list.c_str());
    std::ifstream is_list(path.c_str());
    std::string dir = re_list.dir();
    LOG_D("MergeList:%s relative %s\n", re_list.c_str(), dir.c_str());
    return load(is_list, dir);
}

void SSources::exclude(const char* path, const std::string& relative) {
    if (path && *path) {
        _exclude.push_back(shared::Path(relative.c_str(), path).string());
    }
}

bool SSources::isExclude(const char* path) const {
    for (auto iter = _exclude.begin(); iter != _exclude.end(); ++iter) {
        const auto& filter = *iter;
        if (strncasecmp(filter.c_str(), path, filter.length()) == 0) {
            const char* sub = path + filter.length();
            if (filter.back() == '/') {
                if (strchr(sub, '/') == NULL) {
                    LOG_D("Exclude:%s by %s\n", path, filter.c_str());
                    return true;
                }
                continue;
            }
            if (*sub == '/' || *sub == 0) {
                LOG_D("Exclude:%s by %s\n", path, filter.c_str());
                return true;
            }
        }
    }
    return false;
}

void SSources::ensureUnifiedDir() {
    struct stat info;
    shared::Path unifiedPath(_root.c_str(), _unified_Path.c_str());
    if (stat(unifiedPath.c_str(), &info) < 0) {
        std::string src = _unified_Path;
        src.append("a.txt");
        CreateDirs(_root, src);
    }
}

bool SSources::addDir(SUnifyUnit& bu, const std::string& path, bool recursive) {
    ++_stats.scanDirs;
    //std::string dirname = getDirName(path.c_str());
    struct stat info;
    
    DIR* dir = opendir(shared::Path(_root.c_str(), path.c_str()).c_str());
    if (!dir) {
        LOG_E_ONLY(perror(path.c_str()));
        ++_stats.scanErrors;
        return false;
    }
    struct dirent* item;
    std::string subpath;
    bool success = true;
    while (success && (item = readdir(dir)) != NULL) {
        if (item->d_name[0] == '.') {
            continue;
        }
        subpath = path + item->d_name;
        if (stat(shared::Path(_root.c_str(), subpath.c_str()).c_str(), &info) < 0) {
            LOG_E_ONLY(perror(subpath.c_str()));
            ++_stats.scanErrors;
            continue;
        }
        if (isExclude(subpath.c_str())) {
            //LOG_V("Exclude:%s\n", subpath.c_str());
            if (S_ISREG(info.st_mode)) {
                ++_stats.scanFiles;
                ++_stats.excludeFiles;
            } else if (S_ISDIR(info.st_mode)) {
                ++_stats.scanDirs;
                ++_stats.excludeDirs;
            }
            continue;
        }

        if (S_ISREG(info.st_mode)) {
            ++_stats.scanFiles;
            const char* ext = fileext(subpath.c_str());
            if (!ext) {
                ++_stats.skipFilesNoExt;
                continue;
            }
            if (_exts.find(ext + 1) == _exts.end()) {
                LOG_D("Skip by ext:%s\n", subpath.c_str());
                ++_stats.skipFilesByExt;
                continue;
            }
            if (_allFiles && !_allFiles->empty()) {
                if (_allFiles->find(subpath) == _allFiles->end()) {
                    ++_stats.skipFilesByFileLists;
                    LOG_D("Exclude by filelist:%s\n", subpath.c_str());
                    continue;
                }
            }
            
            success = _unified ? bu.add(subpath.c_str()) : addFile(subpath.c_str());
        }
        if (S_ISDIR(info.st_mode) && recursive) {
            subpath.push_back('/');
            success = addDir(bu, subpath, recursive);
        }
    }
    closedir(dir);
    return success;
}

bool SSources::addFile(const char* path) {
    if (_allFiles && !_allFiles->empty()) {
        if (_allFiles->find(path) == _allFiles->end()) {
            LOG_D("Exclude by filelist:%s\n", path);
            ++_stats.skipFilesByFileLists;
            return true;
        }
    }
    LOG_D("Add:%s\n", path);
    ++_stats.singleFiles;

    _files.push_back(path);
    return true;
}

bool SSources::addPath(const char* inPath, const std::string& relative) {
    LOG_I("Path:%s> %s\n", relative.c_str(), inPath);
    std::string path(inPath);
    bool recursive = true;
    bool file = false;
    auto pos = path.find(':');
    if (pos != std::string::npos) {
        SUnifyUnit bu;
        bu.sources = this;
        bu.unifiedRoot = shared::Path(relative.c_str(), path.substr(0, pos).c_str()).string();
        if (bu.unifiedRoot.length() > 0 && bu.unifiedRoot.back() != '/') {
            bu.unifiedRoot.push_back('/');
        }
        while (pos != std::string::npos) {
            auto next = path.find(',', pos + 1);
            std::string sub = path.substr(pos + 1, next - pos - 1);
            pos = next;
            if (sub.length() == 0) {
                continue;
            }
            std::string subFull = bu.unifiedRoot + (sub[0] == '/' ? sub.c_str() + 1 : sub.c_str());
            recursive = true;
            file = false;
            if (subFull.length() > 0 && subFull.back() == '/') {
                recursive = false;
            } else if (isDir(shared::Path(_root.c_str(), subFull.c_str()).c_str())) {
                subFull.push_back('/');
            } else {
                file = true;
            }
            if (!file) {
                if (!addDir(bu, subFull.c_str(), recursive)) {
                    return false;
                }
            } else {
                ++_stats.scanFiles;
                if (_unified) {
                    if (!bu.add(subFull.c_str())) {
                        return false;
                    }
                } else {
                    if (!addFile(path.c_str())) {
                        return false;
                    }
                }
            }
        }
        return (!_unified || bu.commit());
    }
    path = (shared::Path(relative.c_str(), path.c_str())).string();
    if (path.length() > 0 && path.back() == '/') {
        recursive = false;
    } else if (isDir(shared::Path(_root.c_str(), path.c_str()).c_str())) {
        path.push_back('/');
    } else {
        file = true;
    }
    if (!file) {
        SUnifyUnit bu;
        bu.sources = this;
        bu.unifiedRoot = path;
        return addDir(bu, path.c_str(), recursive) && (!_unified || bu.commit());
    } else {
        ++_stats.scanFiles;
        return addFile(path.c_str());
    }
}

//SSources::SUnifyUnit
bool SSources::SUnifyUnit::add(const char* file) {
    std::string ext(fileext(file, ".") + 1);
    lowercase(ext);
    std::vector<std::string>& files = extfiles[ext];

    LOG_D("Unified:%s\n", file);
    std::string sFile = file;
    if (!contains(files, sFile)) {
        files.push_back(std::move(sFile));
    }

    return true;
}

bool SSources::SUnifyUnit::commit() {
    if (sources->_extMap.size()) {
        for (auto toI = sources->_extMap.begin(); toI != sources->_extMap.end(); ++toI) {
            auto to = extfiles.find(toI->first);
            size_t mergetExtCount = to != extfiles.end() ? 1 : 0;
            std::vector<std::string> mergedFiles;
            std::vector<std::string>& merged = to != extfiles.end() ? to->second : mergedFiles;
            for (auto fromI = toI->second.begin(); fromI != toI->second.end(); ++fromI) {
                auto from = extfiles.find(*fromI);
                if (from != extfiles.end()) {
                    merged.insert(merged.end(), from->second.begin(), from->second.end());
                    ++mergetExtCount;
                }
            }
            if (mergetExtCount > 1) {
                if (to == extfiles.end()) {
                    std::swap(extfiles[toI->first], mergedFiles);
                }
                for (auto fromI = toI->second.begin(); fromI != toI->second.end(); ++fromI) {
                    extfiles.erase(*fromI);
                }
            }
        }
    }
    for (auto iter = extfiles.begin(); iter != extfiles.end(); ++iter) {
        auto& files = iter->second;
        if (files.size() == 1) {
            ++sources->_stats.singleFiles;
            LOG_I("Commit:single:%s\n", files[0].c_str());
            sources->_files.push_back(files[0]);
        } else {
            Stats& stats = sources->_stats;
            stats.unifiedFiles += (uint32_t)files.size();
            stats.minUnifyFiles = (stats.minUnifyFiles == 0 || files.size() < stats.minUnifyFiles) ? (uint32_t)files.size() : stats.minUnifyFiles;
            stats.maxUnifyFiles = stats.maxUnifyFiles < files.size() ? (uint32_t)files.size() : stats.maxUnifyFiles;
            sources->ensureUnifiedDir();
            std::sort(files.begin(), files.end(), stricasecmp);
            std::string unifiedPath = sources->_unified_Path;
            unifiedPath.append(getClearFileName(unifiedRoot.c_str()));
            unifiedPath.append(iter->first);
            unifiedPath.push_back('.');
            unifiedPath.append(iter->first);
            std::ostringstream os;
            for (auto fi = files.begin(); fi != files.end(); ++fi) {
                os << "#include \"" << sources->_unified_RelativeRoot << *fi << "\"\n";
            }
            if (!saveContentWithCheck(shared::Path(sources->_root.c_str(), unifiedPath.c_str()).c_str(), os.str())) {
                return false;
            }
            sources->_files.push_back(unifiedPath);
            LOG_I("Commit:unified:%s\n", unifiedPath.c_str());
        }
    }

    return true;
}

void SSources::printStats() {
    printf("================= Stats ================\n");
    printf("Scaned:: %d\n", _stats.scanDirs + _stats.scanFiles);
    printf("  Dirs: %d\n", _stats.scanDirs);
    printf("  Files: %d\n", _stats.scanFiles);
    if (_stats.scanErrors) {
        printf("  Errors: %d\n", _stats.scanErrors);
    }
    if (_stats.excludes()) {
        printf("Excludes: %d\n", _stats.excludes());
    }
    if (_stats.excludeDirs) {
        printf("  Dirs: %d\n", _stats.excludeDirs);
    }
    if (_stats.excludeFiles) {
        printf("  Files: %d\n", _stats.excludeFiles);
    }
    if (_stats.skips()) {
        printf("Skip: %d\n", _stats.skips());
    }
    if (_stats.skipFilesNoExt) {
        printf("  Without ext: %d\n", _stats.skipFilesNoExt);
    }
    if (_stats.skipFilesByExt) {
        printf("  Ext filter: %d\n", _stats.skipFilesByExt);
    }
    if (_stats.skipFilesByFileLists) {
        printf("  Filelists filter: %d\n", _stats.skipFilesByFileLists);
    }
    printf("Files: %d -> %d\n", _stats.singleFiles + _stats.unifiedFiles, (int)_files.size());
    printf("  Single: %d\n", _stats.singleFiles);
    printf("  Unified: %d\n", _stats.unifiedFiles);
    printf("    Unify min: %d\n", _stats.minUnifyFiles);
    printf("    Unify max: %d\n", _stats.maxUnifyFiles);
    printf("========================================\n");
}
