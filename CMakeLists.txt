cmake_minimum_required(VERSION 3.21)
project(MagicBoxDay1 LANGUAGES CXX)

# 使用 C++17，导出 VS Code 所需的真实编译参数。
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
find_package(Qt6 6.5 REQUIRED COMPONENTS Widgets)
if(MSVC)
    add_compile_options(/utf-8)
endif()

# 第一天只有一个窗口程序，不包含截图、测试或样例生成工具。
add_executable(MagicBox MACOSX_BUNDLE src/main.cpp src/mainwindow.cpp src/boxmanager.cpp)
target_link_libraries(MagicBox PRIVATE Qt6::Widgets)
set_target_properties(MagicBox PROPERTIES MACOSX_BUNDLE_BUNDLE_NAME "神气的盒子第1天")
