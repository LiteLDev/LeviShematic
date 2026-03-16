#pragma once
// ============================================================
// RenderHook.h
// 渲染 Hook 声明
//
// Hook 1: RenderChunkBuilder::_sortBlocks
//   → 向渲染队列注入投影方块
//
// Hook 2: BlockTessellator::tessellateInWorld
//   → 精确控制投影方块颜色（mColorOverride）
//
// 这些 Hook 通过 LL_AUTO_TYPE_INSTANCE_HOOK 自动注册，
// 包含此头文件的 .cpp 即可激活。
// ============================================================

namespace levishematic::hook {

// Hook 在 RenderHook.cpp 中通过 LL_AUTO_TYPE_INSTANCE_HOOK 宏自动注册。
// 此头文件仅作为文档和模块入口声明。

} // namespace levishematic::hook
