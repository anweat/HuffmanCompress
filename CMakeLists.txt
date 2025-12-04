cmake_minimum_required(VERSION 3.16)

project(HuffmanCompress VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets LinguistTools)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets LinguistTools)

# 设置源文件和头文件路径
set(TS_FILES HuffmanCompress_zh_CN.ts)

# 添加FileStream目录下的源文件
file(GLOB FILE_STREAM_SOURCES "./FileStream/*.cpp")
file(GLOB FILE_STREAM_HEADERS "./FileStream/*.h")

# 添加huffman目录下的源文件
file(GLOB HUFMAN_SOURCES "./huffman/*.cpp")
file(GLOB HUFMAN_HEADERS "./huffman/*.h")

# 添加FileTaskPool目录下的源文件
file(GLOB TASK_POOL_SOURCES "./FileTaskPool/*.cpp")
file(GLOB TASK_POOL_HEADERS "./FileTaskPool/*.h")

# 添加logger目录下的源文件
file(GLOB LOGGER_SOURCES "./logger/*.cpp")
file(GLOB LOGGER_HEADERS "./logger/*.h")

# 添加task目录下的源文件
file(GLOB TASK_SOURCES "./task/*.cpp")
file(GLOB TASK_HEADERS "./task/*.h")

set(PROJECT_SOURCES
        main.cpp
        mainwindow.cpp
        mainwindow.h
        mainwindow.ui
        ${FILE_STREAM_SOURCES}
        ${FILE_STREAM_HEADERS}
        ${HUFMAN_SOURCES}
        ${HUFMAN_HEADERS}
        ${TASK_POOL_SOURCES}
        ${TASK_POOL_HEADERS}
        ${LOGGER_SOURCES}
        ${LOGGER_HEADERS}
        ${TASK_SOURCES}
        ${TASK_HEADERS}
        ${TS_FILES}
)

# 添加包含目录
include_directories(".")
include_directories("./FileStream")
include_directories("./huffman")
include_directories("./FileTaskPool")
include_directories("./logger")
include_directories("./task")

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(HuffmanCompress
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET HuffmanCompress APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation

    qt_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
else()
    if(ANDROID)
        add_library(HuffmanCompress SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(HuffmanCompress
            ${PROJECT_SOURCES}
        )
    endif()

    qt5_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
endif()

target_link_libraries(HuffmanCompress PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)

# 确保UTF-8编码
if(MSVC)
    target_compile_options(HuffmanCompress PRIVATE /utf-8)
endif()

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.HuffmanCompress)
endif()
set_target_properties(HuffmanCompress PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS HuffmanCompress
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(HuffmanCompress)
endif()
