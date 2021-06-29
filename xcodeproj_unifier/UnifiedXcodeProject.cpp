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

#include "UnifiedXcodeProject.hpp"
#include "xcodeproj/namehash.h"
#include <shared/utils/StrBuf.h>
#include "SXcodeSources.h"

std::string UnifiedXcodeProject::KeyForGroupOrFile(const char* path, const char* name) {
    pbxproj::SId id;
    id._id[0] = 0;
    id._id[1] = HashPath(path, 131);
    id._id[2] = HashPath(name, 131);
    char sid[32];
    id.toString(sid);
    shared::StrBuf buf;
    buf.printf("%s /* %s */", sid, name);
    return buf.string();
}

std::string UnifiedXcodeProject::KeyForBuildFile(const char* path, const char* file, const char* target) {
    pbxproj::SId id;
    id._id[0] = HashPath(target, 131);
    id._id[1] = HashPath(path, 131);
    id._id[2] = HashPath(file, 131);
    char sid[32];
    id.toString(sid);
    shared::StrBuf buf;
    buf.printf("%s /* %s in Sources */", sid, file);
    return buf.string();
}

NeXTSTEP::Object* UnifiedXcodeProject::targetGetUnifiedRoot(const char* target) {
    if (!valid()) {
        return NULL;
    }
    NeXTSTEP::Object* mainGroup = _objects->objectByKey(_project->stringByKey("mainGroup"));
    if (!mainGroup) {
        return NULL;
    }
    NeXTSTEP::Object* unified_root = getOrNewChildGroup(mainGroup, SXcodeSources::Unified_Path);
    if (!unified_root) {
        return NULL;
    }
    return getOrNewChildGroup(unified_root, target);
}

void UnifiedXcodeProject::getAllFiles(std::set<std::string>& files) {
    for (auto iter = _pathmap.begin(); iter != _pathmap.end(); ++iter) {
        const auto& info = iter->second;
        if (info.isFile) {
            if (info.sourceTree == pbxproj::SourceTreeType::Group || info.sourceTree == pbxproj::SourceTreeType::Project) {
                files.insert(info.pathFromSourceTree);
            }
        }
    }
}

NeXTSTEP::Array* UnifiedXcodeProject::targetGetBuildFiles(NeXTSTEP::Object* target) {
    auto buildPhases = target->arrayByKey("buildPhases");
    if (!buildPhases) {
        return NULL;
    }
    for (auto iter = buildPhases->begin(); iter != buildPhases->end(); ++iter) {
        pbxproj::ProjectItem phase(_objects->objectByKey(iter->string()));
        if (phase.isa("PBXSourcesBuildPhase")) {
            return phase->arrayByKey("files");
        }
    }
    return NULL;
}

NeXTSTEP::Object* UnifiedXcodeProject::getOrNewChildGroupOrFile(NeXTSTEP::Object* parent, const char* name_, bool file) {
    const char* isa = file ? "PBXFileReference" : "PBXGroup";
    assert(parent);
    NeXTSTEP::Array* parentChildren = parent->arrayByKey("children");
    if (!parentChildren) {
        return NULL;
    }
    std::string name = pbxproj::PBXPath::escape(name_);
    for (auto iter = parentChildren->begin(); iter != parentChildren->end(); ++iter) {
        pbxproj::ProjectItem obj(_objects->objectByKey(iter->string()));
        if (!obj.isa(isa)) {
            continue;
        }
        auto path = obj->stringByKey("path");
        if (path && strcasecmp(path, name.c_str()) == 0) {
            return obj;
        }
    }
    const pbxproj::PBXPath* parentPath = findGroupOrFilePath(parent);
    assert(parentPath);
    if (!parentPath) {
        return NULL;
    }

    NeXTSTEP::Object* obj = new NeXTSTEP::Object();
    obj->set("isa", isa);
    if (file) {
        obj->set("fileEncoding", "4");
        obj->set("lastKnownFileType", pbxproj::FileTypeForFile(name_));
    } else {
        obj->set("children", new NeXTSTEP::Array());
    }
    obj->set("path", name.c_str());
    obj->set("sourceTree", pbxproj::SourceTreeType::ToString(pbxproj::SourceTreeType::Group));

    std::string key = KeyForGroupOrFile(parentPath->pathFromSourceTree.c_str(), name_);

    {
        const char* comp_key = strchr(key.c_str(), '/');
        if (!comp_key) {
            comp_key = key.c_str();
        }
        bool added = false;
        for (auto iter = parentChildren->begin(); iter != parentChildren->end(); ++iter) {
            if (!iter->isString()) {
                continue;
            }
            const char* str = iter->string();
            const char* comp_str = strchr(str, '/');
            if (!comp_str) {
                comp_str = str;
            }
            if (strcasecmp(comp_str, comp_key) >= 0) {
                parentChildren->insert(iter, NeXTSTEP::Value::NewString(key.c_str()));
                added = true;
                break;
            }
        }
        if (!added) {
            parentChildren->push_back(NeXTSTEP::Value::NewString(key.c_str()));
        }
    }

    NeXTSTEP::KeyValue* kv = new NeXTSTEP::KeyValue(key.c_str(), obj);
    _pathmap.insert({kv->key.c_str(), pbxproj::PBXPath(obj, kv->key.c_str(), parentPath, file)});
    addItem(kv);

    assert(_objects->objectByKey(key.c_str()));

    return obj;

}
