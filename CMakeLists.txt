cmake_minimum_required(VERSION 3.16)

project(ZhaZhaUpper VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt5 5.15 REQUIRED COMPONENTS Core Gui Quick QuickControls2)

# coost 使用 Condition Variable 等 Vista+ API，需在包含 windows.h 前定义
if(WIN32)
    add_compile_definitions(
        WIN32_LEAN_AND_MEAN
        NOMINMAX
        _WIN32_WINNT=0x0601
        WINVER=0x0601
    )
endif()

# backstage 第三方 / 子模块（各自有 CMakeLists）
set(BUILD_ALL OFF CACHE BOOL "Build all coost projects" FORCE)
add_subdirectory(src/backstage/cJSON)
add_subdirectory(src/backstage/ComHost/com_host)
add_subdirectory(src/backstage/coost-4.0.0)

# 忽略 coost 编译与头文件引入时的全部警告
if(MSVC)
    target_compile_options(co PRIVATE /W0)
else()
    target_compile_options(co PRIVATE -w)
endif()
get_target_property(_coost_inc co INTERFACE_INCLUDE_DIRECTORIES)
if(_coost_inc)
    set_property(TARGET co PROPERTY INTERFACE_SYSTEM_INCLUDE_DIRECTORIES "${_coost_inc}")
endif()

add_executable(ZhaZhaUpper
    src/main.cpp
    src/Bridge.cpp
    src/Bridge.hpp
    src/driver/QDebugRWPort.hpp
    src/backstage/ComHost/ComHost.cpp
    src/backstage/ComHost/ComHost.hpp
    src/backstage/RWPort/RWPort.hpp
    qml.qrc
)

set_target_properties(ZhaZhaUpper PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)

target_link_libraries(ZhaZhaUpper
    PRIVATE
        Qt5::Core
        Qt5::Gui
        Qt5::Quick
        Qt5::QuickControls2
        com_host
        cJSON
        co
)

include(GNUInstallDirs)
install(TARGETS ZhaZhaUpper
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
