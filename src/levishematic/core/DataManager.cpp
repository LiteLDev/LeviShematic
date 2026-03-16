#include "DataManager.h"

#include "levishematic/render/ProjectionRenderer.h"

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

void DataManager::init() {
    // 初始化各子系统
    // Phase 3+: PlacementManager, SelectionManager 等
}

void DataManager::shutdown() {
    // 清理投影状态
    render::getProjectionState().clear();
    // Phase 3+: 其他子系统清理
}

} // namespace levishematic::core
