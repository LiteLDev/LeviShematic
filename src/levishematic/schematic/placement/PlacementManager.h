#pragma once
// ============================================================
// PlacementManager.h
// 投影放置管理器 — 管理所有已加载的投影放置实例
//
// 职责：
//   - 管理多个 SchematicPlacement 实例
//   - 加载/卸载 Schematic 文件 → 创建/删除 Placement
//   - 合并所有 enabled Placement 的 ProjEntry → 更新 ProjectionState
//   - 触发渲染重建
//   - 提供当前选中的 Placement（用于命令操作）
//
// Phase 3 核心组件
// ============================================================

#include "levishematic/schematic/placement/SchematicPlacement.h"
#include "levishematic/render/ProjectionRenderer.h"

#include "mc/world/level/BlockPos.h"

#include <filesystem>
#include <functional>
#include <optional>
#include <string>
#include <vector>

// 前向声明
class RenderChunkCoordinator;

namespace levishematic::placement {

// ================================================================
// PlacementManager — 投影放置管理器
// ================================================================
class PlacementManager {
public:
    PlacementManager() = default;

    // ---- 加载与创建 ----

    // 从文件加载 .mcstructure 并在指定位置创建 Placement
    // 返回 Placement ID，失败返回 0
    SchematicPlacement::Id loadAndPlace(
        const std::filesystem::path& path,
        BlockPos                      origin,
        const std::string&            name = ""
    );

    // ---- 删除 ----
    bool removePlacement(SchematicPlacement::Id id);
    void removeAllPlacements();

    // ---- 查询 ----
    SchematicPlacement*       getPlacement(SchematicPlacement::Id id);
    const SchematicPlacement* getPlacement(SchematicPlacement::Id id) const;

    // 获取所有 Placement
    const std::vector<SchematicPlacement>& getAllPlacements() const { return mPlacements; }
    size_t getPlacementCount() const { return mPlacements.size(); }
    bool   isEmpty() const { return mPlacements.empty(); }

    // ---- 当前选中的 Placement（用于命令快捷操作）----
    SchematicPlacement::Id getSelectedId() const { return mSelectedId; }
    void setSelectedId(SchematicPlacement::Id id) { mSelectedId = id; }

    // 获取当前选中的 Placement（若无选中或已删除，返回 nullptr）
    SchematicPlacement* getSelected();
    const SchematicPlacement* getSelected() const;

    // 自动选中最后添加的 Placement
    void selectLast();

    // ---- 投影状态更新 ----

    // 重建 ProjectionState（合并所有 enabled Placement 的 ProjEntry）
    void rebuildProjection();

    // 重建投影并触发渲染重建
    void rebuildAndRefresh(std::shared_ptr<RenderChunkCoordinator> coordinator);

    // ---- 回调注册（为未来 GUI 预留）----
    using OnChangeCallback = std::function<void()>;
    void setOnChangeCallback(OnChangeCallback cb) { mOnChange = std::move(cb); }

    // ---- 生命周期 ----
    void clear();

    // ---- Schematic 文件搜索路径 ----
    void setSchematicDirectory(const std::filesystem::path& dir) { mSchematicDir = dir; }
    const std::filesystem::path& getSchematicDirectory() const { return mSchematicDir; }

    // 在 schematic 目录中查找文件（支持不带扩展名）
    std::filesystem::path resolveSchematicPath(const std::string& filename) const;

    // 列出 schematic 目录中的所有 .mcstructure 文件
    std::vector<std::string> listAvailableFiles() const;

private:
    std::optional<SchematicPlacement> loadMcstructurePlacement(
        const std::filesystem::path& path,
        BlockPos                      origin,
        const std::string&            name
    ) const;

    void notifyChange();

    std::vector<SchematicPlacement> mPlacements;
    SchematicPlacement::Id          mSelectedId = 0;
    std::filesystem::path           mSchematicDir;
    OnChangeCallback                mOnChange;
};

} // namespace levishematic::placement
