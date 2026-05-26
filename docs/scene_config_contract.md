# 场景配置文件约定

本项目支持从外部配置文件读取以下内容：

- 多个模型实例
- 地面参数
- 房间碰撞边界
- 多个障碍物碰撞箱
- 方向光、点光源、聚光灯
- 阴影开关与偏移
- 多个粒子发射器

推荐配置文件路径：

```text
assets/config/default_scene.cfg
```

## 文件格式

采用简单的 `INI` 风格文本：

```ini
[section]
key = value
```

注释使用 `#` 开头。向量统一写成：

```ini
1.0, 2.0, 3.0
```

## 支持的 section 组织方式

### 多模型

```ini
[model.main]
obj_path = ../models/room/scene.obj
position = 0.0, 0.0, 0.0
rotation_axis = 0.0, 1.0, 0.0
rotation_speed = 60.0

[model.fan]
obj_path = ../models/props/fan.obj
position = 0.8, 0.3, -0.6
rotation_axis = 0.0, 1.0, 0.0
rotation_speed = 180.0
```

规则：
- `model.<name>` 表示一个模型实例
- `<name>` 只作为命名分组标识
- 当前解析顺序按文件出现顺序保留

### 地面

```ini
[ground]
y = -0.8
extent = 3.0
```

### 碰撞

```ini
[collision.room]
min = -2.8, -0.2, -2.8
max = 2.8, 1.8, 2.8

[collision.obstacle.center]
min = -0.5, -0.2, -0.5
max = 0.5, 1.0, 0.5

[collision.obstacle.side]
min = 1.0, -0.2, 0.2
max = 1.6, 0.8, 0.8
```

规则：
- `collision.room` 定义整个可移动空间
- `collision.obstacle.<name>` 定义多个障碍物包围盒

### 光照

```ini
[lighting.directional]
direction = -0.2, -1.0, -0.3
color = 1.0, 0.95, 0.85

[lighting.point]
position = 1.2, 0.8, 1.5
color = 0.3, 0.7, 1.0
linear = 0.14
quadratic = 0.07

[lighting.spot]
color = 1.0, 0.9, 0.8
inner_cutoff = 12.5
outer_cutoff = 20.0
```

### 阴影

```ini
[shadow]
enabled = true
bias = 0.005
```

### 多粒子发射器

```ini
[particles.fire]
count = 24
origin = 0.0, -0.55, 0.0
spread = 0.16, 0.0, 0.2
base_velocity = 0.0, 0.6, 0.0
lifetime = 1.5
point_size = 6.0
color = 1.0, 0.55, 0.15

[particles.smoke]
count = 12
origin = 0.6, -0.4, -0.2
spread = 0.1, 0.0, 0.1
base_velocity = 0.0, 0.3, 0.0
lifetime = 2.0
point_size = 4.0
color = 0.7, 0.7, 0.7
```

规则：
- `particles.<name>` 表示一个粒子发射器
- 每个发射器有自己的数量、起点、扩散、速度、寿命和颜色

## 地编实现要求

地编在交付资源时，至少需要提供：

```text
assets/
├─ config/
│  └─ default_scene.cfg
├─ models/
│  ├─ room/
│  │  ├─ scene.obj
│  │  ├─ scene.mtl
│  │  └─ <textures>
│  └─ props/
│     ├─ fan.obj
│     ├─ fan.mtl
│     └─ <textures>
```

要求如下：
- 每个 `model.<name>.obj_path` 指向一个真实模型文件
- 模型内部的 `mtllib` 与 `map_Kd` 路径必须有效
- 障碍物包围盒尽量与实际大件家具或墙体位置一致
- 粒子参数、光照参数、阴影参数优先通过配置调整

## 当前限制

- 当前渲染主循环还主要消费一个主模型；多模型配置接口已经铺好，后续可继续扩展为批量实例化渲染
- 当前碰撞是多个 AABB，但主程序还可以继续增强为更完整的逐物体碰撞系统
- 当前粒子接口已经支持多个发射器配置，主循环侧可继续扩为多发射器同时渲染

这套命名分组格式的目标是：对地编友好、易增量编辑、无需复杂 JSON 数组语法，也能自然扩展多对象场景
