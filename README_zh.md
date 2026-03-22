# LeviShematic

基于 [LeviLamina](https://github.com/LiteLDev/LeviLamina) 的 Bedrock Edition 投影模组。

LeviShematic 现在仅使用 `.mcstructure` 作为蓝图格式。它支持加载结构文件、在世界中渲染半透明投影视图，并把游戏内选区保存回 `.mcstructure`。

## 功能特性

- 基于 Bedrock `StructureTemplate` 的 `.mcstructure` 读取与保存
- 虚影方块投影渲染
- 投影移动、旋转、镜像、隐藏、启用/禁用
- 多投影管理
- 选区角点与线框渲染
- 基于 `/schem` 的命令工作流

## 命令列表

所有命令以 `/schem` 为前缀。

| 命令 | 说明 |
|------|------|
| `/schem load <file> [x y z]` | 加载 `.mcstructure` 投影 |
| `/schem move <dx> <dy> <dz>` | 移动当前投影 |
| `/schem origin <x y z>` | 设置投影原点 |
| `/schem rotate <cw90\|ccw90\|r180>` | 旋转当前投影 |
| `/schem mirror <x\|z\|none>` | 镜像当前投影 |
| `/schem list` | 列出所有投影 |
| `/schem select <id>` | 选中投影 |
| `/schem remove [id]` | 删除投影 |
| `/schem toggle [render]` | 开关投影启用或渲染可见性 |
| `/schem info` | 显示当前投影信息 |
| `/schem files` | 列出可用 `.mcstructure` 文件 |
| `/schem reset` | 重置变换 |
| `/schem clear` | 清空所有投影 |
| `/schem pos1 [x y z]` | 设置选区角点1 |
| `/schem pos2 [x y z]` | 设置选区角点2 |
| `/schem save <name>` | 保存选区为 `.mcstructure` |
| `/schem selection clear\|mode\|info` | 管理选区 |

## 项目结构

```
src/levishematic/
├── core/                         # 全局状态中枢
├── command/                      # 命令注册与处理
├── hook/                         # 渲染/Tick/选区 Hook
├── input/                        # 热键/输入层（预留）
├── render/                       # 投影状态与渲染辅助
├── schematic/placement/          # 投影加载与变换逻辑
├── selection/                    # 选区与 .mcstructure 保存
├── util/                         # 坐标/SubChunk 工具
└── verifier/                     # 验证器占位
```

## 构建

环境要求：

- Windows x64
- Visual Studio 2022+
- [xmake](https://xmake.io/)
- [LeviLamina](https://github.com/LiteLDev/LeviLamina)

构建步骤：

```bash
xmake f -y -p windows -a x64 -m release --target_type=client
xmake
```

将构建产物放入 LeviLamina 的 `mods/` 目录中。把 `.mcstructure` 文件放到 `schematics/` 目录后，即可通过 `/schem load` 加载。

## 当前状态

- Phase 1：核心渲染 Hook — 已完成
- Phase 2：`.mcstructure` 加载与放置 — 已完成
- Phase 3：放置管理与命令系统 — 已完成
- Phase 4：选区与保存 — 已完成
- Phase 5：验证器 — 计划中
- Phase 6：材料表与 GUI — 计划中

## 说明

当前某些环境下构建仍可能在最终链接阶段因为外部 Bedrock/Hook 符号而失败，但源码层面已经完全收敛到仅支持 `.mcstructure`。
