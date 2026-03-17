#include "RenderHook.h"

#include "levishematic/render/ProjectionRenderer.h"
#include "levishematic/util/PositionUtils.h"

#include "ll/api/memory/Hook.h"

#include "mc/client/renderer/block/BlockTessellator.h"
#include "mc/client/renderer/chunks/RenderChunkBuilder.h"
#include "mc/world/level/block/Block.h"

// ================================================================
// 引擎内部结构（从 IDA 反编译获得）
// ================================================================

struct BlockQueueEntry {
    BlockPos     pos;       // offset 0x00, 12 bytes
    const Block* blockInfo; // offset 0x10, 8 bytes
};

struct RenderChunkGeometry {
    char     unk[0x34];
    BlockPos mPosition; // SubChunk 原点
    BlockPos mCenter;
    char     unk2[0x1fc];
};

namespace levishematic::hook {

using namespace levishematic::render;
using namespace levishematic::util;

// ================================================================
// Hook 1：RenderChunkBuilder::_sortBlocks
//
// 真实签名（IDA 0x14201a600，RVA = 0x201a600）：
//   char _sortBlocks(BlockSource& region,
//                    RenderChunkGeometry& renderChunkGeometry,
//                    bool airAndSimpleBlocks,
//                    AirAndSimpleBlockBits& airBits)
//
// 职责：
//   1. 原子 load 快照到 tl_currentSnapshot（无锁）
//   2. 用 renderChunkGeometry.mPosition 查 bySubChunk HashMap（O(1)）
//   3. push_back 投影块进 mQueues[BLEND]
//   4. 设置 tl_hasProjection 标志
// ================================================================
LL_AUTO_TYPE_INSTANCE_HOOK(
    ProjectionSortBlocksHook,
    ll::memory::HookPriority::Normal,
    RenderChunkBuilder,
    &RenderChunkBuilder::_sortBlocks,
    bool,
    BlockSource&                                                 region,
    RenderChunkGeometry&                                         renderChunkGeometry,
    bool                                                         transparentLeaves,
    AirAndSimpleBlockBits&                                       airAndSimpleBlocks,
    RenderChunkPerformanceTrackingData::RenderChunkBuildDetails& renderChunkBuildDetails
) {
    // Step 1: 执行原始 _sortBlocks
    bool result = origin(region, renderChunkGeometry, transparentLeaves, airAndSimpleBlocks, renderChunkBuildDetails);

    // load 快照（atomic，无锁，工作线程安全）
    tl_currentSnapshot = getProjectionState().getSnapshot();
    tl_hasProjection   = false;

    if (!tl_currentSnapshot || tl_currentSnapshot->empty()) {
        return result;
    }

    // renderChunkGeometry.mPosition 是 SubChunk 原点（偏移 0x34）
    uint64_t scKey = encodeSubChunkKey(renderChunkGeometry.mPosition);

    auto it = tl_currentSnapshot->bySubChunk.find(scKey);
    if (it == tl_currentSnapshot->bySubChunk.end() || it->second.empty()) {
        return result;
    }

    // 有投影块：设置标志，push 进 BLEND 队列
    tl_hasProjection = true;

    for (const auto& e : it->second) {
        BlockQueueEntry entry;
        entry.pos       = e.pos;
        entry.blockInfo = e.block;
        this->mQueues[RENDERLAYER_BLEND].push_back(entry);
    }

    return result;
}

// ================================================================
// Hook 2：BlockTessellator::tessellateInWorld（精确颜色控制）
//
// 签名（IDA 0x141f250e0）：
//   bool tessellateInWorld(Tessellator& tessellator,
//                          const Block& block,
//                          const BlockPos& pos,
//                          bool useCalcWithCache)
//
// 性能分层：
//   无投影 SubChunk → 读 1 次 bool，直接 origin（几乎零开销）
//   有投影 SubChunk → 1 次 unordered_map 查找（O(1)）
//     命中（投影块）→ 设置 mColorOverride + origin + 清除
//     未命中（真实方块）→ 直接 origin
//
// 草方块变色根治：
//   每次只对命中 pos 的那次调用设置 mColorOverride，origin 返回后立刻清除。
//   其他方块走正常生物群系色采样路径。
// ================================================================
LL_AUTO_TYPE_INSTANCE_HOOK(
    ProjectionTessellateHook,
    ll::memory::HookPriority::Normal,
    BlockTessellator,
    &BlockTessellator::tessellateInWorld,
    bool,
    Tessellator&    tessellator,
    Block const&    block,
    BlockPos const& pos,
    bool            useCalcWithCache
) {
    // 快速路径：当前 SubChunk 无投影块（绝大多数 SubChunk）
    if (!tl_hasProjection) {
        return origin(tessellator, block, pos, useCalcWithCache);
    }

    // O(1) 查 pos 是否是投影块
    auto it = tl_currentSnapshot->posColorMap.find(encodePosKey(pos));
    if (it == tl_currentSnapshot->posColorMap.end()) {
        // 真实方块，完全不碰 mColorOverride
        return origin(tessellator, block, pos, useCalcWithCache);
    }

    // 投影块：精确夹住 mColorOverride
    this->mColorOverride = it->second;

    bool result = origin(tessellator, block, pos, useCalcWithCache);

    // 立刻清除，不影响下一个方块
    this->mColorOverride->reset();
    return result;
}

} // namespace levishematic::hook
