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

#ifndef NeXTSTEP_plist_hpp
#define NeXTSTEP_plist_hpp
#include <string>
#include <vector>
#include <utility>
#include <shared/utils/StrBuf.h>
#include <assert.h>

// https://en.wikipedia.org/wiki/Property_list
// http://www.monobjc.net/xcode-project-file-format.html
// http://danwright.info/blog/2010/10/xcode-pbxproject-files/
// http://www.cnblogs.com/fengmin/p/5944474.html
// https://www.jianshu.com/p/e82ec6a56fc2

namespace NeXTSTEP {
    struct TokenType {
        enum Enum {
            None,
            Comment, // single comment /* Begin xxxxx section */
            ValuePrompt, // =
            Token, // string, with comment for key/value
            ArrayBegin, // (
            ArrayEnd, // )
            ArraySeparator, // ,
            ObjectBegin, // {
            ObjectEnd, // }
            ObjectSeparator, // ;
        };
        static const char* ToName(TokenType::Enum t) {
            const char* _enums[] = {
                "None",
                "Comment", // single comment /* Begin xxxxx section */
                "ValuePrompt", // =
                "Token", // string, with comment for key/value
                "ArrayBegin", // (
                "ArrayEnd", // )
                "ArraySeparator", // ,
                "ObjectBegin", // {
                "ObjectEnd", // }
                "ObjectSeparator", // ;
            };
            return _enums[t];
        }
    };

    struct Token {
        TokenType::Enum type;
        char* begin;
        size_t line;
        size_t column;

        void dump() {
            printf("%s: %d(%d): |%s|\n", TokenType::ToName(type), (int)line, (int)column, begin);
        }
    };

    struct InlineTokenParser {
        explicit InlineTokenParser(char* data);
        void parseNext(Token& token);
        bool expectToken(TokenType::Enum type);

    private:
        bool fillNextFromCurrent();
        void expectSpace(const char* p);
        void nextLine();

    private:
        char* _data;
        char* _current;
        char* _lineBegin;
        size_t _line;
        Token _cachedNextToken;
    };

    enum class ValueType {
        None,
        String,
        SharedString,
        Array,
        Object,
    };

    struct Array;
    struct Object;

    struct Value {
        ~Value();

        Value(Value&& other);
        void operator=(Value&& other);

        void clear();

        ValueType type() const {
            return _vt;
        }
        bool isString() const {
            return _vt == ValueType::String || _vt == ValueType::SharedString;
        }
        bool isArray() const {
            return _vt == ValueType::Array;
        }
        bool isObject() const {
            return _vt == ValueType::Object;
        }

        const char* string() const {
            assert(isString());
            return _value._string;
        }
        Array* array() const {
            assert(isArray());
            return _value._array;
        }
        Object* object() const {
            assert(isObject());
            return _value._object;
        }

        const Value* valueByKey(const char* key) const;
        const char* stringByKey(const char* key) const;
        Array* arrayByKey(const char* key) const;
        Object* objectByKey(const char* key) const;

        static Value NewSharedString(const char* string);
        static Value NewString(const char* string);
        static Value NewArray(Array* array);
        static Value NewObject(Object* object);

        bool parse(InlineTokenParser& parser);
        void write(int32_t indent, shared::StrBuf& buf) const;

    protected:
        ValueType _vt;
        union {
            const char* _string;
            Array* _array;
            Object* _object;
        } _value;

    protected:
        Value() : _vt(ValueType::None) {}
        Value(const Value& other) = delete;
        Value& operator =(const Value& other) = delete;
        friend struct KeyValue;
    };

    struct KeyValue {
        std::string key;
        Value value;

        KeyValue() {
        }

        KeyValue(const char* key_, Value&& value_) : key(key_), value(std::move(value_)) {
        }

        KeyValue(const char* key_, Object* value_) : key(key_), value(Value::NewObject(value_)) {
        }

        bool isComment() const {
            return key.empty();
        }

        KeyValue(KeyValue&& other) {
            swap(other);
        }
        void operator=(KeyValue&& other) {
            swap(other);
        }

        void swap(KeyValue& other) {
            std::swap(key, other.key);
            std::swap(value, other.value);
        }
        void write(int32_t indent, shared::StrBuf& buf) const;

    private:
        KeyValue(const KeyValue& other) = delete;
        KeyValue& operator =(const KeyValue& other) = delete;
    };

    inline void swap(KeyValue& __lhs, KeyValue& __rhs) {
        __lhs.swap(__rhs);
    }

    struct Array : std::vector<Value> {
    public:
        bool parse(InlineTokenParser& parser);
        void write(int32_t indent, shared::StrBuf& buf) const;
    };

    struct Object : std::vector<KeyValue*> {
    public:
        ~Object();
        bool parse(InlineTokenParser& parser);
        void write(int32_t indent, shared::StrBuf& buf) const;
        const Value* valueByKey(const char* key) const;
        const char* stringByKey(const char* key) const;
        Array* arrayByKey(const char* key) const;
        Object* objectByKey(const char* key) const;
        bool set(const char* key, Value&& value);
        bool set(const char* key, const char* string);
        bool set(const char* key, Array* array);
        bool set(const char* key, Object* object);
        bool remove(const char* key);
    };

    class PList : public Value {
    public:
        PList() {}

        bool inlineParse(char* data);

    protected:
        char* _data;
    };
}

#endif//NeXTSTEP_plist_hpp
