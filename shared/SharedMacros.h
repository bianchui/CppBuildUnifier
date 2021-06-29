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

#ifndef __shared_SharedMacros_h__
#define __shared_SharedMacros_h__
#include <stdlib.h>

#define CLASS_DELETE_COPY_CONSTRUCTOR(cls) \
/**/cls(const cls&) = delete; \

#define CLASS_DELETE_COPY_ASSIGN(cls) \
/**/cls& operator=(const cls&) = delete; \

#define CLASS_DELETE_COPY(cls) \
/**/cls(const cls&) = delete; \
/**/cls& operator=(const cls&) = delete; \

#define CLASS_DELETE_MOVE_CONSTRUCTOR(cls) \
/**/cls(cls&&) = delete; \

#define CLASS_DELETE_MOVE_ASSIGN(cls) \
/**/cls& operator=(cls&&) = delete; \

#define CLASS_DELETE_MOVE(cls) \
/**/cls(cls&&) = delete; \
/**/cls& operator=(cls&&) = delete; \

#define CLASS_DELETE_COPY_MOVE(cls) \
/**/CLASS_DELETE_COPY(cls); \
/**/CLASS_DELETE_MOVE(cls); \

#define CLASS_ZEROED_NEW() \
/**/static void* operator new(size_t size) { \
/**/    void* mem = malloc(size); \
/**/    memset(mem, 0, size); \
/**/    return mem; \
/**/} \
/**/static void operator delete(void* p) { \
/**/    free(p); \
/**/} \
/**/static void* operator new(size_t size, const std::nothrow_t&) { \
/**/    void* mem = malloc(size); \
/**/    memset(mem, 0, size); \
/**/    return mem; \
/**/} \
/**/static void operator delete(void* p, const std::nothrow_t&) { \
/**/    free(p); \
/**/} \
/**/static void* operator new(size_t size, void* p) { \
/**/    memset(p, 0, size); \
/**/    return p; \
/**/} \
/**/static void operator delete(void* p, void*) { \
/**/} \

#ifdef NDEBUG

#  define DEBUGF(...)
#  define DEBUG_ONLY(...)

#else

#  define DEBUGF(...) printf(__VA_ARGS__)
#  define DEBUG_ONLY(...) __VA_ARGS__

#endif//DEBUG

#define NAMESPACE_BEGIN(ns) namespace ns {
#define NAMESPACE_END(ns) }

#ifndef NS_SHARED
#  define NS_SHARED shared
#endif//NS_SHARED

#define SHARED_NAMESPACE_BEGIN NAMESPACE_BEGIN(NS_SHARED)
#define SHARED_NAMESPACE_END NAMESPACE_END(NS_SHARED)


// compilers
#ifdef _MSC_VER // for MSVC

#  define UNUSED(x) __pragma(warning(suppress:4100)) x
#  define UNUSED_VAR
#  define forceinline __forceinline
#  define STRUCT_PACKED __pragma(pack(1))
#  define CHECK_FMT(a, b)

#  if defined _M_X64 || defined _M_ARM || defined _M_ARM64
#    define UNALIGNED_DATA __unaligned
#  else
#    define UNALIGNED_DATA
#  endif

#elif defined __GNUC__ // for gcc on Linux/Apple OS X

#  define UNUSED(x) (void)x
#  define UNUSED_VAR __attribute__((__unused__))
#  define forceinline __inline__ __attribute__((always_inline))
#  define STRUCT_PACKED __attribute__((packed))
#  define CHECK_FMT(a, b) __attribute__((format(printf, a, b)))

#  define UNALIGNED_DATA

#else

#  error unknown compiler
#  define UNUSED(x) (void)x
#  define UNUSED_VAR
#  define forceinline
#  define STRUCT_PACKED
#  define CHECK_FMT(a, b)

#  define UNALIGNED_DATA

#endif

#ifdef __cplusplus
#  define C_EXTERN_C extern "C"
#else
#  define C_EXTERN_C extern
#endif

#ifndef _countof
#  ifdef __cplusplus
extern "C++" {
    template <typename _CountofType, size_t _SizeOfArray>
    char (*__countof_helper(UNALIGNED_DATA _CountofType (&_Array)[_SizeOfArray]))[_SizeOfArray];
}
#      define _countof(_Array) (sizeof(*__countof_helper(_Array)) + 0)
#  else
#      define _countof(_Array) (sizeof(_Array) / sizeof(_Array[0]))
#  endif
#endif//_countof

#endif//__shared_SharedMacros_h__
