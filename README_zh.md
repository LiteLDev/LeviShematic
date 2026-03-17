# LeviShematic

**Bedrock Edition 投影模组** — 基于 [LeviLamina](https://github.com/LiteLDev/LeviLamina) 的 Litematica 风格建筑投影工具

将 Java Edition 的 [Litematica](https://github.com/maruohon/litematica) 模组核心功能移植到 Minecraft Bedrock Edition，支持加载 `.litematic` 格式的建筑蓝图，在游戏世界中以半透明虚影方块叠加渲染投影，帮助玩家精确还原建筑。

---

## ✨ 功能特性

### 已实现 ✅

- **`.litematic` 文件完整解析** — 自定义 Big-Endian NBT 解析器（支持 `TAG_Long_Array`），GZip 解压 + 递归 NBT 解析
- **Java → BE 方块映射** — 内置 150+ 条 Java blockstate 到 Bedrock `Block*` 的差异映射表
- **投影虚影渲染** — 通过 Hook `RenderChunkBuilder::_sortBlocks` 和 `BlockTessellator::tessellateInWorld`，将投影方块以半透明颜色叠加渲染到游戏世界
- **投影放置管理** — 支持多个投影同时加载，独立控制启用/隐藏
- **旋转与镜像** — 支持 CW90°/CCW90°/180° 旋转和 X/Z 轴镜像变换
- **子区域支持** — 每个子区域可独立设置偏移、旋转、镜像
- **完整命令系统** — 13+ 个命令/子命令覆盖加载、移动、旋转、镜像、列表、删除等操作
- **无锁渲染架构** — 基于 `atomic shared_ptr` 的 `ProjectionState` 快照机制，渲染线程 lock-free 读取

### 开发中 🚧

- **选区系统** — `AreaSelection` + `Box` 圈选世界区域，用于保存建筑为 Schematic
- **建筑验证器** — `SchematicVerifier` 逐 Chunk 对比投影与真实世界，高亮错误/缺失/多余方块
- **材料表统计** — 统计投影所需方块数量，支持与玩家背包对比
- **GUI 界面** — 基于 LL GUI API 的图形化操作界面
- **热键绑定** — 键盘快捷键操控投影（移动、旋转、开关渲染等）

---

## 🏗️ 架构概览

```
┌─────────────────────────────────────────────────────────────────┐
│                      LeviShematic                               │
│                                                                 │
│  ┌──────────────┐    ┌──────────────┐    ┌──────────────────┐   │
│  │  命令系统     │    │  输入系统     │    │   GUI系统        │   │
│  │  /schem ...  │    │  热键处理     │    │  (GUI API)       │   │
│  └──────┬───────┘    └──────┬───────┘    └──────────────────┘   │
│         │                   │                                   │
│  ┌──────▼───────────────────▼──────────────────────────────┐    │
│  │                   DataManager（核心单例）                │    │
│  │  SelectionManager | PlacementManager | SchematicHolder  │    │
│  └──┬────────────┬──────────────┬─────────────┬────────────┘    │
│     │            │              │             │                 │
│  ┌──▼───┐  ┌─────▼───┐   ┌──────▼───┐  ┌──────▼───┐             │
│  │选区层 │ │Schematic │   │  渲染层  │  │ 验证层    │             │
│  │      │  │  格式层  │   │          │  │          │             │
│  └──────┘  └─────────┘   └──────────┘  └──────────┘             │
│                                                                 │
│  ┌─────────────────────────────────────────────────────────┐    │
│  │               底层Hook层（渲染注入）                      │    │
│  │  RenderChunkBuilder Hook | BlockTessellator Hook        │    │
│  └─────────────────────────────────────────────────────────┘    │
└─────────────────────────────────────────────────────────────────┘
```

### 核心数据流

```
.litematic 文件 → GZip解压 → NBT解析 → LitematicSchematic
                                              ↓
                                    SchematicPlacement（坐标 + 旋转 + 镜像）
                                              ↓
                                    PlacementManager::rebuildProjection()
                                              ↓
                                    ProjectionState（atomic快照更新）
                                              ↓
                                ┌─────────────┴───────────────┐
                                ↓                             ↓
                    RenderChunkBuilder Hook          BlockTessellator Hook
                   （注入方块到BLEND队列）          （半透明颜色覆盖）
                                ↓
                        投影虚影渲染到屏幕
```

---

## 📁 项目结构

```
src/levishematic/
├── LeviShematic.h/cpp            ← 模组入口（LL 注册）
├── core/
│   └── DataManager.h/cpp         ← 全局状态中枢单例
├── schematic/
│   ├── LitematicSchematic.h/cpp  ← .litematic 完整解析（自定义 NBT 解析器）
│   ├── SchematicMetadata.h       ← 元数据（名称/作者/时间戳）
│   ├── BlockStateContainer.h/cpp ← 紧密位压缩方块存储解包
│   ├── BlockStatePalette.h/cpp   ← Java blockstate → BE Block* 映射
│   ├── NBTReader.h/cpp           ← GZip + NBT 文件读写
│   ├── SchematicTransform.h/cpp  ← 旋转/镜像坐标变换
│   └── placement/
│       ├── SchematicPlacement.h/cpp  ← 投影放置实例
│       └── PlacementManager.h/cpp    ← 多投影管理器
├── render/
│   ├── ProjectionRenderer.h/cpp  ← 投影状态 & 快照（lock-free）
│   ├── WorldSchematic.h/cpp      ← 虚拟世界方块缓存
│   └── ChunkRenderCache.h/cpp    ← SubChunk 渲染缓存
├── hook/
│   ├── RenderHook.h/cpp          ← 渲染管线 Hook（方块注入 + 颜色控制）
│   └── TickHook.h/cpp            ← 客户端 Tick Hook（初始化 + 调度）
├── command/
│   └── Command.h/cpp             ← 完整命令系统（13+ 子命令）
├── input/
│   └── InputHandler.h/cpp        ← 输入/热键处理（接口预留）
├── selection/
│   ├── AreaSelection.h/cpp       ← 区域选择
│   ├── Box.h/cpp                 ← 选区盒子
│   └── SelectionManager.h/cpp    ← 选区管理器
├── verifier/
│   └── SchematicVerifier.h/cpp   ← 建筑验证器
├── util/
│   ├── BlockUtils.h/cpp          ← Java→BE 方块名映射（150+ 条）
│   └── PositionUtils.h           ← SubChunk 坐标编码工具
├── materials/                    ← 材料表（待实现）
├── scheduler/                    ← 异步任务调度（待实现）
└── gui/screens/                  ← GUI 界面（待实现）
```

---

## 📋 命令列表

所有命令以 `/schem` 为前缀：

| 命令 | 说明 | 示例 |
|------|------|------|
| `/schem load <文件> [x y z]` | 加载 Schematic 并放置到指定/玩家位置 | `/schem load house.litematic 0 64 0` |
| `/schem move <dx> <dy> <dz>` | 移动当前选中投影 | `/schem move 10 0 5` |
| `/schem origin <x> <y> <z>` | 设置投影原点坐标 | `/schem origin 100 64 200` |
| `/schem rotate <方向>` | 旋转当前投影（cw90/ccw90/r180） | `/schem rotate cw90` |
| `/schem mirror <轴>` | 镜像当前投影（x/z/none） | `/schem mirror x` |
| `/schem list` | 列出所有已加载投影 | `/schem list` |
| `/schem select <id>` | 选中指定 ID 的投影 | `/schem select 2` |
| `/schem remove [id]` | 删除指定/当前选中投影 | `/schem remove` |
| `/schem toggle [render]` | 开关投影启用/渲染可见性 | `/schem toggle render` |
| `/schem info` | 显示当前投影详细信息 | `/schem info` |
| `/schem files` | 列出可用的 .litematic 文件 | `/schem files` |
| `/schem reset` | 重置当前投影变换 | `/schem reset` |
| `/schem clear` | 清空所有投影 | `/schem clear` |

---

## 🔧 构建指南

### 环境要求

- **操作系统**: Windows (x64)
- **编译器**: MSVC (Visual Studio 2022+)
- **构建工具**: [xmake](https://xmake.io/)
- **依赖项**: 
  - [LeviLamina](https://github.com/LiteLDev/LeviLamina) — BDS 模组加载器
  - [zlib 1.3.2](https://zlib.net/) — GZip 解压缩

### 构建步骤

```bash
# 1. 克隆仓库
git clone <repo-url>
cd LeviShematic

# 2. 配置项目（客户端模组）
xmake f -y -p windows -a x64 -m release --target_type=client

# 3. 构建
xmake

# 构建产物位于 bin/ 目录
```

> **注意**: 本项目为 **客户端模组**（Client Mod），需要配合支持客户端 Mod 加载的 LeviLamina 使用。

### 安装

将 `bin/` 目录下的构建产物复制到 LeviLamina 的 `mods/` 目录中。同时在 `schematics/` 目录下放置 `.litematic` 文件即可使用 `/schem load` 命令加载。

---

## 🗺️ 开发路线

| 阶段 | 内容 | 状态 |
|------|------|------|
| Phase 1 | 核心渲染 Hook（ProjectionState / RenderChunkBuilder / BlockTessellator） | ✅ 已完成 |
| Phase 2 | Schematic 加载（NBT 解析 / BlockStatePalette / 方块映射） | ✅ 已完成 |
| Phase 3 | 放置管理（SchematicPlacement / PlacementManager / 命令系统） | ✅ 已完成 |
| Phase 4 | 选区与保存（AreaSelection / 鼠标选区 Hook / Schematic 写入） | 🚧 开发中 |
| Phase 5 | 验证器（WorldSchematic 缓存 / 逐 Chunk 对比 / 错误高亮） | 📋 计划中 |
| Phase 6 | 材料表与 GUI（MaterialList / LL GUI 展示 / 配置持久化） | 📋 计划中 |

---

## 🔑 技术亮点

- **自定义 Big-Endian NBT 解析器** — LL 原生 API 不支持 `TAG_Long_Array`（type 12），项目实现了完整的自定义解析器以正确读取 Litematica 格式
- **无锁渲染线程安全** — 使用 `std::atomic<std::shared_ptr>` 实现 ProjectionState 快照的 lock-free 读取，渲染线程零等待
- **紧密位压缩解包** — 实现 Litematica 的紧密位压缩方块存储解包算法，支持跨 `long` 边界的位提取
- **Java→BE 方块状态转换** — 解析 Java Edition 的 blockstate 字符串（如 `minecraft:oak_log[axis=y]`），映射为 BE 的 `Block*` 对象

---

## 📄 许可证

[CC0-1.0](LICENSE) — 公共领域，自由使用。

---

## 🤝 贡献

欢迎提交 Issue 和 Pull Request！

如有问题或建议，请通过创建 Issue 进行讨论。

## 🤖 提示

本仓库内容均由AI生成
