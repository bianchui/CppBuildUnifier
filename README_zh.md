# CppBuildUnifier

## 问题来源

大项目中，开发者需要按照面向对象的规则将每个功能拆分，每个类一套 cpp/cxx 和 h/hpp 文件，让代码更容易维护。

这样会带来大量的只有几行或者几十行的 cpp 文件，这将大大减慢编译速度。

## 为什么能加速？

通过将多个C++文件整合到一个文件编译，可以提升编译速度。

生成一些中间c/cpp/m/mm文件将原有文件include进来，不修改原始代码。

相同的库文件只会被解析一次，可以减少模版分析占用的时间。

## CppBuildUnifier支持什么项目呢
  
  1. Android.mk
    * 支持单一target
    * 通过配置文件(Android.lst)决定文件确定编译文件列表
    * 修改原始文件
  2. xcodeproj
    - 支持多个target
    - 通过配置文件(${Target}.lst)和项目内引用文件确定编译文件列表
    - 修改原始文件
  3. vcxproj
    - 以project为单位
    - 通过vcxproj引用的文件确定编译文件列表
    - 生成一个新的项目，不会修改原始项目
    - 若要使用pch需要单独创建一个vcxproj来生成
  
## CppBuildUnifier使用有什么条件呢
  
  * 编译选项都是统一的，即没有为单个文件指定编译选项
  * 不适合编译单个文件就已经编译很慢的文件，这样的文件编译时可能占用太多内存从而减慢编译

## 灵感来源

这个项目是好几年前写的了，原本是用来加速 cocos2dx 的编译，3.0版本的 cocos2dx 的粒子系统有很多文件，但是每个文件只有几行代码，导致编译很慢。
在使用Unreal引擎时，Unreal会将多个 cpp 文件 include 到一个 cpp 中以减小编译时间，这个方法是可以通用的。

## Dependency

- [nom for android.mk parse](https://github.com/aep/nom)
