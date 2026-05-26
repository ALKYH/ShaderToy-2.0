# 源码目录说明

当前阶段先提供最小入口程序，后续建议按模块逐步拆分：

```text
src/
├─ core/
├─ render/
├─ scene/
├─ ui/
└─ main.cpp
```

## 建议后续模块

- `core/`
  - 应用生命周期
  - 窗口初始化
  - 输入管理

- `render/`
  - Shader
  - Texture
  - Mesh
  - Model
  - Light

- `scene/`
  - 场景对象
  - 变换管理
  - 相机

- `ui/`
  - ImGui 面板
  - 调试与参数控制
