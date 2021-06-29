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

#ifndef pbxproj_parser_hpp
#define pbxproj_parser_hpp
#include <vector>
#include <string>
#include <stdint.h>
#include "NeXTSTEP_plist.hpp"
#include <shared/utils/StrBuf.h>
#include <unordered_map>
#include "namehash.h"

namespace pbxproj {
    struct SId {
        uint32_t _id[3];
        void fromString(const char* str) {
            const char* sid = "0123456789ABCDEF";
            uint32_t id2 = 0;
            for (size_t i = 0; i < 24; ++i) {
                const char* pcur = strchr(sid, str[i]);
                uint32_t index = pcur ? ((uint32_t)(pcur - sid)) : 0;
                id2 <<= 4;
                id2 |= index;
                if ((i % 8) == 7) {
                    _id[i / 8] = id2;
                    id2 = 0;
                }
            }
        }
        void toString(char* out) const {
            sprintf(out, "%08X%08X%08X", _id[0], _id[1], _id[2]);
        }
        std::string toString() const {
            char ch[32];
            toString(ch);
            return ch;
        }
        void print() const {
            char ch[32];
            toString(ch);
            printf("id:%s\n", ch);
        }
    };

    struct STag {
        SId id;
        std::string comment;
    };

    struct Section {
        std::string type;
        std::vector<const NeXTSTEP::KeyValue*> items;

        Section() {
        }

        Section(Section&& other) {
            swap(other);
        }
        void operator=(Section&& other) {
            swap(other);
        }

        void swap(Section& other) {
            std::swap(type, other.type);
            std::swap(items, other.items);
        }

        void clear() {
            type.clear();
            items.clear();
        }

    private:
        Section(const Section& other) = delete;
        Section& operator=(const Section& other) = delete;
    };

    inline void swap(Section& __lhs, Section& __rhs) {
        __lhs.swap(__rhs);
    }

    struct SourceTreeType {
        enum Enum {
            Unknown,
            Absolute, // "<absolute>"
            Group, // "<group>"
            Project, // SOURCE_ROOT
            Developer, // DEVELOPER_DIR
            Sdk, // SDKROOT
            Products, // BUILT_PRODUCTS_DIR
        };
        static Enum FromString(const char* src);
        static const char* ToString(Enum src);
        static const char* ToName(Enum src);
    };

    struct PBXPath {
        std::string pathFromSourceTree;
        const NeXTSTEP::Object* obj;
        const char* key;
        const char* path;
        const char* parentKey;
        SourceTreeType::Enum sourceTree;
        bool isFile;

        PBXPath(const NeXTSTEP::Object* o, const char* key, const PBXPath* parent, bool file);

        PBXPath(const PBXPath& other) : pathFromSourceTree(other.pathFromSourceTree) {
            obj = other.obj;
            key = other.key;
            path = other.path;
            parentKey = other.parentKey;
            sourceTree = other.sourceTree;
            isFile = other.isFile;
        }

        PBXPath(PBXPath&& other) {
            swap(other);
        }
        void operator=(PBXPath&& other) {
            swap(other);
        }

        void swap(PBXPath& other) {
            std::swap(pathFromSourceTree, other.pathFromSourceTree);
            std::swap(obj, other.obj);
            std::swap(key, other.key);
            std::swap(path, other.path);
            std::swap(parentKey, other.parentKey);
            std::swap(sourceTree, other.sourceTree);
            std::swap(isFile, other.isFile);
        }

        void debugDump(shared::StrBuf& buf) const;

        static std::string unescape(std::string path);
        static std::string escape(std::string path);

    private:
        PBXPath& operator=(const PBXPath& other) = delete;
    };

    struct hash_c_str {
        size_t operator()(const char* str) const noexcept {
            return HashString(str, 131);
        }
    };

    struct equal_to_c_str {
        bool operator()(const char* lhs, const char* rhs) const noexcept {
            return strcmp(lhs, rhs) == 0;
        }
    };

    typedef std::unordered_map<const char*, PBXPath, hash_c_str, equal_to_c_str> PBXPath_map;

    struct ProjectItem {
        explicit ProjectItem(NeXTSTEP::Object* obj) : _obj(obj) {
        }
        bool isa(const char* t) const {
            if (_obj) {
                const char* isa_ = _obj->stringByKey("isa");
                return isa_ && strcmp(isa_, t) == 0;
            }
            return false;
        }
        NeXTSTEP::Object* operator->() {
            return _obj;
        }
        operator NeXTSTEP::Object*() const {
            return _obj;
        }

    protected:
        NeXTSTEP::Object* _obj;
    };


    class Project {
    public:
        Project();

        bool inlineParse(char* src);
        void write(shared::StrBuf& buf) const;

        size_t targetCount() const {
            return _targets.size();
        }
        NeXTSTEP::Object* target(size_t index) {
            return _targets[index];
        }

        bool valid() const {
            return _project && _objects;
        }

    public:
        void debugDump(shared::StrBuf& buf) const;

    protected:
        void writeObjects(shared::StrBuf& buf) const;
        const PBXPath* findGroupOrFilePath(NeXTSTEP::Object* obj) const;
        const PBXPath* findGroupOrFilePath(const char* fullpath) const;
        bool addItem(NeXTSTEP::KeyValue* kv);
        void removeItem(const char* key);

    protected:
        NeXTSTEP::PList _plist;

        // objects
        std::vector<Section> _sections;

        NeXTSTEP::Object* _project;
        NeXTSTEP::Object* _objects;
        std::vector<NeXTSTEP::Object*> _targets;

        PBXPath_map _pathmap;
    };


    const char* FileTypeForFile(const char* name);
}

#endif//pbxproj_parser_hpp
