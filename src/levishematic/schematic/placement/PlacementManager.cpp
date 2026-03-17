#include "PlacementManager.h"

#include "levishematic/core/DataManager.h"
#include "levishematic/render/ProjectionRenderer.h"
#include "levishematic/schematic/LitematicSchematic.h"

#include <algorithm>

namespace levishematic::placement {

// ================================================================
// SchematicHolder 实现
// ================================================================

std::shared_ptr<schematic::LitematicSchematic>
SchematicHolder::loadSchematic(const std::filesystem::path& path) {
    // 规范化路径作为 key
    std::string key = std::filesystem::absolute(path).string();

    // 检查缓存
    auto it = mCache.find(key);
    if (it != mCache.end() && it->second && it->second->isLoaded()) {
        return it->second;
    }

    // 加载新的 Schematic
    auto schem = std::make_shared<schematic::LitematicSchematic>();
    if (!schem->loadFromFile(path)) {
        return nullptr; // 加载失败
    }

    mCache[key] = schem;
    return schem;
}

std::shared_ptr<schematic::LitematicSchematic>
SchematicHolder::getSchematic(const std::string& key) const {
    auto it = mCache.find(key);
    return (it != mCache.end()) ? it->second : nullptr;
}

bool SchematicHolder::hasSchematic(const std::string& key) const {
    return mCache.find(key) != mCache.end();
}

void SchematicHolder::cleanup() {
    for (auto it = mCache.begin(); it != mCache.end(); ) {
        // use_count == 1 表示只有缓存自身持有引用
        if (it->second.use_count() <= 1) {
            it = mCache.erase(it);
        } else {
            ++it;
        }
    }
}

void SchematicHolder::clear() {
    mCache.clear();
}

std::vector<std::string> SchematicHolder::getCachedNames() const {
    std::vector<std::string> names;
    names.reserve(mCache.size());
    for (const auto& [key, schem] : mCache) {
        if (schem && schem->isLoaded()) {
            names.push_back(schem->getMetadata().name);
        }
    }
    return names;
}

// ================================================================
// PlacementManager 实现
// ================================================================

SchematicPlacement::Id PlacementManager::loadAndPlace(
    const std::filesystem::path& path,
    BlockPos                      origin,
    const std::string&            name
) {
    // 尝试加载 Schematic（优先从缓存获取）
    auto schem = mHolder.loadSchematic(path);
    if (!schem) {
        return 0; // 加载失败
    }

    // 创建 Placement
    SchematicPlacement placement(schem, origin, name);
    placement.setFilePath(std::filesystem::absolute(path).string());

    SchematicPlacement::Id id = placement.getId();
    mPlacements.push_back(std::move(placement));
    mSelectedId = id; // 自动选中新创建的

    notifyChange();
    return id;
}

SchematicPlacement::Id PlacementManager::addPlacement(
    std::shared_ptr<schematic::LitematicSchematic> schematic,
    BlockPos                                        origin,
    const std::string&                              name
) {
    if (!schematic || !schematic->isLoaded()) {
        return 0;
    }

    SchematicPlacement placement(std::move(schematic), origin, name);
    SchematicPlacement::Id id = placement.getId();
    mPlacements.push_back(std::move(placement));
    mSelectedId = id;

    notifyChange();
    return id;
}

bool PlacementManager::removePlacement(SchematicPlacement::Id id) {
    auto it = std::find_if(mPlacements.begin(), mPlacements.end(),
        [id](const SchematicPlacement& p) { return p.getId() == id; });

    if (it == mPlacements.end()) return false;

    mPlacements.erase(it);

    // 如果删除的是当前选中的，自动选中最后一个
    if (mSelectedId == id) {
        if (!mPlacements.empty()) {
            mSelectedId = mPlacements.back().getId();
        } else {
            mSelectedId = 0;
        }
    }

    notifyChange();
    return true;
}

void PlacementManager::removeAllPlacements() {
    mPlacements.clear();
    mSelectedId = 0;
    notifyChange();
}

SchematicPlacement* PlacementManager::getPlacement(SchematicPlacement::Id id) {
    auto it = std::find_if(mPlacements.begin(), mPlacements.end(),
        [id](const SchematicPlacement& p) { return p.getId() == id; });
    return (it != mPlacements.end()) ? &(*it) : nullptr;
}

const SchematicPlacement* PlacementManager::getPlacement(SchematicPlacement::Id id) const {
    auto it = std::find_if(mPlacements.begin(), mPlacements.end(),
        [id](const SchematicPlacement& p) { return p.getId() == id; });
    return (it != mPlacements.end()) ? &(*it) : nullptr;
}

SchematicPlacement* PlacementManager::getSelected() {
    return getPlacement(mSelectedId);
}

const SchematicPlacement* PlacementManager::getSelected() const {
    return getPlacement(mSelectedId);
}

void PlacementManager::selectLast() {
    if (!mPlacements.empty()) {
        mSelectedId = mPlacements.back().getId();
    } else {
        mSelectedId = 0;
    }
}

// ================================================================
// 投影状态更新
// ================================================================

void PlacementManager::rebuildProjection() {
    std::vector<render::ProjEntry> allEntries;

    for (const auto& placement : mPlacements) {
        if (!placement.isEnabled() || !placement.isRenderEnabled()) continue;

        auto entries = placement.generateProjEntries();
        allEntries.insert(allEntries.end(),
                          std::make_move_iterator(entries.begin()),
                          std::make_move_iterator(entries.end()));
    }

    // 更新全局 ProjectionState
    render::getProjectionState().setEntries(std::move(allEntries));
}

void PlacementManager::rebuildAndRefresh(std::shared_ptr<RenderChunkCoordinator> coordinator) {
    rebuildProjection();
    render::triggerRebuildForProjection(std::move(coordinator));
}

// ================================================================
// Schematic 文件路径解析
// ================================================================

std::filesystem::path PlacementManager::resolveSchematicPath(const std::string& filename) const {
    namespace fs = std::filesystem;

    // 1. 如果是绝对路径且存在，直接返回
    fs::path p(filename);
    if (p.is_absolute() && fs::exists(p)) {
        return p;
    }

    // 2. 在 schematic 目录中查找
    if (!mSchematicDir.empty()) {
        fs::path inDir = mSchematicDir / filename;
        if (fs::exists(inDir)) return inDir;

        // 尝试添加 .litematic 扩展名
        fs::path withExt = mSchematicDir / (filename + ".litematic");
        if (fs::exists(withExt)) return withExt;
    }

    // 3. 在当前目录查找
    if (fs::exists(p)) return fs::absolute(p);

    // 4. 尝试添加扩展名
    fs::path withExt(filename + ".litematic");
    if (fs::exists(withExt)) return fs::absolute(withExt);

    // 未找到，返回原始路径（让调用方处理错误）
    return p;
}

std::vector<std::string> PlacementManager::listAvailableFiles() const {
    namespace fs = std::filesystem;
    std::vector<std::string> files;

    if (mSchematicDir.empty() || !fs::is_directory(mSchematicDir)) {
        return files;
    }

    for (const auto& entry : fs::directory_iterator(mSchematicDir)) {
        if (entry.is_regular_file() && entry.path().extension() == ".litematic") {
            files.push_back(entry.path().filename().string());
        }
    }

    std::sort(files.begin(), files.end());
    return files;
}

// ================================================================
// 生命周期
// ================================================================

void PlacementManager::clear() {
    mPlacements.clear();
    mSelectedId = 0;
    mHolder.clear();
    // 清空 ProjectionState
    render::getProjectionState().clear();
}

void PlacementManager::notifyChange() {
    if (mOnChange) {
        mOnChange();
    }
}

} // namespace levishematic::placement
