#include "ProjectionRefresh.h"

#include "levishematic/app/AppKernel.h"

#include "ll/api/service/TargetedBedrock.h"

#include "mc/client/game/ClientInstance.h"
#include "mc/client/player/LocalPlayer.h"
#include "mc/client/renderer/game/LevelRenderer.h"
#include "mc/world/level/BlockSource.h"
#include "mc/world/level/dimension/Dimension.h"

namespace levishematic::app {

std::shared_ptr<RenderChunkCoordinator> resolveCoordinatorForDimension(int dimensionId) {
    auto client = ll::service::getClientInstance();
    if (!client || !client->getLevelRenderer()) {
        return nullptr;
    }

    return client->getLevelRenderer()->mRenderChunkCoordinators->at(dimensionId);
}

bool refreshProjectionState(
    BlockSource*                                    source,
    std::shared_ptr<RenderChunkCoordinator> const& coordinator
) {
    if (!hasAppKernel()) {
        return false;
    }

    auto& kernel = getAppKernel();
    if (source) {
        kernel.verifier().refresh(*source);
    } else {
        kernel.verifier().refresh();
    }

    return kernel.projection().flushRefresh(coordinator);
}

bool refreshProjectionStateForDimension(Dimension* dimension) {
    if (!dimension) {
        return refreshProjectionState(nullptr, nullptr);
    }

    return refreshProjectionState(
        &dimension->getBlockSourceFromMainChunkSource(),
        resolveCoordinatorForDimension(dimension->getDimensionId())
    );
}

bool refreshCurrentClientProjectionState() {
    auto client = ll::service::getClientInstance();
    if (!client || !client->getLevelRenderer()) {
        return false;
    }

    auto* player = client->getLocalPlayer();
    if (!player) {
        return false;
    }

    return refreshProjectionState(
        client->getRegion(),
        resolveCoordinatorForDimension(static_cast<int>(player->getDimensionId()))
    );
}

} // namespace levishematic::app
