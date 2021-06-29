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

#ifndef shared_utils_StrBuf_h__
#define shared_utils_StrBuf_h__
#include <stdio.h>
#include <string>
#include <stdlib.h>
#include <stdarg.h>
#include <assert.h>
#include <errno.h>

namespace shared {

template <typename CharT>
struct StrBufTraits;

template <>
struct StrBufTraits<char> {
    typedef char CharT;
    typedef wchar_t YCharT;
    typedef std::string StringT;

    static const char NilChar = '\0';

    static inline size_t strlenT(const char* Str) {
        return ::strlen(Str);
    }
    static inline int vsnprintfT(char* dst, size_t size, const char* format, va_list ap) {
        return ::vsnprintf(dst, size, format, ap);
    }
};

template <>
struct StrBufTraits<wchar_t> {
    typedef wchar_t CharT;
    typedef char YCharT;
    typedef std::wstring StringT;

    static const wchar_t NilChar = L'\0';

    static inline size_t strlenT(const wchar_t* Str) {
        return ::wcslen(Str);
    }

    static inline int vsnprintfT(wchar_t* dst, size_t size, const wchar_t* format, va_list ap) {
        return ::vswprintf(dst, size, format, ap);
    }
};

template <typename T>
class StrBufBase {
public:
	enum { kMaxDoubleCapacity = 1024 * 1024 / sizeof(T) };
    typedef T CharT;
    typedef StrBufTraits<T> TraitsT;
    typedef typename TraitsT::StringT StringT;
    typedef typename TraitsT::YCharT YCharT;

    StrBufBase(CharT* buf, size_t bufsize) : _sbuf(buf), _dbuf(buf) {
		buf[0] = 0;
		_capacity = bufsize;
		_length = 0;
	}
	~StrBufBase() {
		if (_dbuf != _sbuf) {
			free(_dbuf);
		}
	}
	void printf(const CharT* format, ...) {
		va_list ap;
		va_start(ap, format);
		vprintf(format, ap);
		va_end(ap);
	}
	void vprintf(const CharT* format, va_list ap) {
        _length = 0;
        vappendf(format, ap);
	}
	void appendf(const CharT* format, ...) {
		va_list ap;
		va_start(ap, format);
		vappendf(format, ap);
		va_end(ap);
	}
	void vappendf(const CharT* format, va_list ap) {
		while(1) {
			int n = (int)(_capacity - _length);
            va_list apcp;
            va_copy(apcp, ap);
            int final_n = TraitsT::vsnprintfT(_dbuf + _length, n, format, apcp);
            va_end(apcp);
			if (final_n < 0 || final_n >= n) {
				if (errno == EILSEQ) {
					_dbuf[_length] = 0;
					return;
				}
				int add = final_n - n + 1;
				if (add < 0) {
					add = -add;
				}
				enlargeToSize(_capacity + add);
			} else {
				_length += final_n;
				break;
			}
		}
	}
	void assign(const CharT* str, size_t len = -1) {
		if (len == (size_t)-1) {
			len = TraitsT::strlenT(str);
		}
		if (len) {
			if (len + 1 > _capacity) {
				_capacity = calcCapacity(len + 1);
				if (_dbuf != _sbuf) {
					free(_dbuf);
				}
				_dbuf = (CharT*)malloc(_capacity * sizeof(CharT));
			}
			memcpy(_dbuf, str, len * sizeof(CharT));
		}
		_dbuf[(_length = len)] = 0;
	}
    void assign(const StringT& str) {
        assign(str.c_str(), str.length());
    }
	void append(const CharT* str, size_t len = -1) {
		if (len == (size_t)-1) {
			len = strlen(str);
		}
		if (!len) {
			return;
		}
		size_t n = _capacity - _length;
		if (n < len + 1) {
			enlargeToSize(_capacity + len + 1);
		}
		memcpy(_dbuf + _length, str, len * sizeof(CharT));
		_dbuf[(_length += len)] = 0;
	}
    void append(CharT ch) {
        append(&ch, 1);
    }
    void append(const StringT& str) {
        append(str.c_str(), str.length());
    }
    void append(const StrBufBase& str) {
        append(str.c_str(), str.length());
    }
    void pop(size_t length = 1) {
        if (length < _length) {
            _length -= length;
        } else {
            _length = 0;
        }
        _dbuf[_length] = 0;
    }

	// CAUTION: when set larger than old size, buffer is not filled by zero by performance.
	void resize(size_t length) {
		if (length > _length) {
			if (length >= _capacity) {
				enlargeToSize(length + 1);
			}
		}
		_dbuf[(_length = length)] = 0;
	}
	
	// size is plus 1 to meet std::string::reserve
	void reserve(size_t size) {
		if (size >= _capacity) {
			enlargeToSize(size + 1);
		}
	}

	const CharT* c_str() const {
		return _dbuf;
	}
	const CharT* get() const {
		return _dbuf;
	}
	const CharT* data() const {
		return _dbuf;
	}
	CharT* buf() {
		return _dbuf;
	}
	operator const CharT* () const {
		return c_str();
	}
	size_t length() const {
		return _length;
	}
	size_t capacity() const {
		return _capacity - 1;
	}
	void clear() {
		_length = 0;
	}
    bool empty() const {
        return !_length;
    }
    StringT string() const {
        return StringT(_dbuf, _length);
    }

    bool same(const StringT& str) const {
        return _length == str.length() && memcmp(c_str(), str.c_str(), _length * sizeof(CharT)) == 0;
    }

    void assignConvert(const YCharT* ychar, size_t len = size_t(-1));
    void appendConvert(const YCharT* ychar, size_t len = size_t(-1));

protected:
	static inline size_t Align32(size_t org) {
		static const size_t kAlign = 0x20;
		return (org + kAlign - 1) & ~(kAlign - 1);
	}
	inline size_t calcCapacity(size_t needCapacity) const {
		const size_t oldCapacity = _capacity;
		assert(needCapacity > oldCapacity);
		const size_t alignedCapacity = Align32(needCapacity);
		size_t newCapacity = oldCapacity < kMaxDoubleCapacity ? (oldCapacity << 1) : (oldCapacity + kMaxDoubleCapacity);
		if (newCapacity < alignedCapacity) {
			newCapacity = alignedCapacity;
		}
		return newCapacity;
	}
	inline void enlargeToSize(size_t size) {
		assert(_length < _capacity);
		_capacity = calcCapacity(size);
		CharT* new_buf = (CharT*)malloc(_capacity * sizeof(CharT));
		if (_length) {
			memcpy(new_buf, _dbuf, (_length + 1) * sizeof(CharT));
		} else {
			new_buf[0] = 0;
		}
		if (_dbuf != _sbuf) {
			free(_dbuf);
		}
		_dbuf = new_buf;
	}

private:
	CharT* _sbuf;
	CharT* _dbuf;
	size_t _capacity;
	size_t _length;

private:
	StrBufBase(StrBufBase&& other) = delete;
	StrBufBase(const StrBufBase& other) = delete;
	StrBufBase& operator=(StrBufBase&& other) = delete;
	StrBufBase& operator=(const StrBufBase& other) = delete;
};

template <typename T, size_t Capacity>
class StrBufT : public StrBufBase<T> {
public:
    typedef StrBufBase<T> Base;
    typedef typename Base::CharT CharT;

    StrBufT() : Base(_buf, Capacity) {
	}
	StrBufT(const CharT* format, ...) : Base(_buf, Capacity) {
		va_list ap;
		va_start(ap, format);
		this->vprintf(format, ap);
		va_end(ap);
	}
	StrBufT(const CharT* format, va_list ap) : Base(_buf, Capacity) {
		this->vprintf(format, ap);
	}
	~StrBufT() {
	}

private:
	StrBufT(StrBufT&& other) = delete;
	StrBufT(const StrBufT& other) = delete;
	StrBufT& operator=(StrBufT&& other) = delete;
	StrBufT& operator=(const StrBufT& other) = delete;

private:
	CharT _buf[Capacity];
};

typedef StrBufT<char, 512> StrBuf;
typedef StrBufT<wchar_t, 256> WStrBuf;

}

#endif//shared_utils_StrBuf_h__
