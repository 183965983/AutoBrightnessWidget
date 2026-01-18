# 测试指南

## 概述
本文档说明如何构建和运行 AutoBrightnessWidget 项目的测试套件。

## 测试框架
- **框架**: Qt Test (QTest)
- **语言**: C++17
- **依赖**: Qt Widgets, Qt Test, OpenCV

## 测试文件结构

```
tests/
├── CMakeLists.txt           # 测试构建配置
├── test_main.cpp            # 测试入口和注册
├── test_brightness.cpp      # 亮度计算测试
├── test_ui.cpp              # UI 基础测试
├── test_new_features.cpp    # 新功能测试
├── MockCamera.h/.cpp        # 模拟摄像头
└── test_images/             # 测试图像资源
```

## 构建测试

### Windows (Visual Studio)

```bash
# 1. 创建构建目录
mkdir build
cd build

# 2. 配置 CMake（启用测试）
cmake .. -G "Visual Studio 17 2022" -A x64 -DBUILD_TESTS=ON

# 3. 编译测试
cmake --build . --config Release

# 4. 运行测试
Release\AutoBrightnessWidgetTests.exe
```

### Linux / macOS

```bash
# 1. 创建构建目录
mkdir build
cd build

# 2. 配置 CMake（启用测试）
cmake .. -DBUILD_TESTS=ON

# 3. 编译测试
cmake --build . --config Release

# 4. 运行测试
./AutoBrightnessWidgetTests
```

## 测试套件说明

### 1. TestBrightness
**文件**: `test_brightness.cpp`

**测试内容**:
- 基础亮度计算
- 采样点配置
- 图像处理算法

**测试用例**:
- `test_brightness_calculation()` - 验证亮度计算正确性
- `test_sample_points_configuration()` - 测试采样点设置

### 2. TestUI
**文件**: `test_ui.cpp`

**测试内容**:
- 主窗口创建
- UI 控件存在性
- 基本交互

**测试用例**:
- `test_main_window_creation()` - 主窗口正常创建
- `test_ui_elements_exist()` - 关键 UI 元素存在

### 3. TestNewFeatures
**文件**: `test_new_features.cpp`

**测试内容**:
- 实时亮度显示功能
- 配置按钮和验证
- 曲线调整功能
- 设置持久化

**测试用例**:
- `test_brightness_display_widgets_exist()` - 亮度显示控件
- `test_config_buttons_exist()` - 配置按钮和 SpinBox
- `test_advanced_settings_checkboxes()` - 高级设置复选框
- `test_brightness_configuration()` - 亮度配置方法
- `test_curve_configuration()` - 曲线配置
- `test_settings_save_load()` - 设置保存和加载
- `test_brightness_validation()` - 配置验证逻辑（**新增**）
- `test_curve_editor_widget()` - 曲线编辑器 UI（**新增**）

## 预期测试结果

### 成功输出示例

```
********* Start testing of TestBrightness *********
Config: Using QtTest library 6.x.x
PASS   : TestBrightness::initTestCase()
PASS   : TestBrightness::test_brightness_calculation()
PASS   : TestBrightness::test_sample_points_configuration()
PASS   : TestBrightness::cleanupTestCase()
Totals: 4 passed, 0 failed, 0 skipped, 0 blacklisted, Xms
********* Finished testing of TestBrightness *********

********* Start testing of TestUI *********
Config: Using QtTest library 6.x.x
PASS   : TestUI::initTestCase()
PASS   : TestUI::test_main_window_creation()
PASS   : TestUI::test_ui_elements_exist()
PASS   : TestUI::cleanupTestCase()
Totals: 4 passed, 0 failed, 0 skipped, 0 blacklisted, Xms
********* Finished testing of TestUI *********

********* Start testing of TestNewFeatures *********
Config: Using QtTest library 6.x.x
PASS   : TestNewFeatures::initTestCase()
PASS   : TestNewFeatures::test_brightness_display_widgets_exist()
PASS   : TestNewFeatures::test_config_buttons_exist()
PASS   : TestNewFeatures::test_advanced_settings_checkboxes()
PASS   : TestNewFeatures::test_brightness_configuration()
PASS   : TestNewFeatures::test_curve_configuration()
PASS   : TestNewFeatures::test_settings_save_load()
PASS   : TestNewFeatures::test_brightness_validation()
PASS   : TestNewFeatures::test_curve_editor_widget()
PASS   : TestNewFeatures::cleanupTestCase()
Totals: 10 passed, 0 failed, 0 skipped, 0 blacklisted, Xms
********* Finished testing of TestNewFeatures *********
```

## 常见问题

### Q1: 找不到 Qt
**错误**: `CMake Error: Could not find Qt`

**解决**:
```bash
# 设置 Qt 路径
cmake .. -DCMAKE_PREFIX_PATH="C:/Qt/6.x.x/msvc2019_64" -DBUILD_TESTS=ON
```

### Q2: 找不到 OpenCV
**错误**: `CMake Error: Could not find OpenCV`

**解决**:
```bash
# 设置 OpenCV 路径
cmake .. -DOpenCV_DIR="C:/OpenCV/opencv/build" -DBUILD_TESTS=ON
```

### Q3: 测试运行时找不到 DLL
**错误**: 运行测试时提示缺少 DLL

**解决**:
- 确保 Qt 和 OpenCV 的 DLL 在 PATH 中
- 或将 DLL 复制到测试可执行文件目录

### Q4: UI 测试失败
**错误**: UI 相关测试失败

**原因**: 可能需要显示服务器（Linux）

**解决**:
```bash
# Linux 下使用 xvfb-run
xvfb-run ./AutoBrightnessWidgetTests
```

## 持续集成

### GitHub Actions 示例

```yaml
name: Tests

on: [push, pull_request]

jobs:
  test:
    runs-on: windows-latest
    steps:
      - uses: actions/checkout@v2
      
      - name: Install Qt
        uses: jurplel/install-qt-action@v3
        with:
          version: '6.5.0'
      
      - name: Install OpenCV
        run: |
          # 安装 OpenCV
      
      - name: Configure CMake
        run: cmake -B build -DBUILD_TESTS=ON
      
      - name: Build
        run: cmake --build build --config Release
      
      - name: Run Tests
        run: build\Release\AutoBrightnessWidgetTests.exe
```

## 测试覆盖率

### 当前覆盖情况

| 模块 | 测试覆盖 | 说明 |
|------|---------|------|
| AutoBrightness 核心 | ✅ 高 | 亮度计算、配置、验证 |
| UI 控件 | ✅ 中 | 主要控件存在性测试 |
| 曲线功能 | ✅ 中 | 后端逻辑和 UI 元素 |
| 托盘功能 | ⚠️ 低 | 手动测试为主 |
| 自启动功能 | ⚠️ 低 | 需要注册表访问，难以自动化 |

### 改进建议

1. **增加集成测试**: 端到端功能流程测试
2. **Mock 系统调用**: 托盘、注册表等系统功能
3. **UI 交互测试**: 模拟用户点击、输入等操作
4. **性能测试**: 亮度计算性能基准测试

## 调试测试

### 单独运行某个测试类

Qt Test 支持通过命令行参数运行特定测试：

```bash
# 只运行 TestNewFeatures
AutoBrightnessWidgetTests.exe TestNewFeatures

# 只运行某个测试函数
AutoBrightnessWidgetTests.exe TestNewFeatures::test_brightness_validation
```

### 详细输出

```bash
# 输出详细日志
AutoBrightnessWidgetTests.exe -v2

# 输出所有调试信息
AutoBrightnessWidgetTests.exe -v2 -maxwarnings 0
```

### Visual Studio 调试

1. 在 Visual Studio 中打开项目
2. 右键 `AutoBrightnessWidgetTests` 项目
3. 设置为启动项目
4. 按 F5 调试运行

## 贡献测试

### 编写新测试

1. 在 `tests/` 目录创建新测试文件（或编辑现有文件）
2. 继承 `QObject` 并使用 `Q_OBJECT` 宏
3. 在 `private slots:` 中定义测试函数
4. 使用 `QVERIFY`, `QCOMPARE` 等宏进行断言
5. 在 `test_main.cpp` 中注册测试类

### 测试命名规范

- 测试类: `TestXXX`
- 测试函数: `test_具体功能描述()`
- 使用下划线分隔单词，保持一致性

### 示例

```cpp
class TestMyFeature : public QObject
{
    Q_OBJECT

private slots:
    void test_basic_functionality() {
        // 测试代码
        QVERIFY(someCondition);
        QCOMPARE(actualValue, expectedValue);
    }
};

// 在 test_main.cpp 中注册
QTest::qExec(new TestMyFeature(), argc, argv);
```

---

**更新日期**: 2026-01-18  
**版本**: 1.0  
**维护者**: AutoBrightnessWidget Team
