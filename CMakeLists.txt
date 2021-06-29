cmake_minimum_required(VERSION 3.0)

set(CMAKE_BUILD_TYPE "Release")

project(CppBuildUnifier)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

include_directories(${PROJECT_SOURCE_DIR})

message("${CMAKE_CXX_COMPILER_ID}")

set(CC_CLANG_FLAGS "-fno-threadsafe-statics -Oz -fno-stack-protector")

if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "(.*)Clang(.*)")
  # using Clang
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${CC_CLANG_FLAGS}")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CC_CLANG_FLAGS} -fvisibility-inlines-hidden -fno-exceptions -fno-rtti")
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -dead_strip")

elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
  # using GCC
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -s")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s")

elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Intel")
  # using Intel C++

elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
  # using Visual Studio C++

endif()

add_executable(
  android_mk_unifier
    shared/SSources.cpp
    android_mk_unifier/Android.mk.cpp
    android_mk_unifier/nom/src/lexer.cpp
    android_mk_unifier/main.cpp
)

add_executable(
  xcodeproj_unifier
    shared/SSources.cpp
    xcodeproj_unifier/xcodeproj/namehash.cpp
    xcodeproj_unifier/xcodeproj/NeXTSTEP_plist.cpp
    xcodeproj_unifier/xcodeproj/pbxproj_parser.cpp
    xcodeproj_unifier/UnifiedXcodeProject.cpp
    xcodeproj_unifier/XcodeProjUnifier.cpp
    xcodeproj_unifier/main.cpp
)
