#pragma once
// ============================================================
// Command.h
// 命令系统注册
//
// Phase 3 命令（已实现）：
//   /schem load <文件名> [x y z]     ← 加载 Schematic 并放置
//   /schem move <dx dy dz>           ← 移动当前投影
//   /schem rotate <cw90|ccw90|180>   ← 旋转
//   /schem mirror <x|z|none>         ← 镜像
//   /schem list                      ← 列出已加载投影
//   /schem remove [id]               ← 删除投影
//   /schem select <id>               ← 选中指定投影
//   /schem toggle [render]           ← 开关投影
//   /schem info                      ← 显示当前投影信息
//   /schem files                     ← 列出可用的 .litematic 文件
//   /schem clear                     ← 清空所有投影
//   /schem origin <x y z>            ← 设置投影原点
//   /schem reset                     ← 重置当前投影变换
//
// 保留的测试命令：
//   /rendert ...                     ← 测试渲染（临时）
//
// Phase 4+ 命令（预留接口）：
//   /schem verify / verify stop
//   /schem save <名称>
//   /schem materials
// ============================================================

namespace levishematic::command {

// 注册所有命令
// isClient: 是否注册为客户端命令
void registerCommands(bool isClient);

} // namespace levishematic::command
