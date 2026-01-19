# Test CMakeLists.txt

# Create test executable
set(TEST_SOURCES
    test_main.cpp
    test_brightness.cpp
    test_ui.cpp
    test_new_features.cpp
    test_status_bar.cpp
    MockCamera.cpp
    MockCamera.h
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(AutoBrightnessWidgetTests
        ${TEST_SOURCES}
        ../AutoBrightness.cpp
        ../AutoBrightness.h
        ../CurveEditorWidget.cpp
        ../CurveEditorWidget.h
        ../CurveEditorDialog.cpp
        ../CurveEditorDialog.h
        ../mainwindow.cpp
        ../mainwindow.h
        ../mainwindow.ui
    )
else()
    add_executable(AutoBrightnessWidgetTests
        ${TEST_SOURCES}
        ../AutoBrightness.cpp
        ../AutoBrightness.h
        ../CurveEditorWidget.cpp
        ../CurveEditorWidget.h
        ../CurveEditorDialog.cpp
        ../CurveEditorDialog.h
        ../mainwindow.cpp
        ../mainwindow.h
    )
endif()

target_link_libraries(AutoBrightnessWidgetTests PRIVATE 
    Qt${QT_VERSION_MAJOR}::Widgets 
    Qt${QT_VERSION_MAJOR}::Test
    ${OpenCV_LIBS}
)

target_include_directories(AutoBrightnessWidgetTests PRIVATE ${CMAKE_SOURCE_DIR})

# Add tests
add_test(NAME AutoBrightnessWidgetTests COMMAND AutoBrightnessWidgetTests)

# Copy test images to build directory
file(COPY test_images DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
