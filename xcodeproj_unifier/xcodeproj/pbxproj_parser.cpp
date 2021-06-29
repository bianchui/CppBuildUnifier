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

#include "pbxproj_parser.hpp"
#include <shared/utils/Path.h>
#include <unordered_set>
#include <shared/SharedMacros.h>
#include "shared/file_utils.h"

#define PROJ_VisitAllKnownNode 1
#define PROJ_WarnUnknownNode 1
#define PROJ_WarnUnvisitNode 1

namespace pbxproj {

    struct SectionInfo {
        const char* name;
        bool breakline;
    };

    static const SectionInfo kKnownSections[] = {
        { "PBXBuildFile", false },

        { "PBXFileReference", false },
        { "PBXGroup", true },
        { "PBXVariantGroup", true },

        { "PBXProject", true },
        { "PBXTargetDependency", true },
        { "PBXContainerItemProxy", true },
        { "PBXReferenceProxy", true },
        { "XCBuildConfiguration", true },
        { "XCConfigurationList", true },

        // BuildPhase
        { "PBXBuildPhase", true },
        { "PBXAppleScriptBuildPhase", true },
        { "PBXHeadersBuildPhase", true },
        { "PBXSourcesBuildPhase", true },
        { "PBXFrameworksBuildPhase", true },
        { "PBXCopyFilesBuildPhase", true },
        { "PBXResourcesBuildPhase", true },
        { "PBXShellScriptBuildPhase", true },

        // Target
        { "PBXTarget", true },
        { "PBXAggregateTarget", true },
        { "PBXLegacyTarget", true },
        { "PBXNativeTarget", true },
    };
    static const SectionInfo* sectionInfo(const char* section) {
        static std::unordered_map<const char*, const SectionInfo*, hash_c_str, equal_to_c_str> g_infos;
        if (!g_infos.size()) {
            for (size_t i = 0; i < _countof(kKnownSections); ++i) {
                g_infos.insert({kKnownSections[i].name, kKnownSections + i});
            }
        }
        const auto iter = g_infos.find(section);
        if (iter == g_infos.end()) {
            return NULL;
        }
        return iter->second;
    }

    static bool sectionBreakline(const char* section) {
        const auto info = sectionInfo(section);
        return !info || info->breakline;
    }

    static const std::string kBegin = "/* Begin ";
    static const std::string kEnd = "/* End ";
    static const std::string kSection = " section */";

    bool isBeginWith(const std::string& str, const std::string& begin) {
        return strncmp(str.c_str(), begin.c_str(), begin.length()) == 0;
    }

    bool isEndWith(const std::string& str, const std::string& end) {
        return str.length() >= end.length() && strcmp(str.c_str() + (str.length() - end.length()), end.c_str()) == 0;
    }

    static const char* kSourceTreeType_EnumStrings[] = {
        "",
        "\"<absolute>\"", // Absolute = 1
        "\"<group>\"", // Group
        "SOURCE_ROOT", // Project
        "DEVELOPER_DIR", // Developer
        "SDKROOT", // Sdk
        "BUILT_PRODUCTS_DIR", // Products
    };

    SourceTreeType::Enum SourceTreeType::FromString(const char* src) {
        if (src) {
            for (size_t index = 1; index < _countof(kSourceTreeType_EnumStrings); ++index) {
                if (strcmp(src, kSourceTreeType_EnumStrings[index]) == 0) {
                    return (Enum)index;
                }
            }
        }
        return Unknown;
    }
    const char* SourceTreeType::ToString(Enum src) {
        if (src && src < _countof(kSourceTreeType_EnumStrings)) {
            return kSourceTreeType_EnumStrings[src];
        }
        return kSourceTreeType_EnumStrings[Group];
    }

    const char* SourceTreeType::ToName(Enum src) {
        switch (src) {
#define CASE_TO_STRING(x) case x: return #x;
                CASE_TO_STRING(Unknown);
                CASE_TO_STRING(Absolute);
                CASE_TO_STRING(Group);
                CASE_TO_STRING(Project);
                CASE_TO_STRING(Developer);
                CASE_TO_STRING(Sdk);
                CASE_TO_STRING(Products);
            default:
                return "Error";
        }
    }

    PBXPath::PBXPath(const NeXTSTEP::Object* o, const char* key, const PBXPath* parent, bool file) {
        obj = o;
        path = o->stringByKey("path");
        sourceTree = SourceTreeType::FromString(o->stringByKey("sourceTree"));
        parentKey = parent ? parent->key : NULL;
        this->key = key;
        isFile = file;

        if (path) {
            pathFromSourceTree = unescape(path);
#if 0
            std::string un = escape(pathFromSourceTree.c_str());
            if (strcmp(pathFromSourceTree.c_str(), path) != 0) {
                printf("%s -> %s\n", path, pathFromSourceTree.c_str());
            }
            assert(strcmp(path, un.c_str()) == 0);
            if (strcmp(path, un.c_str()) != 0) {
                printf("Error %s -> %s : %s\n", path, pathFromSourceTree.c_str(), un.c_str());
            }
#endif
        }
        if (sourceTree == SourceTreeType::Group) {
            if (parent) {
                sourceTree = parent->sourceTree;
                if (path) {
                    shared::Path full(parent->pathFromSourceTree.c_str(), pathFromSourceTree.c_str());
                    pathFromSourceTree = full.c_str();
                } else {
                    pathFromSourceTree = parent->pathFromSourceTree;
                }
            }
        }
    }

    void PBXPath::debugDump(shared::StrBuf& buf) const {
        buf.appendf("%s: %s -> %s\n", SourceTreeType::ToName(sourceTree), path, pathFromSourceTree.c_str());
    }

    std::string PBXPath::unescape(std::string path) {
        const char* begin = path.c_str();
        const char* src = begin;
        char* dst = (char*)src;
        bool bEscape = false;

        while (*src) {
            char ch = *(src++);
            if (bEscape) {
                *(dst++) = ch;
                bEscape = false;
            } else {
                if (ch == '\\') {
                    bEscape = true;
                } else if (ch == '"') {

                } else {
                    *(dst++) = ch;
                }
            }
        }
        if (dst - begin < path.length()) {
            path.resize(dst - begin);
        }
        return path;
    }

    std::string PBXPath::escape(std::string path) {
        static const char* kCharsNeedEscape = "\"\\";
        static const char* kCharsNeedQuotes = " !\"#%&'()*+,-:;<=>?@[\\]^`{|}~";

        const char* begin = path.c_str();
        size_t index = 0;
        bool needQuotes = false;
        while (begin[index]) {
            char ch = begin[index];
            if (strchr(kCharsNeedQuotes, ch)) {
                needQuotes = true;
                if (strchr(kCharsNeedEscape, ch)) {
                    path.insert(index, 1, '\\');
                    begin = path.c_str();
                    ++index;
                }
            }
            ++index;
        }
        if (!needQuotes) {
            needQuotes = std::string::npos != path.find("___");
        }
        if (needQuotes) {
            path.insert(0, 1, '"');
            path.insert(path.length(), 1, '"');
        }
        return path;
    }

    Project::Project() : _project(NULL), _objects(NULL) {
    }

    bool Project::inlineParse(char* src) {
        if (!_plist.inlineParse(src) || !_plist.isObject()) {
            return false;
        }

        NeXTSTEP::Object* objs = _plist.objectByKey("objects");
        if (!objs) {
            return false;
        }

        _objects = objs;

        Section section;
        for (auto iter = objs->begin(); iter < objs->end(); ++iter) {
            const auto& key = (*iter)->key;
            if (!key.length()) {
                if (!(*iter)->value.isString()) {
                    return false;
                }
                std::string comment = (*iter)->value.string();
                // comment
                if (isBeginWith(comment, kBegin) && isEndWith(comment, kSection)) {
                    std::string sectionType(comment.c_str() + kBegin.length(), comment.length() - kBegin.length() - kSection.length());
                    assert(section.type.length() == 0);
                    if (section.type.length() != 0) {
                        _sections.push_back(std::move(section));
                        assert(section.type.length() == 0);
                        section.clear();
                        return false;
                    }
                    section.type = sectionType;
                } else if (isBeginWith(comment, kEnd) && isEndWith(comment, kSection)) {
                    std::string sectionType(comment.c_str() + kEnd.length(), comment.length() - kEnd.length() - kSection.length());
                    assert(section.type.length() != 0);
                    if (section.type != sectionType) {
                        return false;
                    }
                    if (section.type.length() != 0) {
                        _sections.push_back(std::move(section));
                        assert(section.type.length() == 0);
                        section.clear();
                    } else {
                        return false;
                    }
                } else {
                    return false;
                }
            } else if ((*iter)->value.isObject()) {
                auto isa = (*iter)->value.stringByKey("isa");
                if (!isa) {
                    return false;
                }
                assert(section.type.length() != 0);
                assert(section.type.compare(isa) == 0);
                if (section.type.compare(isa) != 0) {
                    _sections.push_back(std::move(section));
                    assert(section.type.length() == 0);
                    section.clear();
                    section.type = isa;
                    return false;
                }
                section.items.push_back(*iter);
            } else {
                return false;
            }
        }
        assert(section.type.length() == 0);
        if (section.type.length() != 0) {
            _sections.push_back(std::move(section));
        }

        struct BuildPathContext {
            Project* proj;

            void buildArray(const PBXPath* parentGroup, const NeXTSTEP::Array* arr) {
                if (arr) {
                    for (auto iter = arr->begin(); iter != arr->end(); ++iter) {
                        if (iter->isString()) {
                            build(parentGroup, iter->string());
                        }
                    }
                }
            }

            void build(const PBXPath* parentGroup, const char* key) {
                if (!key) {
                    return;
                }
                auto obj = proj->_objects->objectByKey(key);
                if (!obj) {
                    return;
                }
                const char* isa = obj->stringByKey("isa");
                if (!isa) {
                    return;
                }
                if (strcmp(isa, "PBXProject") == 0) {
                    proj->_project = obj;
                    build(NULL, obj->stringByKey("mainGroup"));
                    buildArray(NULL, obj->arrayByKey("targets"));

                } else if (strcmp(isa, "PBXGroup") == 0) {
                    PBXPath group(obj, key, parentGroup, false);
                    buildArray(&group, obj->arrayByKey("children"));
                    auto iter = proj->_pathmap.find(key);
                    assert(iter == proj->_pathmap.end());
                    proj->_pathmap.insert({key, std::move(group)});

                } else if (strcmp(isa, "PBXFileReference") == 0) {
                    auto iter = proj->_pathmap.find(key);
                    if (!parentGroup) {
                        assert(iter != proj->_pathmap.end());
                        return;
                    }
                    assert(iter == proj->_pathmap.end());
                    if (iter == proj->_pathmap.end()) {
                        PBXPath file(obj, key, parentGroup, true);
                        proj->_pathmap.insert({key, std::move(file)});
                    }

                } else if (strcmp(isa, "PBXNativeTarget") == 0) {
                    proj->_targets.push_back(obj);
                    buildArray(NULL, obj->arrayByKey("buildPhases"));

                } else if (strcmp(isa, "PBXSourcesBuildPhase") == 0) {
                    buildArray(NULL, obj->arrayByKey("files"));

                } else if (strcmp(isa, "PBXFrameworksBuildPhase") == 0) {
#if PROJ_VisitAllKnownNode
                    buildArray(NULL, obj->arrayByKey("files"));
#endif//PROJ_VisitAllKnownNode

                } else if (strcmp(isa, "PBXHeadersBuildPhase") == 0) {
#if PROJ_VisitAllKnownNode
                    buildArray(NULL, obj->arrayByKey("files"));
#endif//PROJ_VisitAllKnownNode

                } else if (strcmp(isa, "PBXCopyFilesBuildPhase") == 0) {
#if PROJ_VisitAllKnownNode
                    buildArray(NULL, obj->arrayByKey("files"));
#endif//PROJ_VisitAllKnownNode

                } else if (strcmp(isa, "PBXResourcesBuildPhase") == 0) {
#if PROJ_VisitAllKnownNode
                    buildArray(NULL, obj->arrayByKey("files"));
#endif//PROJ_VisitAllKnownNode

                } else if (strcmp(isa, "PBXShellScriptBuildPhase") == 0) {
#if PROJ_VisitAllKnownNode
                    buildArray(NULL, obj->arrayByKey("files"));
#endif//PROJ_VisitAllKnownNode

                } else if (strcmp(isa, "PBXReferenceProxy") == 0) {

                } else if (strcmp(isa, "PBXBuildFile") == 0) {
                    build(NULL, obj->stringByKey("fileRef"));
                    
                } else {
#if PROJ_WarnUnknownNode
                    printf("!!!!Error: Unknown %s type\n", isa);
#endif//PROJ_WarnUnknownNode
                }
            }
        };

        BuildPathContext ctx;
        ctx.proj = this;
        ctx.build(NULL, _plist.stringByKey("rootObject"));

        return true;
    }

    const PBXPath* Project::findGroupOrFilePath(NeXTSTEP::Object* obj) const {
        const auto end_ = _pathmap.end();
        for (auto iter = _pathmap.begin(); iter != end_; ++iter) {
            if (iter->second.obj == obj) {
                return &iter->second;
            }
        }
        return NULL;
    }

    const PBXPath* Project::findGroupOrFilePath(const char* fullpath) const {
        const auto end_ = _pathmap.end();
        for (auto iter = _pathmap.begin(); iter != end_; ++iter) {
            if (strcasecmp(iter->second.pathFromSourceTree.c_str(), fullpath) == 0) {
                return &iter->second;
            }
        }
        return NULL;
    }


    bool Project::addItem(NeXTSTEP::KeyValue* kv) {
        assert(kv);
        assert(kv->value.isObject());
        if (!kv || !kv->value.isObject()) {
            return false;
        }
        NeXTSTEP::Object* obj = kv->value.object();
        const char* isa = obj->stringByKey("isa");
        if (!isa) {
            return false;
        }
        bool findSection = false;
        for (auto iterSection = _sections.begin(); iterSection != _sections.end(); ++iterSection) {
            if (iterSection->type.compare(isa) == 0) {
                for (auto i = iterSection->items.begin(); i != iterSection->items.end(); ++i) {
                    if ((*i)->key.compare(kv->key) >= 0) {
                        iterSection->items.insert(i, kv);
                        findSection = true;
                        break;
                    }
                }
                if (!findSection) {
                    iterSection->items.push_back(kv);
                    findSection = true;
                }
            }
        }
        if (findSection) {
            std::string sectionComment = kBegin + isa + kSection;
            for (auto iter = _objects->begin(); iter != _objects->end(); ++iter) {
                if (!(*iter)->key.empty()) {
                    continue;
                }
                if (!(*iter)->value.isString()) {
                    continue;
                }
                if (strcmp((*iter)->value.string(), sectionComment.c_str()) != 0) {
                    continue;
                }
                for (auto iter2 = iter + 1; iter2 != _objects->end(); ++iter2) {
                    if ((*iter2)->key.empty()) {
                        _objects->insert(iter2, kv);
                        break;
                    }
                    if ((*iter2)->key.compare(kv->key) >= 0) {
                        _objects->insert(iter2, kv);
                        break;
                    }
                }
                break;
            }
        }
        return findSection;
    }

    void Project::removeItem(const char* key) {
        if (!key || !key[0]) {
            return;
        }
        auto iter = _objects->begin();
        for (; iter != _objects->end(); ++iter) {
            if ((*iter)->key.compare(key) == 0) {
                break;
            }
        }
        if (iter == _objects->end()) {
            return;
        }
        if (!(*iter)->value.isObject()) {
            return;
        }
        NeXTSTEP::Object* obj = (*iter)->value.object();
        const char* isa = obj->stringByKey("isa");
        if (!isa) {
            return;
        }
        for (auto iterSection = _sections.begin(); iterSection != _sections.end(); ++iterSection) {
            if (iterSection->type.compare(isa) == 0) {
                for (auto i = iterSection->items.begin(); i != iterSection->items.end(); ++i) {
                    if ((*i)->key.compare(key) == 0) {
                        iterSection->items.erase(i);
                        break;
                    }
                }
            }
        }
        if (iter != _objects->end()) {
            delete *iter;
            _objects->erase(iter);
        }
    }

    void Project::write(shared::StrBuf& buf) const {
#if 0
        _plist.write(0, buf);
#else
        assert(_plist.isObject());
        if (_plist.isObject()) {
            buf.append("// !$*UTF8*$!\n");
            buf.append("{\n");
            const NeXTSTEP::Object& obj = *_plist.object();
            for (auto iter = obj.begin(); iter != obj.end(); ++iter) {
                if ((*iter)->key.compare("objects") == 0) {
                    buf.append("\tobjects = {\n");
                    writeObjects(buf);
                    buf.append("\t};\n");
                } else {
                    (*iter)->write(1, buf);
                    buf.append(";\n");
                }
            }
            buf.append("}\n");
        }
#endif
    }

    void Project::writeObjects(shared::StrBuf& buf) const {
        for (auto iter = _sections.begin(); iter != _sections.end(); ++iter) {
            buf.append('\n');
            buf.appendf("/* Begin %s section */\n", iter->type.c_str());

            int32_t indent = sectionBreakline(iter->type.c_str()) ? 2 : -1;
            for (auto i2 = iter->items.begin(); i2 != iter->items.end(); ++i2) {
#if 0
                buf.appendf("\t\t%s = ", i2->key().c_str());
                (*i2)->value.write(indent, buf);
#else
                if (indent < 0) {
                    buf.appendf("\t\t");
                }
                (*i2)->write(indent, buf);
#endif
                buf.append(";\n");
            }
            buf.appendf("/* End %s section */\n", iter->type.c_str());
        }
    }

    struct DumpContext {
        const NeXTSTEP::Object* objects;
        shared::StrBuf* buf;
        const PBXPath_map* path_map;
#if PROJ_WarnUnvisitNode
        std::unordered_set<const char*, hash_c_str, equal_to_c_str> visited;
#endif//PROJ_WarnUnvisitNode
        std::unordered_map<std::string, uint32_t> visitTimes;

        static const bool dumpVisitTimes = false;

        void dumpArray(int32_t indent, const NeXTSTEP::Array* arr) {
            if (arr) {
                for (auto iter = arr->begin(); iter != arr->end(); ++iter) {
                    if (iter->isString()) {
                        dump(indent, iter->string());
                    }
                }
            }
        }

        void _indent(int32_t indent) {
            for (int32_t i = 0; i < indent; ++i) {
                buf->append('\t');
            }
        }

        void dump(int32_t indent, const char* key) {
            if (!key) {
                return;
            }
            if (dumpVisitTimes) {
                uint32_t times = ++visitTimes[key];
                if (times > 1) {
                    printf("!!!!!Visit times %d\n", times);
                }
            }
            visited.insert(key);
            const NeXTSTEP::Object* obj = objects->objectByKey(key);
            if (!obj) {
                return;
            }
            const char* isa = obj->stringByKey("isa");
            if (!isa) {
                return;
            }
            _indent(indent);
            buf->appendf("%s = %s\n", isa, key);
            if (strcmp(isa, "PBXProject") == 0) {
                buf->append("==================== mainGroup ====================\n");
                dump(indent + 1, obj->stringByKey("mainGroup"));

                buf->append("==================== buildConfigurationList ====================\n");
                dump(indent + 1, obj->stringByKey("buildConfigurationList"));

                buf->append("==================== targets ====================\n");
                dumpArray(indent + 1, obj->arrayByKey("targets"));

            } else if (strcmp(isa, "PBXGroup") == 0) {
                _indent(indent + 1);
                auto iter = path_map->find(key);
                if (iter != path_map->end()) {
                    iter->second.debugDump(*buf);
                } else {
                    assert(false);
                    buf->appendf("Error not find path for %s!!!!!\n", obj->stringByKey("path"));
                }
                dumpArray(indent + 1, obj->arrayByKey("children"));

            } else if (strcmp(isa, "PBXFileReference") == 0) {
                _indent(indent + 1);
                auto iter = path_map->find(key);
                if (iter != path_map->end()) {
                    iter->second.debugDump(*buf);
                } else {
                    assert(false);
                    buf->appendf("Error not find path for %s!!!!!\n", obj->stringByKey("path"));
                }

            } else if (strcmp(isa, "PBXNativeTarget") == 0) {
                dump(indent + 1, obj->stringByKey("buildConfigurationList"));
                dumpArray(indent + 1, obj->arrayByKey("buildPhases"));
                dumpArray(indent + 1, obj->arrayByKey("dependencies"));

            } else if (strcmp(isa, "PBXSourcesBuildPhase") == 0) {
                dumpArray(indent + 1, obj->arrayByKey("files"));

            } else if (strcmp(isa, "PBXFrameworksBuildPhase") == 0) {
                dumpArray(indent + 1, obj->arrayByKey("files"));

            } else if (strcmp(isa, "PBXHeadersBuildPhase") == 0) {
                dumpArray(indent + 1, obj->arrayByKey("files"));

            } else if (strcmp(isa, "PBXCopyFilesBuildPhase") == 0) {
                dumpArray(indent + 1, obj->arrayByKey("files"));

            } else if (strcmp(isa, "PBXResourcesBuildPhase") == 0) {
                dumpArray(indent + 1, obj->arrayByKey("files"));

            } else if (strcmp(isa, "PBXShellScriptBuildPhase") == 0) {
                dumpArray(indent + 1, obj->arrayByKey("files"));

            } else if (strcmp(isa, "PBXBuildFile") == 0) {
                dump(indent + 1, obj->stringByKey("fileRef"));

            } else if (strcmp(isa, "PBXTargetDependency") == 0) {
                dump(indent + 1, obj->stringByKey("targetProxy"));

            } else if (strcmp(isa, "PBXReferenceProxy") == 0) {
                dump(indent + 1, obj->stringByKey("remoteRef"));

            } else if (strcmp(isa, "XCConfigurationList") == 0) {
                dumpArray(indent + 1, obj->arrayByKey("buildConfigurations"));

            } else if (strcmp(isa, "XCBuildConfiguration") == 0) {

            } else if (strcmp(isa, "PBXContainerItemProxy") == 0) {

            } else {
                _indent(indent + 1);
                buf->appendf("!!!!Error: Unknown %s type\n", isa);
            }
        }

        void dumpUnvisit() const {
            bool find = false;
            for (auto iter = objects->begin(); iter != objects->end(); ++iter) {
                if (!(*iter)->key.empty() && visited.find((*iter)->key.c_str()) == visited.end()) {
                    if (!find) {
                        buf->append("==================== unvisited ====================\n");
                        find = true;
                    }
                    const char* isa = (*iter)->value.stringByKey("isa");
                    buf->appendf("%s: %s\n", isa, (*iter)->key.c_str());
                }
            }
        }
    };

    void Project::debugDump(shared::StrBuf& buf) const {
        DumpContext context;
        context.objects = _plist.objectByKey("objects");
        context.buf = &buf;
        context.path_map = &_pathmap;
        if (context.objects) {
            context.dump(0, _plist.stringByKey("rootObject"));
            context.dumpUnvisit();
        }
    }

    const char* FileTypeForFile(const char* name) {
        static const struct {
            const char* ext;
            const char* fileType;
        } kFileTypeForExt[] = {
            {"h",    "sourcecode.c.h"},
            {"pch",  "sourcecode.c.h"},
            {"c",    "sourcecode.c.c"},
            {"m",    "sourcecode.c.objc"},
            {"hpp",  "sourcecode.cpp.h"},
            {"cpp",  "sourcecode.cpp.cpp"},
            {"mm",   "sourcecode.cpp.objcpp"},
            {"metal","sourcecode.metal"},
            {"swift","sourcecode.swift"},
            {"js",   "sourcecode.javascript"},
        };

        const char* ext = fileext(name, ".") + 1;
        for (size_t i = 0; i < _countof(kFileTypeForExt); ++i) {
            const auto& item = kFileTypeForExt[i];
            if (strcmp(item.ext, ext) == 0) {
                return item.fileType;
            }
        }
        return kFileTypeForExt[0].fileType;
    }
}
