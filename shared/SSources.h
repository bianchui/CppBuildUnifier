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

#ifndef __shared_SSources_h__
#define __shared_SSources_h__
#include <string>
#include <vector>
//#include <regex>
#include <map>
#include <set>
#include <istream>

struct ELogLevel {
    enum Enum {
        NONE,
        ERROR,
        WARNING,
        INFO,
        LDEBUG,
        LTRACE,
        VERBOSE,
    };

    static Enum GetLogLevel() {
        return g_logLevel;
    }
    static void SetLogLevel(Enum level) {
        g_logLevel = level;
    }
    static Enum FromString(const char* str);

    static bool ParseLogLevel(const char* arg) {
        bool ok = false;
        Enum level = NONE;
        if (0 == strcasecmp(arg, "v")) {
            level = ELogLevel::VERBOSE;
            ok = true;
        } else if (0 == strncasecmp(arg, "l=", 2)) {
            level = ELogLevel::FromString(arg + 2);
            ok = true;
        } else if (0 == strncasecmp(arg, "log=", 4)) {
            level = ELogLevel::FromString(arg + 4);
            ok = true;
        } else if (0 == strncasecmp(arg, "loglevel=", 9)) {
            level = ELogLevel::FromString(arg + 9);
            ok = true;
        }
        if (ok) {
            ELogLevel::SetLogLevel(level);
        }
        return ok;
    }

private:
    static Enum g_logLevel;
};

#define LOG_E(...) if (ELogLevel::GetLogLevel() >= ELogLevel::ERROR) { fprintf(stderr, "Error: " __VA_ARGS__); }
#define LOG_E_ONLY(x) if (ELogLevel::GetLogLevel() >= ELogLevel::ERROR) { (x); }
#define LOG_W(...) if (ELogLevel::GetLogLevel() >= ELogLevel::WARNING) { printf("Warning: " __VA_ARGS__); }
#define LOG_W_ONLY(x) if (ELogLevel::GetLogLevel() >= ELogLevel::WARNING) { (x); }
#define LOG_I(...) if (ELogLevel::GetLogLevel() >= ELogLevel::INFO) { printf(__VA_ARGS__); }
#define LOG_D(...) if (ELogLevel::GetLogLevel() >= ELogLevel::LDEBUG) { printf(__VA_ARGS__); }
#define LOG_T(...) if (ELogLevel::GetLogLevel() >= ELogLevel::LTRACE) { printf(__VA_ARGS__); }
#define LOG_V(...) if (ELogLevel::GetLogLevel() >= ELogLevel::VERBOSE) { printf(__VA_ARGS__); }

struct SSources {
protected:
    std::string _root;
    std::vector<std::string> _alreadyProcessedFiles;
    std::vector<std::string> _files;
    std::vector<std::string> _exclude;
    std::set<std::string> _exts;
    // map<to, from[]>
    std::map<std::string, std::vector<std::string>> _extMap;
    const std::set<std::string> *_allFiles;
    bool _unified;
    bool _forceUnify;

    std::string _unified_Path;
    std::string _unified_RelativeRoot;

    // stats
    struct Stats {
        uint32_t scanDirs;
        uint32_t scanFiles;
        uint32_t scanErrors;

        uint32_t excludeDirs;
        uint32_t excludeFiles;
        uint32_t skipFilesNoExt;
        uint32_t skipFilesByExt;
        uint32_t skipFilesByFileLists;

        uint32_t singleFiles;
        uint32_t unifiedFiles;
        uint32_t minUnifyFiles;
        uint32_t maxUnifyFiles;

        uint32_t excludes() const {
            return excludeDirs + excludeFiles;
        }

        uint32_t skips() const {
            return skipFilesNoExt + skipFilesByExt + skipFilesByFileLists;
        }
    };

    Stats _stats;

public:
    SSources() : _unified(true), _unified_Path("@unified_build"), _unified_RelativeRoot("../"), _allFiles(NULL) {
        memset(&_stats, 0, sizeof(Stats));
    }
    void setUnified(bool unified) {
        _unified = unified;
    }
    bool load(const char* root, const char* src_list);
    bool mergelist(const char* list, const std::string& relative = "");

    void addExt(const char* ext);
    void addExtMap(const char* from, const char* to);

    void exclude(const char* path, const std::string& relative = "");
    bool addPath(const char* path, const std::string& relative = "");

    bool isExclude(const char* path) const;

    void ensureUnifiedDir();

    void printStats();

private:
    bool load(std::istream& is_list, std::string path_prefix);

    struct SUnifyUnit {
        SSources* sources;
        std::string unifiedRoot;
        std::map<std::string, std::vector<std::string>> extfiles;
        
        bool add(const char* file);
        bool commit();
    };

    bool addDir(SUnifyUnit& bu, const std::string& path, bool recursive);

    bool addFile(const char* path);
};

#endif//__shared_SSources_h__
