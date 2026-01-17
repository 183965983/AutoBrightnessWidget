# AutoBrightnessWidget

[![Build](https://github.com/183965983/AutoBrightnessWidget/actions/workflows/build.yml/badge.svg)](https://github.com/183965983/AutoBrightnessWidget/actions/workflows/build.yml)
[![Release](https://github.com/183965983/AutoBrightnessWidget/actions/workflows/release.yml/badge.svg)](https://github.com/183965983/AutoBrightnessWidget/actions/workflows/release.yml)

A Qt-based desktop application for automatic screen brightness adjustment using camera input and OpenCV.

## Features

- Automatic brightness adjustment based on ambient light detection
- Uses computer webcam for light sensing
- Qt-based graphical user interface
- Cross-platform support (Windows focus)

## Requirements

### Development Dependencies

- **Qt 6** (or Qt 5) - GUI framework
  - Qt Widgets module
  - Qt LinguistTools module
- **OpenCV 4.8+** - Computer vision library for camera input processing
- **CMake 3.16+** - Build system
- **C++17 compatible compiler**
  - Windows: Visual Studio 2019/2022 (MSVC)

## Building from Source

### Windows

1. **Install Qt**
   - Download from [Qt Official Website](https://www.qt.io/download)
   - Install Qt 6.5.0 or later with MSVC 2019 64-bit

2. **Install OpenCV**
   - Download from [OpenCV Releases](https://github.com/opencv/opencv/releases)
   - Extract to `C:\OpenCV\opencv` (or custom location)

3. **Build with CMake**
   ```cmd
   mkdir build
   cd build
   cmake .. -G "Visual Studio 17 2022" -A x64
   cmake --build . --config Release
   ```

4. **Run**
   ```cmd
   .\Release\AutoBrightnessWidget.exe
   ```

### Custom OpenCV Location

If OpenCV is installed in a different location, specify it during CMake configuration:

```cmd
cmake .. -G "Visual Studio 17 2022" -A x64 -DOpenCV_DIR="C:\path\to\opencv\build"
```

## CI/CD Pipeline

This project uses GitHub Actions for continuous integration and deployment. See [WORKFLOWS.md](.github/WORKFLOWS.md) for detailed information.

### Workflows

- **Build**: Unified CI/CD pipeline with three jobs
  - **Build Job**: Compiles the application with BUILD_TESTS=ON
  - **Test Job**: Runs comprehensive tests (depends on build)
    - Camera functionality tests (using mocked camera with grayscale test images)
    - Windows brightness adjustment API tests (actual API calls with verification)
    - Qt UI functionality tests
  - **Package Job**: Creates release packages (only on push to main branches, depends on test)
  - **Purpose**: Required for PRs - must pass build and test before merging
- **Release**: Creates GitHub releases when version tags are pushed

### Testing

The test suite includes:

1. **Brightness Calculation Tests**: Tests the brightness calculation algorithm using stubbed camera input with grayscale test images
2. **Windows Brightness API Tests**: Tests actual Windows brightness adjustment interface by setting and verifying brightness levels
3. **UI Tests**: Tests Qt widget functionality with mocked camera input

Run tests locally:
```cmd
mkdir build
cd build
cmake .. -G "Visual Studio 17 2022" -A x64 -DOpenCV_DIR="C:\path\to\opencv\build" -DBUILD_TESTS=ON
cmake --build . --config Release
Release\AutoBrightnessWidgetTests.exe
```

### Creating a Release

To create a new release:

```bash
git tag v0.1.0
git push origin v0.1.0
```

The release workflow will automatically build and package the application with all dependencies.

## Usage

1. Launch the application
2. Click the start button to begin automatic brightness adjustment
3. The application will use your webcam to detect ambient light
4. Screen brightness will be adjusted automatically based on lighting conditions
5. Click stop to disable automatic adjustment

## Project Structure

```
AutoBrightnessWidget/
├── .github/
│   ├── workflows/          # GitHub Actions workflows
│   └── WORKFLOWS.md        # CI/CD documentation
├── main.cpp                # Application entry point
├── mainwindow.cpp/h/ui     # Main window UI
├── AutoBrightness.cpp/h    # Core brightness adjustment logic
├── CMakeLists.txt          # CMake build configuration
└── README.md               # This file
```

## Architecture

- **MainWindow**: Qt-based GUI for user interaction
- **AutoBrightness**: Core class handling:
  - Camera capture via OpenCV
  - Brightness calculation from video frames
  - System brightness adjustment
  - Threading for continuous monitoring

## Contributing

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/amazing-feature`)
3. Commit your changes (`git commit -m 'Add amazing feature'`)
4. Push to the branch (`git push origin feature/amazing-feature`)
5. Open a Pull Request

## License

This project is open source. Please check the repository for license information.

## Troubleshooting

### Camera Access Issues

- Ensure your webcam is properly connected
- Check that no other application is using the webcam
- Verify camera permissions in your system settings

### Build Errors

- Verify Qt and OpenCV are properly installed
- Check that CMake can find Qt and OpenCV paths
- Ensure all dependencies are for the correct architecture (x64)

### OpenCV Not Found

If CMake cannot find OpenCV:
```cmd
cmake .. -DOpenCV_DIR="C:\path\to\opencv\build"
```

## Support

For issues, questions, or contributions, please open an issue on GitHub.
