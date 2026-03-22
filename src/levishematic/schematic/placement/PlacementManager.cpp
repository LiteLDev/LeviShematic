#include "PlacementManager.h"

#include "levishematic/core/DataManager.h"
#include "levishematic/render/ProjectionRenderer.h"

#include "ll/api/service/Bedrock.h"

#include "mc/nbt/CompoundTag.h"
#include "mc/world/level/Level.h"
#include "mc/world/level/levelgen/structure/StructureTemplate.h"

#include <algorithm>
#include <fstream>

namespace levishematic::placement {
// ================================================================
// PlacementManager 实现
// ================================================================

SchematicPlacement::Id PlacementManager::loadAndPlace(
    const std::filesystem::path& path,
    BlockPos                      origin,
    const std::string&            name
) {
    auto placement = loadMcstructurePlacement(path, origin, name);
    if (!placement.has_value()) {
        return 0;
    }

    auto id = placement->getId();
    placement->setFilePath(path.is_absolute() ? path.string() : std::filesystem::absolute(path).string());
    mPlacements.push_back(std::move(*placement));
    mSelectedId = id;
    notifyChange();
    return id;
}

bool PlacementManager::removePlacement(SchematicPlacement::Id id) {
    auto it = std::find_if(mPlacements.begin(), mPlacements.end(), [id](const SchematicPlacement& p) {
        return p.getId() == id;
    });

    if (it == mPlacements.end()) return false;

    mPlacements.erase(it);

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
    auto it = std::find_if(mPlacements.begin(), mPlacements.end(), [id](const SchematicPlacement& p) {
        return p.getId() == id;
    });
    return (it != mPlacements.end()) ? &(*it) : nullptr;
}

const SchematicPlacement* PlacementManager::getPlacement(SchematicPlacement::Id id) const {
    auto it = std::find_if(mPlacements.begin(), mPlacements.end(), [id](const SchematicPlacement& p) {
        return p.getId() == id;
    });
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
        allEntries.insert(
            allEntries.end(),
            std::make_move_iterator(entries.begin()),
            std::make_move_iterator(entries.end())
        );
    }

    // 更新全局 ProjectionState
    render::getProjectionState().setEntries(std::move(allEntries));
}

void PlacementManager::rebuildAndRefresh(std::shared_ptr<RenderChunkCoordinator> coordinator) {
    auto previousSnapshot = render::getProjectionState().getSnapshot();
    rebuildProjection();
    render::triggerRebuildForProjection(std::move(coordinator), std::move(previousSnapshot));
}

// ================================================================
// Schematic 文件路径解析
// ================================================================

std::filesystem::path PlacementManager::resolveSchematicPath(const std::string& filename) const {
    namespace fs = std::filesystem;
    auto&        dm = core::DataManager::getInstance();

    auto schematicDir = mSchematicDir.empty() ? dm.getSchematicDirectory() : mSchematicDir;
    fs::path inputPath(filename);

    if (inputPath.is_absolute()) {
        if (fs::exists(inputPath)) {
            return inputPath;
        }

        auto absoluteWithExt = dm.makeSchematicFilePath(inputPath);
        if (fs::exists(absoluteWithExt)) {
            return absoluteWithExt;
        }

        return inputPath;
    }

    if (!schematicDir.empty()) {
        auto inDir = schematicDir / inputPath;
        if (fs::exists(inDir)) {
            return inDir;
        }

        auto inDirWithExt = dm.makeSchematicFilePath(inDir);
        if (fs::exists(inDirWithExt)) {
            return inDirWithExt;
        }
    }

    if (fs::exists(inputPath)) {
        return fs::absolute(inputPath);
    }

    auto withExt = dm.makeSchematicFilePath(inputPath);
    if (fs::exists(withExt)) {
        return fs::absolute(withExt);
    }

    return !schematicDir.empty() ? dm.makeSchematicFilePath(schematicDir / inputPath) : withExt;
}

std::vector<std::string> PlacementManager::listAvailableFiles() const {
    namespace fs = std::filesystem;
    auto&        dm = core::DataManager::getInstance();

    std::vector<std::string> files;
    auto schematicDir = mSchematicDir.empty() ? dm.getSchematicDirectory() : mSchematicDir;
    if (schematicDir.empty() || !fs::is_directory(schematicDir)) {
        return files;
    }

    for (const auto& entry : fs::directory_iterator(schematicDir)) {
        if (!entry.is_regular_file()) continue;
        if (entry.path().extension() == ".mcstructure") {
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
    // 清空 ProjectionState
    render::getProjectionState().clear();
}

void PlacementManager::notifyChange() {
    if (mOnChange) {
        mOnChange();
    }
}

std::optional<SchematicPlacement> PlacementManager::loadMcstructurePlacement(
    const std::filesystem::path& path,
    BlockPos                      origin,
    const std::string&            name
) const {
    try {
        std::ifstream file(path, std::ios::binary | std::ios::ate);
        if (!file.is_open()) {
            return std::nullopt;
        }

        auto fileSize = static_cast<size_t>(file.tellg());
        file.seekg(0, std::ios::beg);

        std::string rawData(fileSize, '\0');
        if (!file.read(rawData.data(), static_cast<std::streamsize>(fileSize))) {
            return std::nullopt;
        }

        auto tagResult = CompoundTag::fromBinaryNbt(rawData, true);
        if (!tagResult) {
            return std::nullopt;
        }

        auto registry = ll::service::getLevel()->getUnknownBlockTypeRegistry();
        StructureTemplate structureTemplate(path.filename().string(), registry);
        if (!structureTemplate.load(*tagResult)) {
            return std::nullopt;
        }

        const auto& data = structureTemplate.mStructureTemplateData;
        BlockPos size    = structureTemplate.rawSize();
        if (size.x <= 0 || size.y <= 0 || size.z <= 0) {
            return std::nullopt;
        }

        std::vector<SchematicPlacement::LocalBlockEntry> localBlocks;
        localBlocks.reserve(static_cast<size_t>(size.x) * size.y * size.z);

        for (int y = 0; y < size.y; ++y) {
            for (int z = 0; z < size.z; ++z) {
                for (int x = 0; x < size.x; ++x) {
                    BlockPos localPos{x, y, z};
                    auto* block = StructureTemplate::tryGetBlockAtPos(localPos, data, registry);
                    if (!block || block->isAir()) continue;
                    localBlocks.push_back({localPos, block});
                }
            }
        }

        std::string placementName = name.empty() ? path.stem().string() : name;
        return SchematicPlacement(std::move(localBlocks), size, origin, placementName);
    } catch (const std::exception&) {
        return std::nullopt;
    }
}

} // namespace levishematic::placement
