# 新功能说明

本文档说明最近添加的新功能实现。

## 1. 实时亮度反馈显示

### 功能描述
- **摄像头画面亮度显示**：使用灰阶色块实时显示当前摄像头检测到的亮度值（0-100）
- **屏幕亮度显示**：大号数字实时显示当前设置的屏幕亮度值（0-100）

### 实现细节
- 在 UI 中添加了"实时亮度显示"分组框
- 左侧显示摄像头亮度，包括：
  - 灰阶色块：根据亮度值自动调整灰度（0=黑色，100=白色）
  - 数字显示：当前摄像头亮度值
- 右侧显示屏幕亮度的大号数字
- 使用 Qt 信号槽机制实时更新：
  - `AutoBrightness::cameraBrightnessChanged(int)` 信号
  - `AutoBrightness::screenBrightnessChanged(int)` 信号
  - `MainWindow::updateCameraBrightness(int)` 槽
  - `MainWindow::updateScreenBrightness(int)` 槽

## 2. 自动配置功能

### 功能描述
允许用户根据当前环境配置最大和最小亮度映射关系。

### 使用方法
1. 启动程序，等待摄像头捕获当前环境亮度
2. 在最暗环境下：
   - 在"最小屏幕亮度" SpinBox 中设置期望的最小屏幕亮度（0-100）
   - 点击"配置最小亮度"按钮，系统将记录当前摄像头亮度作为最小值基准
3. 在最亮环境下：
   - 在"最大屏幕亮度" SpinBox 中设置期望的最大屏幕亮度（0-100）
   - 点击"配置最大亮度"按钮，系统将记录当前摄像头亮度作为最大值基准
4. 配置完成后，系统将自动在这两个点之间进行线性插值映射

### 实现细节
- 配置保存在 QSettings 中，下次启动自动加载
- 映射算法：线性插值
  ```
  screenBrightness = minScreen + (cameraValue - minCamera) / (maxCamera - minCamera) * (maxScreen - minScreen)
  ```
- 超出范围的摄像头亮度值会被限制在配置范围内

## 3. 曲线调整功能

### 功能描述
支持使用自定义曲线来调整屏幕亮度和摄像头画面的映射关系（高级功能）。

### 当前状态
- **后端实现**：完整的曲线数据结构和多点线性插值算法已实现
- **UI界面**：预留了"使用曲线调整"复选框，但曲线编辑界面标注为"暂未实现"
- 可通过编程方式设置曲线点：
  ```cpp
  std::vector<std::pair<int, int>> curvePoints = {
      {0, 0},    // 摄像头亮度0 -> 屏幕亮度0
      {30, 20},  // 摄像头亮度30 -> 屏幕亮度20
      {70, 80},  // 摄像头亮度70 -> 屏幕亮度80
      {100, 100} // 摄像头亮度100 -> 屏幕亮度100
  };
  AutoBrightness::getInstance()->setCurvePoints(curvePoints);
  AutoBrightness::getInstance()->setUseCurve(true);
  ```

### 未来改进
- 可添加可视化曲线编辑器
- 支持贝塞尔曲线或样条插值
- 预设曲线模板（线性、对数、指数等）

## 4. 托盘最小化功能

### 功能描述
支持将程序最小化到系统托盘，在后台继续运行。

### 使用方法
1. 勾选"关闭时最小化到托盘"复选框
2. 点击"启动"开始自动亮度调节
3. 点击窗口关闭按钮（X），程序会：
   - 如果正在运行且勾选了托盘选项：最小化到托盘，显示提示消息
   - 如果已停止或未勾选托盘选项：正常退出程序

### 托盘菜单
- 双击托盘图标：显示主窗口
- 右键托盘图标：
  - "显示窗口"：显示主窗口
  - "退出"：完全退出程序

### 实现细节
- 使用 `QSystemTrayIcon` 实现
- 托盘图标使用应用程序默认图标
- 重载 `closeEvent()` 实现关闭行为控制
- 托盘设置保存在 QSettings 中

## 5. 开机自启动配置

### 功能描述
支持设置程序随 Windows 系统启动。

### 使用方法
1. 勾选"开机自启动"复选框：程序会添加到 Windows 启动项
2. 取消勾选：从启动项中移除

### 实现细节
- **Windows 平台**：通过注册表实现
  - 注册表路径：`HKEY_CURRENT_USER\Software\Microsoft\Windows\CurrentVersion\Run`
  - 键名：`AutoBrightnessWidget`
  - 键值：应用程序完整路径
- **其他平台**：当前未实现（可扩展支持 Linux/macOS）

### 注意事项
- 需要管理员权限可能不需要，用户级别的注册表项即可
- 程序路径变更后需要重新设置自启动

## 配置文件

所有配置保存在 QSettings 中：
- **组织名称**：`AutoBrightnessWidget`
- **应用名称**：`Settings`
- **Windows 存储位置**：注册表 `HKEY_CURRENT_USER\Software\AutoBrightnessWidget\Settings`

保存的配置项包括：
- `minCameraBrightness`：最小摄像头亮度
- `maxCameraBrightness`：最大摄像头亮度
- `minScreenBrightness`：最小屏幕亮度
- `maxScreenBrightness`：最大屏幕亮度
- `useCurve`：是否使用曲线
- `curvePoints`：曲线点数组
- `exposure`：曝光值
- `captureInterval`：拍照间隔
- `samplePoints`：采样点数
- `minimizeToTray`：是否最小化到托盘

## 技术实现要点

### 信号槽连接
```cpp
// 在 MainWindow 构造函数中
connect(m_autoBrightness, &AutoBrightness::cameraBrightnessChanged,
        this, &MainWindow::updateCameraBrightness);
connect(m_autoBrightness, &AutoBrightness::screenBrightnessChanged,
        this, &MainWindow::updateScreenBrightness);
```

### 线程安全
- `AutoBrightness::update()` 在工作线程中调用
- 通过信号槽机制跨线程通信（Qt 自动处理）
- UI 更新在主线程中执行

### 内存管理
- 使用 Qt 父子对象关系管理内存
- `AutoBrightness` 使用单例模式
- 托盘图标和菜单在 MainWindow 析构时清理

## 已知问题和限制

1. **曲线编辑界面未实现**：只能通过代码设置曲线点
2. **跨平台支持**：开机自启动仅支持 Windows
3. **托盘图标**：使用默认应用图标，可能需要设计专用托盘图标
4. **配置验证**：没有检查最小值是否小于最大值的逻辑

## 未来改进建议

1. 添加可视化曲线编辑器
2. 支持 Linux/macOS 的开机自启动
3. 设计专用的系统托盘图标
4. 添加配置导入/导出功能
5. 添加亮度变化动画（平滑过渡）
6. 支持多显示器独立控制
7. 添加日志查看功能
8. 支持快捷键控制
