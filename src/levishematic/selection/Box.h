#pragma once
// ============================================================
// Box.h
// 选区盒子（Phase 4 实现）
// ============================================================

#include "mc/world/level/BlockPos.h"

namespace levishematic::selection {

struct Box {
    BlockPos pos1;
    BlockPos pos2;

    // 获取最小/最大角
    BlockPos getMin() const {
        return {
            std::min(pos1.x, pos2.x),
            std::min(pos1.y, pos2.y),
            std::min(pos1.z, pos2.z)
        };
    }

    BlockPos getMax() const {
        return {
            std::max(pos1.x, pos2.x),
            std::max(pos1.y, pos2.y),
            std::max(pos1.z, pos2.z)
        };
    }

    // 尺寸
    BlockPos getSize() const {
        auto mn = getMin();
        auto mx = getMax();
        return {mx.x - mn.x + 1, mx.y - mn.y + 1, mx.z - mn.z + 1};
    }
};

} // namespace levishematic::selection
