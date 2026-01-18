cmake_minimum_required(VERSION 3.16)

project(AutoBrightnessWidget VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Test)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Test)


# 设置 OpenCV 路径
# Allow overriding OpenCV_DIR via CMake command line (-DOpenCV_DIR=...)
if(NOT DEFINED OpenCV_DIR)
    set(OpenCV_DIR "C:/OpenCV/opencv/build")
endif()
find_package(OpenCV REQUIRED)
include_directories("${OpenCV_INCLUDE_DIRS}")


# Translation feature disabled to avoid deployment issues
# set(TS_FILES AutoBrightnessWidget_zh_CN.ts)

set(PROJECT_SOURCES
        main.cpp
        mainwindow.cpp
        AutoBrightness.cpp
        CurveEditorWidget.cpp
        CurveEditorDialog.cpp
        mainwindow.h
        mainwindow.ui
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(AutoBrightnessWidget
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        AutoBrightness.h
        CurveEditorWidget.h
        CurveEditorDialog.h
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET AutoBrightnessWidget APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation

    # Translation feature disabled to avoid deployment issues
    # qt_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
else()
    if(ANDROID)
        add_library(AutoBrightnessWidget SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(AutoBrightnessWidget
            ${PROJECT_SOURCES}
        )
    endif()

    # Translation feature disabled to avoid deployment issues
    # qt5_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
endif()

target_link_libraries(AutoBrightnessWidget PRIVATE Qt${QT_VERSION_MAJOR}::Widgets ${OpenCV_LIBS})

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.AutoBrightnessWidget)
endif()
set_target_properties(AutoBrightnessWidget PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS AutoBrightnessWidget
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(AutoBrightnessWidget)
endif()

# Test configuration
option(BUILD_TESTS "Build tests" OFF)

if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
