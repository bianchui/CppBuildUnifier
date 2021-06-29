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

#include "NeXTSTEP_plist.hpp"

namespace NeXTSTEP {
    struct SingleToken {
        char ch;
        TokenType::Enum type;
    };
    static const SingleToken kSingleTokens[] = {
        {'=',  TokenType::ValuePrompt},

        {'(',  TokenType::ArrayBegin},
        {')',  TokenType::ArrayEnd},
        {',',  TokenType::ArraySeparator},

        {'{',  TokenType::ObjectBegin},
        {'}',  TokenType::ObjectEnd},
        {';',  TokenType::ObjectSeparator},
    };
    static const size_t kSingleTokenCount = sizeof(kSingleTokens) / sizeof(SingleToken);

    InlineTokenParser::InlineTokenParser(char* data) : _data(data), _current(data), _lineBegin(data), _line(0) {
        _cachedNextToken.type = TokenType::None;
    }

    bool InlineTokenParser::fillNextFromCurrent() {
        if (_cachedNextToken.type != TokenType::None) {
            fprintf(stderr, "!!!!!Error expect None cached token.");
            exit(1);
        }
        for (size_t i = 0; i < kSingleTokenCount; ++i) {
            if (_current[0] == kSingleTokens[i].ch) {
                *_current = 0;
                _cachedNextToken.type = kSingleTokens[i].type;
                _cachedNextToken.begin = _current;
                _cachedNextToken.line = _line + 1;
                _cachedNextToken.column = _current - _lineBegin + 1;
                ++_current;
                return true;
            }
        }
        expectSpace(_current);
        return false;
    }

    void InlineTokenParser::expectSpace(const char* p) {
        const char ch = p[0];
        if (ch && ch != '\n' && ch != ' ' && ch != '\t') {
            fprintf(stderr, "!!!!!Error expect space.");
            exit(1);
        }
    }

    void InlineTokenParser::nextLine() {
        const char ch = *_current;
        if (ch != '\n') {
            fprintf(stderr, "!!!!!Error expect end line.");
            exit(1);
        }
        ++_line;
        _lineBegin = _current + 1;
    }

    void InlineTokenParser::parseNext(Token& token) {
        if (_cachedNextToken.type != TokenType::None) {
            memcpy(&token, &_cachedNextToken, sizeof(Token));
            _cachedNextToken.type = TokenType::None;
            return;
        }

        while (true) {
            char ch = *_current;
            if (ch == ' ' || ch == '\t' || ch == '\r') {

            } else if (ch == '\n') {
                nextLine();
            } else {
                break;
            }
            ++_current;
        }

        token.begin = _current;
        token.line = _line + 1;
        token.column = _current - _lineBegin + 1;
        if (!*_current) {
            token.type = TokenType::None;
            return;
        }

        for (size_t i = 0; i < kSingleTokenCount; ++i) {
            if (_current[0] == kSingleTokens[i].ch) {
                *_current = 0;
                ++_current;
                token.type = kSingleTokens[i].type;
                return;
            }
        }

        if (_current[0] == '/') {
            if (_current[1] == '*') {
                token.type = TokenType::Comment;
                _current += 2;
                while (true) {
                    char ch = *_current;
                    if (ch == 0) {
                        break;
                    } else if (ch == '\n') {
                        nextLine();
                        ++_current;
                    } else if (ch == '*') {
                        if (_current[1] == '/') {
                            _current += 2;
                            if (!fillNextFromCurrent()) {
                                if (*_current == '\n') {
                                    nextLine();
                                }
                                *_current = 0;
                                ++_current;
                            }
                            break;
                        }
                    }
                    ++_current;
                }
                return;
            } else if (_current[1] == '/') {
                token.type = TokenType::Comment;
                _current += 2;
                while (true) {
                    char ch = *_current;
                    if (ch == 0) {
                        break;
                    } else if (ch == '\n') {
                        nextLine();
                        *_current = 0;
                        ++_current;
                        break;
                    }
                    ++_current;
                }
                return;
            }
        }

        token.type = TokenType::Token;
        bool inString = false;
        bool escape = false;
        bool inComment = false;
        char *lastKnownTokenEnd = _current;
        while (true) {
            char ch = *_current;
            if (ch == 0) {
                break;
            }
            if (inComment) {
                if (ch == '*' && _current[1] == '/') {
                    ++_current;
                    inComment = false;
                }
            } else if (inString) {
                if (ch == '\n') {
                    nextLine();
                }
                if (escape) {
                    escape = false;
                } else {
                    if (ch == '"') {
                        inString = false;
                    } else if (ch == '\\') {
                        escape = true;
                    }
                }
            } else {
                if (ch == '"') {
                    inString = true;
                } else if (ch == ';' || ch == ',' || ch == '=') {
                    if (!fillNextFromCurrent()) {
                        exit(1);
                    }
                    break;
                } else if (ch == '\n') {
                    nextLine();
                } else if (ch == '/' && _current[1] == '*') {
                    ++_current;
                    inComment = true;
                }
            }
            ++_current;
            if (inComment || inString || (ch != ' ' && ch != '\t')) {
                lastKnownTokenEnd = _current;
            }
        }
        expectSpace(lastKnownTokenEnd);
        *lastKnownTokenEnd = 0;
    }

    bool InlineTokenParser::expectToken(TokenType::Enum type) {
        Token token;
        parseNext(token);
        if (token.type == type) {
            return true;
        }
        printf("!!!!Expect %s but get :", TokenType::ToName(type));
        token.dump();
        return false;
    }


    Value::~Value() {
        clear();
    }

    void Value::clear() {
        switch (_vt) {
            case ValueType::String:
                free((char*)_value._string);
                break;

            case ValueType::Array:
                delete _value._array;
                break;

            case ValueType::Object:
                delete _value._object;
                break;

            default:
                break;
        }
        _value._string = NULL;
    }

    Value::Value(Value&& other) {
        memcpy(this, &other, sizeof(Value));
        other._vt = ValueType::None;
    }

    void Value::operator=(Value&& other) {
        std::swap(_vt, other._vt);
        std::swap(_value._object, other._value._object);
    }

    const Value* Value::valueByKey(const char* key) const {
        if (isObject()) {
            return object()->valueByKey(key);
        } else {
            return NULL;
        }
    }

    const char* Value::stringByKey(const char* key) const {
        auto fileRef_ = valueByKey(key);
        if (fileRef_ && fileRef_->isString()) {
            return fileRef_->string();
        } else {
            return NULL;
        }
    }

    Array* Value::arrayByKey(const char* key) const {
        auto fileRef_ = valueByKey(key);
        if (fileRef_ && fileRef_->isArray()) {
            return fileRef_->array();
        } else {
            return NULL;
        }
    }

    Object* Value::objectByKey(const char* key) const {
        auto fileRef_ = valueByKey(key);
        if (fileRef_ && fileRef_->isObject()) {
            return fileRef_->object();
        } else {
            return NULL;
        }
    }


    Value Value::NewSharedString(const char* string) {
        Value v;
        v._vt = ValueType::SharedString;
        v._value._string = string;
        return v;
    }

    Value Value::NewString(const char* string) {
        Value v;
        v._vt = ValueType::String;
        v._value._string = strdup(string);
        return v;
    }

    Value Value::NewArray(Array* array) {
        Value v;
        v._vt = ValueType::Array;
        if (!array) {
            array = new Array();
        }
        v._value._array = array;
        return v;
    }

    Value Value::NewObject(Object* object) {
        Value v;
        v._vt = ValueType::Object;
        if (!object) {
            object = new Object();
        }
        v._value._object = object;
        return v;
    }

    bool Value::parse(InlineTokenParser& parser) {
        Token token;

        while (true) {
            parser.parseNext(token);
            switch (token.type) {
                case TokenType::Token: {
                    _vt = ValueType::SharedString;
                    _value._string = token.begin;
                    return true;
                }
                case TokenType::ObjectBegin: {
                    _vt = ValueType::Object;
                    _value._object = new Object();
                    return _value._object->parse(parser);
                }
                case TokenType::ArrayBegin: {
                    _vt = ValueType::Array;
                    _value._array = new Array();
                    return _value._array->parse(parser);
                }
                default: {
                    printf("!!!Error unexpected token: ");
                    token.dump();
                    return false;
                }
            }
        }
    }

    void Value::write(int32_t indent, shared::StrBuf& buf) const {
        switch (_vt) {
            case ValueType::String:
            case ValueType::SharedString:
                buf.appendf("%s", _value._string);
                break;

            case ValueType::Array:
                _value._array->write(indent, buf);
                break;

            case ValueType::Object:
                _value._object->write(indent, buf);
                break;

            default:
                break;
        }
    }

    void KeyValue::write(int32_t indent, shared::StrBuf& buf) const {
        if (isComment()) {
            value.write(0, buf);
        } else {
            for (int32_t i = 0; i < indent; ++i) {
                buf.append('\t');
            }
            buf.appendf("%s = ", key.c_str());
            value.write(indent, buf);
        }
    }

    bool Array::parse(InlineTokenParser& parser) {
        Token token;
        while (true) {
            parser.parseNext(token);
            switch (token.type) {
                case TokenType::ArrayEnd: {
                    return true;
                }
                case TokenType::Token: {
                    push_back(Value::NewSharedString(token.begin));
                    break;
                }
                case TokenType::ObjectBegin: {
                    Object* object = new Object();
                    push_back(Value::NewObject(object));
                    if (!object->parse(parser)) {
                        return false;
                    }
                    break;
                }
                case TokenType::ArrayBegin: {
                    Array* array = new Array();
                    push_back(Value::NewArray(array));
                    if (!array->parse(parser)) {
                        return false;
                    }
                    break;
                }
                default: {
                    printf("!!!Error unexpected token: ");
                    token.dump();
                    return false;
                }
            }
            if (!parser.expectToken(TokenType::ArraySeparator)) {
                return false;
            }
        }
    }

    void Array::write(int32_t indent, shared::StrBuf& buf) const {
        buf.append('(');
        if (indent >= 0) {
            buf.append('\n');
        }
        int32_t childIndent = indent < 0 ? indent : indent + 1;
        for (auto iter = begin(); iter != end(); ++iter) {
            for (int32_t i = 0; i < childIndent; ++i) {
                buf.append('\t');
            }
            iter->write(childIndent, buf);
            if (childIndent < 0) {
                buf.append(", ");
            } else {
                buf.append(",\n");
            }
        }
        for (int32_t i = 0; i < indent; ++i) {
            buf.append('\t');
        }
        buf.append(')');
    }

    Object::~Object() {
        for (auto iter = begin(); iter != end(); ++iter) {
            delete *iter;
        }
    }

    bool Object::parse(InlineTokenParser& parser) {
        Token token;
        while (true) {
            parser.parseNext(token);
            switch (token.type) {
                case TokenType::ObjectEnd: {
                    return true;
                }
                case TokenType::Token: {
                    KeyValue* kv = new KeyValue();
                    kv->key = token.begin;
                    if (!parser.expectToken(TokenType::ValuePrompt)) {
                        return false;
                    }
                    if (!kv->value.parse(parser)) {
                        return false;
                    }
                    if (!parser.expectToken(TokenType::ObjectSeparator)) {
                        return false;
                    }
                    push_back(kv);
                    break;
                }
                case TokenType::Comment: {
                    push_back(new KeyValue("", Value::NewSharedString(token.begin)));
                    break;
                }
                default: {
                    printf("!!!Error unexpected token: ");
                    token.dump();
                    return false;
                }
            }
        }
    }

    void Object::write(int32_t indent, shared::StrBuf& buf) const {
        buf.append('{');
        if (indent >= 0) {
            buf.append('\n');
        }
        int32_t childIndent = indent < 0 ? indent : indent + 1;
        for (auto iter = begin(); iter != end(); ++iter) {
            if ((*iter)->isComment()) {
                (*iter)->value.write(0, buf);
                if (childIndent >= 0) {
                    buf.append('\n');
                }
            } else {
                for (int32_t i = 0; i < childIndent; ++i) {
                    buf.append('\t');
                }
                buf.appendf("%s = ", (*iter)->key.c_str());
                (*iter)->value.write(childIndent, buf);
                if (childIndent < 0) {
                    buf.append("; ");
                } else {
                    buf.append(";\n");
                }
            }
        }
        for (int32_t i = 0; i < indent; ++i) {
            buf.append('\t');
        }
        buf.append('}');
    }

    const Value* Object::valueByKey(const char* key) const {
        if (!key || !key[0]) {
            return NULL;
        }
        for (auto iter = begin(); iter != end(); ++iter) {
            if (strcmp((*iter)->key.c_str(), key) == 0) {
                return &(*iter)->value;
            }
        }
        return NULL;
    }

    const char* Object::stringByKey(const char* key) const {
        auto fileRef_ = valueByKey(key);
        if (fileRef_ && fileRef_->isString()) {
            return fileRef_->string();
        } else {
            return NULL;
        }
    }

    Array* Object::arrayByKey(const char* key) const {
        auto fileRef_ = valueByKey(key);
        if (fileRef_ && fileRef_->isArray()) {
            return fileRef_->array();
        } else {
            return NULL;
        }
    }

    Object* Object::objectByKey(const char* key) const {
        auto fileRef_ = valueByKey(key);
        if (fileRef_ && fileRef_->isObject()) {
            return fileRef_->object();
        } else {
            return NULL;
        }
    }

    bool Object::set(const char* key, Value&& value) {
        auto fileRef_ = valueByKey(key);
        if (fileRef_) {
            return false;
        } else {
            push_back(new KeyValue(key, std::move(value)));
            return true;
        }
    }

    bool Object::set(const char* key, const char* string) {
        return set(key, Value::NewString(string));
    }

    bool Object::set(const char* key, Array* array) {
        return set(key, Value::NewArray(array));
    }

    bool Object::set(const char* key, Object* object) {
        return set(key, Value::NewObject(object));
    }

    bool Object::remove(const char* key) {
        if (!key || !key[0]) {
            return false;
        }
        for (auto iter = begin(); iter != end(); ++iter) {
            if ((*iter)->key.compare(key) == 0) {
                erase(iter);
                return true;
            }
        }
        return false;
    }

    bool PList::inlineParse(char* data) {
        _data = data;
        clear();

        InlineTokenParser parser(data);
        if (!parser.expectToken(TokenType::Comment)) {
            return false;
        }
        if (!parse(parser)) {
            return false;
        }
        return parser.expectToken(TokenType::None);
    }
}
