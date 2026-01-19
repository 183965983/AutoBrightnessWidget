# 自动发布功能说明 / Auto-Release Feature Documentation

## 概述 / Overview

本项目现在支持自动发布功能，每次代码合并到主分支（main/master）后，会自动构建并发布一个预发布版本。

This project now supports automatic releases. After each merge to the main branch (main/master), a pre-release version is automatically built and published.

## 功能特性 / Features

### 1. 自动触发 / Automatic Triggering
- **触发条件**: 代码推送到 `main` 或 `master` 分支
- **触发方式**: Git push 或 Pull Request 合并
- **手动触发**: 也支持在 GitHub Actions 页面手动触发

**Trigger conditions**:
- Automatically triggered on push to `main` or `master` branch
- Triggered by Git push or Pull Request merge
- Can also be manually triggered from GitHub Actions page

### 2. 两种打包形式 / Two Package Types

#### Windows 安装包 / Windows Installer
- **文件名**: `AutoBrightnessWidget-Setup-Windows-x64.exe`
- **特点**:
  - 完整的安装向导，支持中英文界面
  - 自动安装到 Program Files
  - 自动创建桌面快捷方式和开始菜单项
  - 支持一键卸载
  - 适合普通用户使用

**Features**:
- Full installation wizard with English and Chinese language support
- Auto-installs to Program Files
- Creates desktop shortcuts and start menu items
- One-click uninstall support
- Recommended for regular users

#### 便携版 / Portable Version
- **文件名**: `AutoBrightnessWidget-Portable-Windows-x64.zip`
- **特点**:
  - 免安装，解压即用
  - 可放在 U 盘或任意目录
  - 包含所有必需的运行库和依赖项
  - 适合需要便携使用的用户

**Features**:
- No installation required, extract and run
- Can be placed on USB drive or any directory
- Includes all required libraries and dependencies
- Perfect for portable use

### 3. 自动版本号生成 / Automatic Version Numbering

版本号格式: `v0.1.0-pre.YYYYMMDD.SHA`

例如: `v0.1.0-pre.20260119.abc1234`

- `0.1.0`: 基础版本号（从 CMakeLists.txt 读取）
- `pre`: 标记为预发布版本
- `YYYYMMDD`: 构建日期
- `SHA`: Git commit 短 SHA

**Version format**: `v0.1.0-pre.YYYYMMDD.SHA`

Example: `v0.1.0-pre.20260119.abc1234`

- `0.1.0`: Base version (read from CMakeLists.txt)
- `pre`: Marks as pre-release
- `YYYYMMDD`: Build date
- `SHA`: Git commit short SHA

### 4. 自动生成发布说明 / Automatic Release Notes

发布说明自动包含以下内容：

**Release notes automatically include**:
- 最新的 merge commit 信息
- 相关的 Pull Request 编号
- 下载说明和安装方式
- 系统要求
- 构建信息（时间、commit、分支）

## 工作流程 / Workflow

### 构建流程 / Build Process

```
┌─────────────┐
│   Merge PR  │
│  to main    │
└──────┬──────┘
       │
       ▼
┌─────────────────────────────────────┐
│  Auto-Release Workflow Triggered    │
└──────┬──────────────────────────────┘
       │
       ├──────────────┬──────────────┐
       ▼              ▼              ▼
   ┌───────┐    ┌─────────┐   ┌──────────┐
   │ Build │    │ Package │   │ Package  │
   │  Job  │    │Portable │   │Installer │
   └───┬───┘    └────┬────┘   └────┬─────┘
       │             │             │
       └─────────────┴─────────────┘
                     │
                     ▼
          ┌──────────────────┐
          │ Create Pre-release│
          │  on GitHub        │
          └──────────────────┘
```

### 各阶段说明 / Stage Descriptions

1. **Build Job**: 
   - 编译应用程序
   - 生成可执行文件和库文件
   - 上传构建产物

2. **Package Portable Job**:
   - 收集所有运行时依赖
   - 使用 windeployqt 部署 Qt 依赖
   - 复制 OpenCV DLL
   - 打包成 ZIP 文件

3. **Package Installer Job**:
   - 准备安装文件
   - 下载并安装 Inno Setup
   - 根据模板生成安装脚本
   - 编译生成安装程序

4. **Create Pre-release Job**:
   - 生成版本号
   - 从 Git 历史提取发布说明
   - 上传两种安装包
   - 创建 GitHub 预发布

## 使用方法 / Usage

### 对于用户 / For Users

1. 访问 [Releases 页面](https://github.com/183965983/AutoBrightnessWidget/releases)
2. 选择最新的预发布版本（标记为 "Pre-release"）
3. 根据需要下载安装包或便携版
4. 按照 README 中的说明安装或使用

### 对于开发者 / For Developers

#### 触发自动发布 / Trigger Auto-Release

将代码合并到主分支即可自动触发：

```bash
# 方式一：通过 Pull Request
git checkout main
git merge feature-branch
git push origin main

# 方式二：直接推送到主分支
git checkout main
git commit -m "Your changes"
git push origin main
```

#### 创建正式发布 / Create Stable Release

正式发布仍然使用原有的 `release.yml` 工作流，通过打标签触发：

```bash
git tag v0.1.0
git push origin v0.1.0
```

这将创建一个正式的稳定版本（不是预发布）。

## 配置文件 / Configuration Files

### 工作流文件 / Workflow Files

- `.github/workflows/auto-release.yml` - 自动发布工作流
- `.github/workflows/release.yml` - 手动发布工作流（通过标签触发）
- `.github/workflows/build.yml` - CI 构建工作流

### 安装脚本模板 / Installer Script Template

- `installer-script.iss.template` - Inno Setup 安装脚本模板
  - 包含占位符 `{{VERSION}}`
  - 在构建过程中会被替换为实际版本号

## 系统要求 / System Requirements

### 构建环境 / Build Environment
- Windows Server 2022 (GitHub Actions runner)
- Visual Studio 2022 (MSVC)
- Qt 6.5.0
- OpenCV 4.10.0
- Inno Setup 6

### 运行环境 / Runtime Environment
- Windows 10/11 (64位)
- 带摄像头的设备
- Visual C++ 运行库（安装包已包含所需 DLL）

## 故障排查 / Troubleshooting

### 工作流失败 / Workflow Failures

1. **构建失败**: 检查代码编译错误
2. **打包失败**: 检查依赖库路径配置
3. **安装包构建失败**: 检查 Inno Setup 安装和脚本语法
4. **发布失败**: 检查 GitHub token 权限

### 查看日志 / View Logs

访问 [Actions 页面](https://github.com/183965983/AutoBrightnessWidget/actions) 查看详细的构建日志。

## 未来改进 / Future Improvements

- [ ] 支持 Linux 打包（AppImage / DEB / RPM）
- [ ] 支持 macOS 打包（DMG / PKG）
- [ ] 添加自动更新检查功能
- [ ] 支持增量更新
- [ ] 添加代码签名
- [ ] 集成更多测试

## 许可证 / License

本功能遵循项目主许可证。

This feature follows the main project license.
