# 资源目录说明

本目录用于存放课程项目所需的模型、纹理和着色器资源。

## 建议结构

```text
assets/
├─ models/
│  ├─ room/
│  ├─ desk/
│  ├─ chair/
│  ├─ lamp/
│  └─ cabinet/
├─ textures/
└─ shaders/
```

## 协作规范

- 文本文件统一使用 `UTF-8`
- 模型优先使用 `OBJ`
- 贴图优先使用 `PNG`
- 文件名使用英文小写与下划线
- 单个模型建议独立子目录管理

## 单模型示例

```text
assets/models/chair/
├─ chair.obj
├─ chair.mtl
└─ chair_albedo.png
```
