#pragma once
// ============================================================
// DataManager.h
// 全局状态中枢（单例）
//
// 管理所有子系统：
//   - ProjectionState（投影渲染状态）
//   - PlacementManager（投影放置管理）[Phase 3]
//   - SelectionManager（选区管理）[Phase 4]
//   - SchematicHolder（Schematic 缓存）[Phase 3]
// ============================================================

#include "levishematic/render/ProjectionRenderer.h"

#include <memory>

// 前向声明
class RenderChunkCoordinator;

namespace levishematic::core {

class DataManager {
public:
    static DataManager& getInstance();

    // 禁止拷贝/移动
    DataManager(const DataManager&)            = delete;
    DataManager& operator=(const DataManager&) = delete;
    DataManager(DataManager&&)                 = delete;
    DataManager& operator=(DataManager&&)      = delete;

    // ---- 投影渲染状态 ----
    render::ProjectionState& getProjectionState();

    // ---- 触发重建 ----
    void triggerRebuild(std::shared_ptr<RenderChunkCoordinator> coordinator);

    // ---- 生命周期 ----
    void init();
    void shutdown();

private:
    DataManager() = default;
    ~DataManager() = default;
};

} // namespace levishematic::core
