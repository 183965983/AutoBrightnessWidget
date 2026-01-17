# Copilot 使用说明 —— AutoBrightnessWidget 项目
本文件用于指导 Copilot 在 AutoBrightnessWidget 项目中生成**符合项目约定**的代码和评审意见。  
所有回答默认使用 **简体中文**，并以**专业、简洁**为主（可以适当解释理由，但避免无关聊天）。
---
## 1. 项目概览
- 项目名称：**AutoBrightnessWidget**
- 技术栈：**C++17 + Qt（Widgets）+ OpenCV + CMake**
- 核心功能：基于摄像头/图像输入的自动亮度、对比度等调节控件（Widget），并与现有 Qt UI 集成。
- 目标平台：桌面（Windows / Linux / macOS），使用 Qt 的事件循环和信号槽机制。
当你补全代码或做代码评审时，应优先：
- 遵守本文档中的 **C++ 编码规范、Qt 使用模式、OpenCV 集成方式和 CMake 约定**。
- 保持现有接口兼容，除非用户明确要求重构或修改 API。
---
## 2. C++17 编码规范
1. **语言标准**
   - 默认使用 **C++17**。避免使用 C++20+ 特性（`concepts`、`ranges` 等），除非项目 CMake 已明确启用。
2. **头文件与命名空间**
   - 头文件使用 `#pragma once` 或传统 include guard（项目已有哪种就保持一致）。
   - 尽量减少在头文件中 `#include` 的数量，能用前向声明就用前向声明。
   - 避免在头文件中使用 `using namespace`；在源文件中也尽量少用，提倡全名或局部 `using`。
3. **内存与资源管理**
   - 优先使用 RAII 和智能指针：
     - 使用 `std::unique_ptr` 表示唯一所有权；
     - 使用 `std::shared_ptr` 仅在确有共享所有权需求时；
     - 在 Qt 类型中，优先利用父子关系（`QObject` parent），不要随意用 `new` + 原始指针。
   - 不要手写 `new/delete` 管理大型资源/容器。若必须使用，需有清晰的所有权说明和释放路径。
4. **现代 C++ 特性使用约定**
   - `auto`：用于冗长类型或显而易见的初始化，避免降低可读性（不要在公共接口中返回 `auto` 除非是 trailing return）。
   - `constexpr`：可用于编译期常量、轻量算法，以提高性能。
   - `enum class`：优先使用强类型枚举替代旧式 `enum`。
   - `std::optional`：用于可缺省返回值，避免使用魔法值（如返回 -1、空指针等）。
   - `std::array` 和 `std::vector`：优先使用标准容器代替裸数组。
5. **代码风格（可类比 Google C++ / Qt 风格）**
   - 缩进：4 空格（除非项目现有文件明显使用其它缩进，则保持一致）。
   - 括号：左花括号与控制语句同行：
     ```cpp
     if (condition) {
         // ...
     }
     ```
   - 避免过长函数：逻辑复杂时拆分为小函数，特别是图像处理和 UI 逻辑，要保证可测试性和可读性。
---
## 3. Qt 使用规范
1. **对象生命周期**
   - Qt 对象继承自 `QObject` 时，优先使用 **父子关系管理生命周期**：
     - 将子对象构造时传入 parent 指针；
     - 不必对有 parent 的 `QObject` 手动 `delete`。
   - 需要在 C++ 世界长期持有的 Qt 对象指针：
     - 若存在不确定生命周期的交叉引用，可使用 `QPointer` 以避免悬空指针。
2. **信号和槽**
   - 优先使用 **新语法**：
     ```cpp
     connect(sender, &SenderType::signalName,
             receiver, &ReceiverType::slotName);
     ```
   - Lambda 槽函数中，尽量捕获必要变量，避免大范围 `[=]`；对于复杂逻辑，提取成成员函数槽。
3. **线程和 UI**
   - 所有 UI 更新必须在 **主线程** 执行。
   - 如果需要在后台线程中进行图像处理（如 OpenCV 运算），请：
     - 使用 `QThread` 或 `QtConcurrent`；
     - 在处理完成后通过信号槽通知主线程更新 UI。
4. **Qt 类型使用**
   - 文本使用 `QString`，与标准库交互时再进行转换（`toStdString()` / `fromStdString()`）。
   - 避免在核心逻辑模块中硬编码 UI 文本，尽量通过资源或翻译机制处理。
---
## 4. OpenCV 集成规范
1. **基础约定**
   - 核心图像处理逻辑使用 OpenCV (`cv::Mat`) 实现。
   - 注意区分颜色空间：`BGR` vs `RGB`。Qt 通常使用 `RGB`，OpenCV 默认 `BGR`。
2. **与 Qt 图像类型互转**
   - 在 `cv::Mat` 与 `QImage`/`QPixmap` 间转换时：
     - 明确指定像素格式（如 `QImage::Format_RGB888`、`Format_Grayscale8` 等）。
     - 避免多余拷贝：在可行时使用共享缓冲区，但要确保生命周期安全。
   - 需要频繁更新 UI 图像时，尽量将转换逻辑封装到专门的工具函数中，避免重复代码。
3. **性能与内存**
   - 避免在循环中频繁分配大尺寸 `cv::Mat`。可复用缓冲或使用 in-place 操作。
   - 算法复杂度较高时，应在注释中说明时间复杂度和可能的瓶颈，并在需要时进行简单基准测试。
---
## 5. 命名规范与文件组织
1. **命名规则（如无现有统一规范时按此执行）**
   - 类名：`PascalCase`，如 `AutoBrightnessWidget`, `ImageProcessor`.
   - 成员变量：`m_` 前缀 + `camelCase`，如 `m_currentBrightness`。
   - 普通函数与本地变量：`camelCase`，如 `updateBrightness()`, `targetValue`。
   - 常量：全大写 + 下划线，如 `kDefaultExposure`, `MAX_BRIGHTNESS`（视现有风格选择其一并保持一致）。
2. **文件组织**
   - 一个类通常对应一个 `.h` + 一个 `.cpp`。
   - UI 相关类（Widget、Dialog）放在 UI 模块或相应子目录中。
   - 纯逻辑/算法类（如图像处理）应与 UI 分离，以便单元测试。
---
## 6. 错误处理与日志
1. **错误处理策略**
   - 对于可预期的业务错误（如非法参数、图像为空、相机未连接）：
     - 注意返回错误码、`bool`、`std::optional` 或自定义结果类型（例如 `Result<T>`）。
     - UI 层可以将错误转化为对用户友好的提示。
   - 对于严重的编程错误（空指针解引用、越界）：
     - 通过 `Q_ASSERT` / `assert` 或早期返回防御式编程。
   - 项目若已有统一错误处理模式，请沿用同样方式；新增代码不要引入不一致风格。
2. **日志**
   - Qt 代码中优先使用 `qDebug()`, `qWarning()`, `qCritical()`。
   - 日志信息应包括上下文（函数名、关键参数），但避免输出过于冗长的调试信息到生产环境。
---
## 7. CMake 配置规范
1. **基础设置**
   - CMake 最低版本应与项目现有 `CMakeLists.txt` 保持一致。
   - 确保设置：
     ```cmake
     set(CMAKE_CXX_STANDARD 17)
     set(CMAKE_CXX_STANDARD_REQUIRED ON)
     set(CMAKE_CXX_EXTENSIONS OFF)
     ```
2. **目标与依赖**
   - 每个主要组件/库使用 `add_library` 或 `add_executable` 定义单独目标。
   - 使用 `target_include_directories`、`target_link_libraries` 为目标添加依赖。
   - 与 Qt 集成时，使用 `target_link_libraries(target PRIVATE Qt::Widgets Qt::Core ...)` 等现代写法。
   - 与 OpenCV 集成时，确保在顶层或相应模块中使用 `find_package(OpenCV REQUIRED)`，并链接 `OpenCV::opencv_core` 等目标（以项目现有用法为准）。
3. **构建选项**
   - 若需要引入新选项（如开启/关闭某些算法），使用 `option(AUTOBRIGHTNESS_ENABLE_X "desc" ON)` 风格，并提供合理默认值。
---
## 8. 文档与注释要求
1. **接口文档**
   - 公共类和对外暴露的函数应在头文件中编写简明注释，说明：
     - 功能；
     - 重要参数及取值范围；
     - 返回值含义；
     - 可能的失败情况（如返回 `std::optional` 为空的语义）。
2. **实现注释**
   - 对于复杂的图像处理算法，请在实现文件中描述：
     - 算法思路；
     - 关键步骤；
     - 需要注意的边界情况。
   - 避免“翻译代码”的注释；注释应解释 **为什么** 这样做，而不是简单重复 **做了什么**。
---
## 9. 测试与覆盖率期望
1. **测试类型**
   - 尽量为核心算法/逻辑（不依赖 UI 的部分）编写单元测试。
   - 对涉及 OpenCV 图像处理的函数，可使用小尺寸测试图像（或合成图像）验证关键行为（如亮度调整是否在合理范围内）。
2. **覆盖率**
   - 优先保证：
     - 自动亮度/对比度计算逻辑；
     - 图像转换工具函数（`cv::Mat` <-> `QImage`）；
     - 参数边界情况（最小/最大亮度、空图像等）；
     具有较高测试覆盖率。
3. **CMake 与测试**
   - 若项目使用 `CTest` 或其它测试框架，新增测试时：
     - 将测试目标添加到相应的 `CMakeLists.txt`；
     - 确保在 `BUILD_TESTING` 或类似选项打开时编译并可执行。
---
## 10. 生成代码时的优先级
当你为本项目生成代码、重构建议或评审意见时，请优先：
1. 遵守上述 **C++17/Qt/OpenCV/CMake** 规范和命名约定。
2. 保持现有模块边界，避免随意打破 UI 与逻辑的分层。
3. 关注可测试性：将复杂逻辑从 UI 抽取到可单测的类/函数中。
4. 在不确定项目现有约定的细节时，选择更安全、更易维护的默认方案，并在注释或说明中显式指出假设前提。
以上规范是面向 Copilot 的技术约束，回答时不必重复解释这些规范本身，只需在代码和建议中自觉遵守。
