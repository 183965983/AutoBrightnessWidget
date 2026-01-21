# Test CMakeLists.txt

# Create test executable
set(TEST_SOURCES
    test_main.cpp
    test_brightness.cpp
    test_ui.cpp
    test_new_features.cpp
    test_multi_monitor.cpp
    test_update_checker.cpp
    test_status_bar.cpp
    test_realtime_curve.cpp
    MockCamera.cpp
    MockCamera.h
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(AutoBrightnessWidgetTests
        ${TEST_SOURCES}
        ../Ctrl/AutoBrightness.cpp
        ../Ctrl/AutoBrightness.h
        ../UI/CurveEditorWidget.cpp
        ../UI/CurveEditorWidget.h
        ../UI/CurveEditorDialog.cpp
        ../UI/CurveEditorDialog.h
        ../UI/BrightnessCurveWidget.cpp
        ../UI/BrightnessCurveWidget.h
        ../Ctrl/UpdateChecker.cpp
        ../Ctrl/UpdateChecker.h
        ../UI/UpdateDialog.cpp
        ../UI/UpdateDialog.h
        ../UI/mainwindow.cpp
        ../UI/mainwindow.h
        ../UI/mainwindow.ui
    )
else()
    add_executable(AutoBrightnessWidgetTests
        ${TEST_SOURCES}
        ../Ctrl/AutoBrightness.cpp
        ../Ctrl/AutoBrightness.h
        ../UI/CurveEditorWidget.cpp
        ../UI/CurveEditorWidget.h
        ../UI/CurveEditorDialog.cpp
        ../UI/CurveEditorDialog.h
        ../UI/BrightnessCurveWidget.cpp
        ../UI/BrightnessCurveWidget.h
        ../Ctrl/UpdateChecker.cpp
        ../Ctrl/UpdateChecker.h
        ../UI/UpdateDialog.cpp
        ../UI/UpdateDialog.h
        ../UI/mainwindow.cpp
        ../UI/mainwindow.h
    )
endif()

target_link_libraries(AutoBrightnessWidgetTests PRIVATE 
    Qt${QT_VERSION_MAJOR}::Widgets 
    Qt${QT_VERSION_MAJOR}::Test
    Qt${QT_VERSION_MAJOR}::Network
    ${OpenCV_LIBS}
)

target_include_directories(AutoBrightnessWidgetTests PRIVATE ${CMAKE_SOURCE_DIR})

# Add tests
add_test(NAME AutoBrightnessWidgetTests COMMAND AutoBrightnessWidgetTests)

# Copy test images to build directory
file(COPY test_images DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
