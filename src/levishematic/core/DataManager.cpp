#include "DataManager.h"

#include "levishematic/render/ProjectionRenderer.h"

#include "ll/api/service/ServerInfo.h"

#include <filesystem>

namespace levishematic::core {

DataManager& DataManager::getInstance() {
    static DataManager instance;
    return instance;
}

render::ProjectionState& DataManager::getProjectionState() {
    return render::getProjectionState();
}

void DataManager::triggerRebuild(std::shared_ptr<RenderChunkCoordinator> coordinator) {
    render::triggerRebuildForProjection(std::move(coordinator));
}

void DataManager::rebuildAndRefresh(std::shared_ptr<RenderChunkCoordinator> coordinator) {
    mPlacementManager.rebuildAndRefresh(std::move(coordinator));
}

const std::filesystem::path& DataManager::getSchematicDirectory() const {
    return mSchematicDirectory;
}

std::filesystem::path DataManager::ensureSchematicDirectory() {
    std::error_code ec;
    std::filesystem::create_directories(mSchematicDirectory, ec);
    return mSchematicDirectory;
}

std::filesystem::path DataManager::makeSchematicFilePath(const std::filesystem::path& path) const {
    auto result = path;
    if (!result.is_absolute()) {
        result = mSchematicDirectory / result;
    }

    if (result.extension() != ".mcstructure") {
        result += ".mcstructure";
    }

    return result;
}

void DataManager::init() {
    std::filesystem::path structurePath;
    structurePath /= ll::getWorldPath().value();
    structurePath = structurePath.parent_path().parent_path();
    structurePath /= "schematics";

    mSchematicDirectory = structurePath;
    mPlacementManager.setSchematicDirectory(ensureSchematicDirectory());
}

void DataManager::shutdown() {
    // 清空 PlacementManager（含 ProjectionState 清理）
    mPlacementManager.clear();
    // 额外确保 ProjectionState 已清空
    render::getProjectionState().clear();
}

} // namespace levishematic::core
