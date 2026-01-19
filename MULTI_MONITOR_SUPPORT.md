# 多显示器支持功能说明

## 概述

AutoBrightnessWidget 现在支持为多个显示器配置不同的亮度曲线。所有显示器共享同一个摄像头输入，但可以各自应用不同的亮度映射策略。

## 功能特性

### 1. 显示器枚举
- 自动检测所有连接的支持亮度调节的显示器
- 显示显示器友好名称和索引
- 支持动态刷新显示器列表

### 2. 独立配置
每个显示器可以独立配置：
- **最小亮度映射**：当摄像头检测到特定亮度时，显示器应该设置的最小亮度
- **最大亮度映射**：当摄像头检测到特定亮度时，显示器应该设置的最大亮度
- **亮度曲线**：自定义的亮度映射曲线，实现非线性的亮度调整

### 3. 配置持久化
- 所有显示器的配置会自动保存到系统设置中
- 重启应用程序后，配置会自动恢复
- 支持导入和导出配置（待实现）

## 使用方法

### 打开多显示器配置对话框

1. 启动 AutoBrightnessWidget
2. 点击主窗口中的"多显示器配置..."按钮
3. 配置对话框将打开并显示所有检测到的显示器

### 配置单个显示器

1. 在显示器列表中选择要配置的显示器
2. 配置最小亮度映射：
   - 调整环境光到目标亮度（例如：很暗的环境）
   - 点击"设置最小亮度"，当前摄像头亮度会被记录
   - 设置对应的屏幕亮度值（例如：10%）
3. 配置最大亮度映射：
   - 调整环境光到目标亮度（例如：很亮的环境）
   - 点击"设置最大亮度"，当前摄像头亮度会被记录
   - 设置对应的屏幕亮度值（例如：100%）
4. （可选）使用自定义曲线：
   - 勾选"使用自定义曲线"
   - 点击"编辑曲线"按钮
   - 在曲线编辑器中调整控制点
5. 点击"应用"保存配置

### 应用配置

配置完成后：
1. 点击"确定"保存并关闭对话框
2. 点击主窗口中的"Start"按钮启动自动亮度调节
3. 每个显示器将根据其配置独立调整亮度

## 技术实现

### 核心架构

```
AutoBrightness (核心类)
├── 显示器枚举 (refreshMonitors)
├── 多显示器配置 (MonitorConfig)
├── 独立亮度映射 (mapBrightnessForMonitor)
└── 配置持久化 (saveSettings/loadSettings)

MonitorConfigDialog (UI)
├── 显示器列表
├── 配置面板
├── 曲线编辑器集成
└── 配置验证
```

### Windows WMI 集成

使用 PowerShell 与 Windows Management Instrumentation (WMI) 交互：

```powershell
# 获取所有显示器
$monitors = Get-WmiObject -Namespace root/wmi -Class WmiMonitorBrightnessMethods

# 为特定显示器设置亮度
$monitors[0].WmiSetBrightness(1, 50)  # 设置第一个显示器亮度为 50%
```

### 数据结构

#### MonitorInfo
```cpp
struct MonitorInfo {
    QString instanceName;    // WMI 实例名称
    QString friendlyName;    // 显示器友好名称
    int index;               // 显示器索引
};
```

#### MonitorConfig
```cpp
struct MonitorConfig {
    int minCameraBrightness;  // 最小摄像头亮度
    int maxCameraBrightness;  // 最大摄像头亮度
    int minScreenBrightness;  // 最小屏幕亮度
    int maxScreenBrightness;  // 最大屏幕亮度
    bool useCurve;            // 是否使用曲线
    std::vector<std::pair<int, int>> curvePoints;  // 曲线控制点
};
```

## 向后兼容性

为了保持与旧版本的兼容性：
- 传统的单显示器 API 仍然可用
- 旧配置会自动迁移到新的多显示器配置系统
- 如果只检测到一个显示器，界面行为与旧版本相同

## 限制和注意事项

1. **平台限制**：目前仅支持 Windows 平台（使用 WMI）
2. **显示器支持**：只有支持 DDC/CI 协议的显示器才能调节亮度
3. **性能考虑**：多显示器配置会略微增加 CPU 使用率
4. **同步延迟**：各显示器的亮度调整是顺序执行的，可能有轻微延迟

## 故障排除

### 显示器未被检测到
- 确保显示器支持亮度调节（DDC/CI）
- 检查显示器连接是否正常
- 点击"刷新显示器列表"按钮重新扫描

### 亮度调节无效
- 确认显示器支持软件亮度控制
- 检查 Windows 显示设置中是否启用了亮度控制
- 尝试重启应用程序

### 配置丢失
- 检查应用程序是否有写入注册表的权限
- 确认 QSettings 配置路径正常
- 手动备份配置文件（位于注册表）

## 未来改进

- [ ] 支持显示器分组
- [ ] 添加配置导入/导出功能
- [ ] 支持跨平台（Linux/macOS）
- [ ] 添加显示器预览功能
- [ ] 支持基于时间的配置切换
- [ ] 添加配置模板功能

## 相关 API 文档

### AutoBrightness 类新增方法

```cpp
// 显示器枚举
void refreshMonitors();
std::vector<MonitorInfo> getMonitors() const;
int getMonitorCount() const;
MonitorInfo getMonitorInfo(int index) const;

// 显示器配置
void setMonitorMinBrightness(int monitorIndex, int cameraBrightness, int screenBrightness);
void setMonitorMaxBrightness(int monitorIndex, int cameraBrightness, int screenBrightness);
void setMonitorCurvePoints(int monitorIndex, const std::vector<std::pair<int, int>>& points);
void setMonitorUseCurve(int monitorIndex, bool use);
MonitorConfig getMonitorConfig(int monitorIndex) const;

// 配置验证
bool validateMonitorConfiguration(int monitorIndex, QString* errorMessage = nullptr) const;

// 获取当前状态
int getMonitorCurrentScreenBrightness(int monitorIndex) const;
```

## 贡献

欢迎为多显示器功能贡献代码或报告问题：
- 提交 Issue：描述问题或功能请求
- 提交 Pull Request：实现新功能或修复 Bug
- 完善文档：帮助改进用户手册

## 许可证

与主项目保持一致的开源许可证。
