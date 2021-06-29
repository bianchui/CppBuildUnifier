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

#ifndef shared_Path_h__
#define shared_Path_h__
#include <string>
#include <assert.h>

namespace shared {

struct Path {
private:
    std::string _buffer;
    
public:
    explicit Path(const char* path) {
        set(path);
    }
    Path(const char* dir, const char* relative) {
        set(dir, relative);
    }
#if 0
    Path(const std::string& dir, const char* relative) {
        set(dir.c_str(), relative);
    }
    Path(const std::string& dir, const std::string& relative) {
        set(dir.c_str(), relative.c_str());
    }
    Path(const Path& dir, const char* relative) {
        set(dir.c_str(), relative);
    }
    Path(const Path& dir, const std::string& relative) {
        set(dir.c_str(), relative.c_str());
    }
#endif//
    void set(const char* path) {
        _buffer = path;
        _buffer.resize(normalize((char*)_buffer.c_str()));
    }
    void set(const char* dir, const char* relative) {
        if (is_absolute(relative)) {
            _buffer = relative;
        } else {
            _buffer = dir;
            if (_buffer.length() && _buffer.back() != '/') {
                _buffer.push_back('/');
            }
            _buffer.append(relative);
        }
        _buffer.resize(normalize((char*)_buffer.c_str()));
    }
    const char* c_str() const { return _buffer.c_str(); }
    const std::string& string() const { return _buffer; }

    std::string dir() const {
        const char* name = c_str();
        const char* sp = strrchr(name, '/');
        return sp ? std::string(name, sp - name) : "";
    }

    const char* name() const {
        const char* name = c_str();
        const char* sp = strrchr(name, '/');
        return sp ? sp + 1 : (name + strlen(name));
    }

    const char* ext() const {
        const char* name = c_str();
        const char* pt = strrchr(name, '.');
        const char* sp = strrchr(name, '/');
        return (pt && sp < pt) ? pt : (name + strlen(name));
    }
                       
    static bool is_absolute(const char* path) {
        return path && *path == '/';
    }
    
    static size_t normalize(char* path) {
        const char* begin = path;
        if (!path) {
            return 0;
        }
        const bool absolute = *path == '/';
        if (absolute) {
            ++path;
        }
        char* cur = path;
        const char* pathEnd = path + strlen(path);
        while (true) {
            char* sp = (char*)strchr(cur, '/');
            if (sp == cur) {
                // abc//def -> abc/def
                assert(cur + 1 + (pathEnd - cur) == pathEnd + 1);
                memmove(cur, cur + 1, pathEnd - cur);
                pathEnd -= 1;
                assert(path + strlen(path) == pathEnd);
                continue;
            }
            if (sp) {
                *sp = 0;
            }
            if (cur[0] == '.') {
                if (cur[1] == 0) {
                    if (sp) {
                        // abc/./def -> abc/def
                        assert(cur + 2 + (pathEnd - cur - 1) == pathEnd + 1);
                        memmove(cur, cur + 2, pathEnd - cur - 1);
                        pathEnd -= 2;
                        assert(path + strlen(path) == pathEnd);
                        continue;
                    } else {
                        // abc/. -> abc/
                        assert(cur + 1 == pathEnd);
                        *cur = 0;
                        pathEnd = cur;
                        break;
                    }
                } else if (cur[1] == '.' && cur[2] == 0) {
                    // abc/def/../ghi -> abc/ghi
                    char* pre = cur;
                    if (pre != path) {
                        --pre;
                        while (pre != path) {
                            if (*(--pre) == '/') {
                                ++pre;
                                break;
                            }
                        }
                    }
                    if (sp) {
                        if (absolute || cur != path) {
                            assert(cur + 3 + (pathEnd - cur - 2) == pathEnd + 1);
                            memmove(pre, cur + 3, pathEnd - cur - 2);
                            pathEnd -= cur + 3 - pre;
                            cur = pre;
                            assert(path + strlen(path) == pathEnd);
                        } else {
                            *sp = '/';
                            cur = path = sp + 1;
                        }
                        continue;
                    } else {
                        if (absolute || cur != path) {
                            assert(cur + 2 == pathEnd);
                            *pre = 0;
                            pathEnd = pre;
                        }
                        break;
                    }

                }
            }
            if (!sp) {
                assert(cur + strlen(cur) == pathEnd);
                break;
            }
            *sp = '/';
            cur = sp + 1;
        }
        assert(begin + strlen(begin) == pathEnd);
        return pathEnd - begin;
    }
    
private:
    Path(const Path&) = delete;
    Path& operator=(const Path&) = delete;
};

}

#endif//shared_Path_h__
