#pragma once
// ============================================================
// Command.h
// 命令系统注册
//
// 当前阶段命令（测试用）：
//   /schem load <文件名> [x y z]   ← Phase 2+ 加载 Schematic
//   /schem clear                    ← 清空投影
//   /rendert ...                    ← 测试渲染（临时）
//
// Phase 3+ 命令：
//   /schem move/rotate/mirror/list/remove/verify/save/select/materials
// ============================================================

namespace levishematic::command {

// 注册所有命令
// isClient: 是否注册为客户端命令
void registerCommands(bool isClient);

} // namespace levishematic::command
