# CppBuildUnifier

Build C++ is so slow? Unifier can make it fast.

## Question!

In big project, cpp developer need Object Oriented, and produce a lot of cpp/cxx files.

This make project build slow. Class template parse one in each file build.

## How to speed up!

Make multi-files to unified-file to speed up compile.

Unifier will create less new c/cpp files to include old ones, just build less files, so class template will parse less times.

## What project CppBuildUnifier support
  
  1. Android.mk
    * single target
    * configure Android.lst decide build file list
    * change original Android.mk
  2. xcodeproj
    - support multi target
    - use ${Target}.lst and reference in project to create build file list
    - change original xcodeproj

## Which project can use CppBuildUnifier

  * same compile options for all files
  * not for big files

## Dependency

- [nom for android.mk parse](https://github.com/aep/nom)
