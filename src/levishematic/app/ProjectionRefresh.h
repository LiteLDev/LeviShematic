#pragma once

#include <memory>

class BlockSource;
class Dimension;
class RenderChunkCoordinator;

namespace levishematic::app {

[[nodiscard]] std::shared_ptr<RenderChunkCoordinator> resolveCoordinatorForDimension(int dimensionId);
[[nodiscard]] bool refreshProjectionState(
    BlockSource*                                    source,
    std::shared_ptr<RenderChunkCoordinator> const& coordinator
);
[[nodiscard]] bool refreshProjectionStateForDimension(Dimension* dimension);
[[nodiscard]] bool refreshCurrentClientProjectionState();

} // namespace levishematic::app
