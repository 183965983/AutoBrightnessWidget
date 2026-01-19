# AutoBrightnessWidget

[![Build](https://github.com/183965983/AutoBrightnessWidget/actions/workflows/build.yml/badge.svg)](https://github.com/183965983/AutoBrightnessWidget/actions/workflows/build.yml)
[![Release](https://github.com/183965983/AutoBrightnessWidget/actions/workflows/release.yml/badge.svg)](https://github.com/183965983/AutoBrightnessWidget/actions/workflows/release.yml)
[![Auto Release](https://github.com/183965983/AutoBrightnessWidget/actions/workflows/auto-release.yml/badge.svg)](https://github.com/183965983/AutoBrightnessWidget/actions/workflows/auto-release.yml)

A Qt-based desktop application for automatic screen brightness adjustment using camera input and OpenCV.

## 下载 / Download

### 📦 最新预发布版本 / Latest Pre-release

每次代码合并后会自动构建并发布预览版本，包含最新的功能更新。

**两种安装方式：**

#### 1️⃣ Windows 安装包（推荐）
- **[下载安装程序](https://github.com/183965983/AutoBrightnessWidget/releases)** (`AutoBrightnessWidget-Setup-Windows-x64.exe`)
- ✅ 完整的安装向导，自动配置所有组件
- ✅ 创建桌面快捷方式和开始菜单项
- ✅ 包含所有必需的运行时依赖库
- ✅ 支持一键卸载
- 📌 适合大多数用户

#### 2️⃣ 便携版（免安装）
- **[下载压缩包](https://github.com/183965983/AutoBrightnessWidget/releases)** (`AutoBrightnessWidget-Portable-Windows-x64.zip`)
- ✅ 解压即用，无需安装
- ✅ 可放在 U 盘或任意目录
- ✅ 包含所有必需的运行库
- 📌 适合需要便携使用的用户

> **获取方式：** 访问 [Releases 页面](https://github.com/183965983/AutoBrightnessWidget/releases)，选择最新的预发布版本（标记为 Pre-release），下载对应的文件。

### 🎯 正式发布版本 / Stable Release

正式版本经过完整测试，适合追求稳定性的用户：
- **[下载最新稳定版](https://github.com/183965983/AutoBrightnessWidget/releases/latest)**

### 🔨 开发构建版本 / Development Build

如需最新的开发构建（包含实验性功能），可以从 CI/CD 工作流下载：
1. 访问 [Build workflow 页面](https://github.com/183965983/AutoBrightnessWidget/actions/workflows/build.yml)
2. 点击最新的成功运行记录
3. 在 "Artifacts" 部分下载 `release-package`

**注意：** 开发构建未经过完整测试，可能存在问题。

## 功能特性 / Features

- 🌞 基于环境光自动调节屏幕亮度 / Automatic brightness adjustment based on ambient light
- 📷 使用电脑摄像头感知环境光线 / Uses computer webcam for light sensing
- 🎨 Qt 图形界面，操作简单直观 / Qt-based graphical user interface
- 💻 跨平台支持（主要支持 Windows）/ Cross-platform support (Windows focus)
- 📦 提供安装包和便携版两种分发形式 / Available as installer and portable version
- 🔄 自动更新检查器：启动时检查新版本并通知用户 / Auto-update checker: Checks for new releases on startup and notifies users

## 系统要求 / Requirements

### 运行要求 / Runtime Requirements
- **操作系统 / OS**: Windows 10/11 (64位)
- **硬件 / Hardware**: 带摄像头的设备 / Device with webcam
- **运行库 / Runtime**: Visual C++ 运行库（安装包已包含所需 DLL）/ Visual C++ runtime (installer includes required DLLs)

### 开发依赖 / Development Dependencies

- **Qt 6** (或 Qt 5) - GUI 框架 / GUI framework
  - Qt Widgets 模块 / Qt Widgets module
  - Qt LinguistTools 模块 / Qt LinguistTools module
- **OpenCV 4.8+** - 计算机视觉库，用于摄像头处理 / Computer vision library for camera input processing
- **CMake 3.16+** - 构建系统 / Build system
- **C++17 兼容编译器** / C++17 compatible compiler
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
- **Auto-Release**: 🆕 Automatically creates pre-releases on every merge to main/master
  - Generates both installer and portable packages
  - See [AUTO_RELEASE.md](AUTO_RELEASE.md) for details

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

## 安装使用 / Installation & Usage

### 安装方法 / Installation

#### 方式一：使用安装包（推荐）
1. 从 [Releases 页面](https://github.com/183965983/AutoBrightnessWidget/releases) 下载 `AutoBrightnessWidget-Setup-Windows-x64.exe`
2. 双击运行安装程序
3. 按照安装向导完成安装
4. 从开始菜单或桌面快捷方式启动程序

#### 方式二：使用便携版
1. 从 [Releases 页面](https://github.com/183965983/AutoBrightnessWidget/releases) 下载 `AutoBrightnessWidget-Portable-Windows-x64.zip`
2. 解压到任意目录
3. 双击 `AutoBrightnessWidget.exe` 运行

### 使用说明 / Usage

1. 启动应用程序 / Launch the application
2. 点击"开始"按钮启用自动亮度调节 / Click the start button to begin automatic brightness adjustment
3. 程序将使用摄像头检测环境光线 / The application will use your webcam to detect ambient light
4. 屏幕亮度会根据光线条件自动调整 / Screen brightness will be adjusted automatically based on lighting conditions
5. 点击"停止"按钮禁用自动调节 / Click stop to disable automatic adjustment

### Auto-Update Feature

The application automatically checks for updates when launched:
- If a new version is available, a dialog will appear with the update details
- Click "下载新版本" to visit the release page and download the latest version
- Click "暂不更新" to skip the update and continue using the current version
- The update check happens in the background and won't block the application startup

1. 启动应用程序 / Launch the application
2. 点击"开始"按钮启用自动亮度调节 / Click the start button to begin automatic brightness adjustment
3. 程序将使用摄像头检测环境光线 / The application will use your webcam to detect ambient light
4. 屏幕亮度会根据光线条件自动调整 / Screen brightness will be adjusted automatically based on lighting conditions
5. 点击"停止"按钮禁用自动调节 / Click stop to disable automatic adjustment

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
- **UpdateChecker**: Handles automatic update checks:
  - Queries GitHub API for latest releases
  - Compares version numbers
  - Notifies when updates are available
- **UpdateDialog**: Displays update notifications with release information

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
