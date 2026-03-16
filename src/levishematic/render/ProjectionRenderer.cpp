#include "ProjectionRenderer.h"

#include "levishematic/util/PositionUtils.h"

#include "ll/api/memory/Memory.h"

#include <windows.h>

namespace levishematic::render {

// ================================================================
// 线程局部状态定义
// ================================================================
thread_local std::shared_ptr<const ProjectionSnapshot> tl_currentSnapshot;
thread_local bool                                      tl_hasProjection = false;

// ================================================================
// ProjectionState 实现
// ================================================================

void ProjectionState::setEntries(std::vector<ProjEntry> newEntries) {
    std::lock_guard<std::mutex> lk(mEntriesMutex);
    mEntries = std::move(newEntries);
    rebuildSnapshot_locked();
}

void ProjectionState::clear() { setEntries({}); }

void ProjectionState::setSingle(BlockPos pos, const Block* block, mce::Color color) {
    setEntries({ProjEntry{pos, block, color}});
}

std::shared_ptr<const ProjectionSnapshot> ProjectionState::getSnapshot() const {
    return mSnapshot.load(std::memory_order_acquire);
}

size_t ProjectionState::getEntryCount() const {
    std::lock_guard<std::mutex> lk(mEntriesMutex);
    return mEntries.size();
}

void ProjectionState::rebuildSnapshot_locked() {
    auto snap = std::make_shared<ProjectionSnapshot>();
    for (const auto& e : mEntries) {
        uint64_t scKey = util::subChunkKeyFromWorldPos(e.pos.x, e.pos.y, e.pos.z);
        snap->bySubChunk[scKey].push_back(e);
        snap->posColorMap[util::encodePosKey(e.pos)] = e.color;
    }
    mSnapshot.store(std::move(snap), std::memory_order_release);
}

// ================================================================
// 全局单例
// ================================================================
static ProjectionState gProjectionState;

ProjectionState& getProjectionState() { return gProjectionState; }

// ================================================================
// SubChunk 重建触发
//
// RenderChunkCoordinator::_setDirty（RVA: 0x201fd00）：
//   void _setDirty(const BlockPos& min, const BlockPos& max,
//                  bool immediate, bool changesVisibility, bool canInterlock)
//
//   min/max 是世界坐标，内部 >>4 转 SubChunk 索引。
//   immediate=false → 下一帧重建（推荐，避免卡顿）
// ================================================================
static constexpr DWORD kSetDirtyRVA = 0x201fd00;

void triggerRebuildForProjection(std::shared_ptr<RenderChunkCoordinator> coordinator) {
    if (!coordinator) return;

    auto snap = gProjectionState.getSnapshot();
    if (!snap || snap->empty()) return;

    HMODULE hModule   = GetModuleHandle(nullptr);
    void*   setDirtyAddr = reinterpret_cast<BYTE*>(hModule) + kSetDirtyRVA;

    for (const auto& [scKey, vec] : snap->bySubChunk) {
        if (vec.empty()) continue;
        const BlockPos& p = vec[0].pos;
        ll::memory::addressCall<
            void,
            RenderChunkCoordinator*,
            BlockPos const&,
            BlockPos const&,
            bool,
            bool,
            bool>(setDirtyAddr, coordinator.get(), p, p, false, false, false);
    }
}

} // namespace levishematic::render
