# ShaderToy-2.0

基于 `OpenGL Core Profile + C++ + GLFW/GLAD + ImGui` 的室内三维场景课程项目。

## 当前状态

当前仓库已完成最小工程骨架：

- CMake 工程
- GLFW 窗口与 OpenGL 上下文
- GLAD OpenGL 函数加载
- ImGui 调试面板
- UTF-8 编辑规范

## 构建方式

```powershell
cmake -S . -B build -G "MinGW Makefiles"
cmake --build build
```

运行：

```powershell
.\build\shader_toy_app.exe
```

## 后续推进方向


