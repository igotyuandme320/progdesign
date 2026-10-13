cmake_minimum_required(VERSION 3.21)
project(MagicBoxDay2 LANGUAGES CXX)

# 使用 C++17，导出 VS Code 所需的真实编译参数。
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
find_package(Qt6 6.5 REQUIRED COMPONENTS Widgets)
if(MSVC)
    add_compile_options(/utf-8)
endif()

# 日常编译只构建桌面程序；测试通过 BUILD_TESTING 单独开启。
set(businessSources src/boxmanager.cpp src/validation.cpp src/datastore.cpp) # 简单业务与存储源码。
add_executable(MagicBox MACOSX_BUNDLE src/main.cpp src/mainwindow.cpp ${businessSources})
target_link_libraries(MagicBox PRIVATE Qt6::Widgets)
set_target_properties(MagicBox PROPERTIES MACOSX_BUNDLE_BUNDLE_NAME "神气的盒子第2天")

option(BUILD_TESTING "构建第二天功能测试" OFF)
if(BUILD_TESTING)
    find_package(Qt6 REQUIRED COMPONENTS Test)
    enable_testing()
    add_executable(day2_test tests/day2_test.cpp src/mainwindow.cpp ${businessSources})
    set_target_properties(day2_test PROPERTIES AUTOMOC ON)
    target_include_directories(day2_test PRIVATE src)
    target_link_libraries(day2_test PRIVATE Qt6::Widgets Qt6::Test)
    add_test(NAME day2_test COMMAND day2_test)
    set_tests_properties(day2_test PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")
endif()
