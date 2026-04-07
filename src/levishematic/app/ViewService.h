#pragma once

#include "levishematic/app/Result.h"
#include "levishematic/editor/EditorState.h"

#include <string>

namespace levishematic::app {

struct ViewError {
    enum class Code {
        InvalidLayerRange,
    };

    Code        code = Code::InvalidLayerRange;
    std::string detail;

    [[nodiscard]] std::string describe() const;
};

using ViewMutationStatus = StatusResult<ViewError>;

class ViewService {
public:
    explicit ViewService(editor::ViewState& viewState) : mViewState(viewState) {}

    [[nodiscard]] editor::LayerRange const& currentLayerRange() const { return mViewState.layerRange; }

    [[nodiscard]] ViewMutationStatus setMinY(int minY);
    [[nodiscard]] ViewMutationStatus setMaxY(int maxY);
    [[nodiscard]] ViewMutationStatus setRange(int minY, int maxY);
    [[nodiscard]] bool               enableLayerRange();
    [[nodiscard]] bool               disableLayerRange();

private:
    [[nodiscard]] static ViewMutationStatus validateRange(int minY, int maxY);
    void                                    touch();

    editor::ViewState& mViewState;
};

} // namespace levishematic::app
