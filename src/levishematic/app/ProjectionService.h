#pragma once

#include "levishematic/editor/EditorState.h"
#include "levishematic/render/ProjectionRenderer.h"
#include "levishematic/schematic/placement/PlacementStore.h"
#include "levishematic/verifier/VerifierTypes.h"

#include <memory>

class RenderChunkCoordinator;

namespace levishematic::app {

class ProjectionService {
public:
    ProjectionService(
        placement::PlacementState const& placementState,
        verifier::VerifierState const&   verifierState,
        editor::ViewState const&         viewState,
        render::ProjectionProjector&     projector
    );

    [[nodiscard]] bool flushRefresh(
        std::shared_ptr<RenderChunkCoordinator> const& coordinator
    );
    [[nodiscard]] std::shared_ptr<const render::ProjectionScene::DimensionScene> sceneForDimension(int dimensionId) const;
    void                                                         clear();

private:
    placement::PlacementState const& mPlacementState;
    verifier::VerifierState const&   mVerifierState;
    editor::ViewState const&         mViewState;
    render::ProjectionProjector&     mProjector;
};

} // namespace levishematic::app
