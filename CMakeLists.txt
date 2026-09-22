cmake_minimum_required(VERSION 3.16)

project(ZhaZhaUpper VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt5 5.15 REQUIRED COMPONENTS Core Gui Quick QuickControls2)

add_executable(ZhaZhaUpper
    src/main.cpp
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
)

include(GNUInstallDirs)
install(TARGETS ZhaZhaUpper
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
