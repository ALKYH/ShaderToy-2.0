# ShaderToy-2.0

![C++](https://img.shields.io/badge/C%2B%2B-20-blue.svg)
![OpenGL](https://img.shields.io/badge/OpenGL-3.3%20Core-5586A4.svg)
![CMake](https://img.shields.io/badge/CMake-3.24%2B-green.svg)
![Platform](https://img.shields.io/badge/Platform-Windows-lightgrey.svg)
![Status](https://img.shields.io/badge/Status-Course%20Project-orange.svg)

基于 `OpenGL Core Profile + C++ + GLFW/GLAD + ImGui` 的室内三维场景图形学课程项目。项目不依赖 Unity、Unreal Engine、Godot 等成熟引擎，重点实现了模型加载、材质解析、实时光照、阴影映射、第一人称相机、碰撞检测、刚体动画、粒子系统以及外部场景配置驱动。

## 项目特色

- `OBJ + MTL + map_Kd` 资源链路，支持直接从指定模型文件读取材质与贴图
- 支持方向光、多个点光源、聚光灯和方向光阴影映射
- 第一人称漫游相机，支持鼠标捕获切换、上下移动、Shift 加速
- `ImGui` 调试面板，按选项卡组织场景、光照、特效和调试信息
- 支持碰撞检测、刚体动画、粒子系统等课程增强项
- 通过外部配置文件管理多模型、多障碍物、多点光源和多粒子发射器

## 项目结构

```text
ShaderToy-2.0/
├─ assets/
│  ├─ config/      # 场景配置文件
│  ├─ models/      # OBJ / MTL / 贴图资源
│  └─ shaders/     # GLSL 着色器
├─ docs/           # 实验报告、PPT、大纲与场景编辑文档
├─ src/
│  ├─ render/      # 模型、纹理、shader、网格等渲染相关模块
│  └─ scene/       # 相机、碰撞、粒子、动画、场景配置等模块
├─ tests/          # 资源解析、配置读取、逻辑行为测试
├─ CMakeLists.txt
└─ README.md
```

## 当前已实现能力

### 渲染与资源
- Shader 文件读取与编译/链接
- OBJ 模型解析
- MTL 材质读取
- 图片纹理解码与上传
- 多模型实例配置读取

### 光照与阴影
- 方向光
- 多个点光源
- 聚光灯
- 方向光阴影映射
- 阴影开关与偏移调节

### 交互与增强
- 第一人称相机
- 碰撞检测
- 刚体动画
- 粒子系统
- ImGui 运行时参数调节

### 配置驱动
- 多模型
- 多障碍物
- 多点光源
- 多粒子发射器

## 编译环境要求

必须说明如下：

### 操作系统
- Windows 10 / Windows 11

### 编译工具链
- `CMake 3.24` 或更高版本
- `MinGW-w64 g++`，已验证环境为 `g++ 13.1.0`

### 图形环境
- 支持 `OpenGL 3.3 Core Profile` 的显卡与驱动

### 开发建议
- 推荐使用 CLion、Visual Studio Code 或 Visual Studio 打开项目
- 若使用 VS Code，建议安装 CMake、C/C++、Git 相关扩展

## 第三方库依赖列表

项目通过 `CMake FetchContent` 自动拉取以下依赖：

- `GLFW 3.4`
  - 窗口管理与输入处理
- `GLAD 2.0.8`
  - OpenGL 函数加载
- `ImGui 1.91.9b`
  - 实时调试界面
- `GLM 1.0.1`
  - 数学库，负责矩阵与向量计算
- `stb_image`
  - 图片解码，用于纹理加载

不需要手动把这些库拷贝进仓库，首次配置时会自动下载。

## 构建与运行

### 1. 配置工程

```powershell
cmake -S . -B build -G "MinGW Makefiles"
```

### 2. 编译主程序

```powershell
cmake --build build --target shader_toy_app
```

### 3. 运行程序

```powershell
.\build\shader_toy_app.exe
```

## 测试命令

可以按模块单独验证：

```powershell
cmake --build build --target camera_tests
.\build\camera_tests.exe

cmake --build build --target resource_tests
.\build\resource_tests.exe

cmake --build build --target obj_loader_tests
.\build\obj_loader_tests.exe

cmake --build build --target scene_logic_tests
.\build\scene_logic_tests.exe

cmake --build build --target scene_config_tests
.\build\scene_config_tests.exe
```

## 操作按键说明

必须说明如下：

### 相机移动
- `W`：前进
- `S`：后退
- `A`：左移
- `D`：右移
- `Q`：向下移动
- `E`：向上移动

### 视角控制
- `鼠标移动`：旋转视角
- `Tab`：切换鼠标捕获/释放
  - 捕获时用于控制视角
  - 释放时可直接操作左侧 UI

### 速度控制
- `Left Shift`：加速移动

### 退出
- `ESC`：关闭程序

## 场景配置与编辑

默认场景配置文件：

- [assets/config/default_scene.cfg](/E:/Projects/ShaderToy-2.0/assets/config/default_scene.cfg)

场景编辑与配置规则说明：

- [docs/scene_config_contract.md](/E:/Projects/ShaderToy-2.0/docs/scene_config_contract.md)

当前配置支持：

- 多模型实例：`model.<name>`
- 多障碍物：`collision.obstacle.<name>`
- 多点光源：`lighting.point.<name>`
- 多粒子发射器：`particles.<name>`

地编可以直接通过编辑配置文件完成：

- 模型路径替换
- 模型位置与旋转动画参数调整
- 房间与障碍物碰撞范围调整
- 点光源摆放与配色
- 粒子发射器数量、颜色、范围与速度调整

## 文档入口

项目内已经附带课程文档骨架：

- [实验报告正文骨架](./docs/course_report.md)
- [实验报告纲要](./docs/experiment_report_outline.md)
- [答辩 PPT 大纲](./docs/ppt_outline.md)
- [项目构建计划](./docs/project_build_plan.md)
- [任务看板](./docs/task_board.md)

## 注意事项

- 终端中若出现中文乱码，通常是 PowerShell 控制台编码问题，不代表文件内容损坏
- MinGW 在重复增量构建静态库时，偶尔可能出现一次 `ar/ranlib file truncated` 的瞬时错误，重跑目标通常即可恢复
- 当前项目虽然已经支持多模型、多障碍物和多粒子发射器配置，但仍建议先用小规模场景联调，再逐步扩展复杂度

## 后续建议

- 继续扩展真正的多模型批量场景资源
- 增加配置保存回文件能力
- 将多个点光源和多个模型的编辑入口进一步做成可视化调试工作流
- 补充真实场景截图与运行截图到 README 中，形成更完整的项目展示页

