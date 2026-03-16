#pragma once
// ============================================================
// SelectionManager.h
// 选区管理器（Phase 4 实现）
// ============================================================

#include "levishematic/selection/AreaSelection.h"

#include <memory>

namespace levishematic::selection {

class SelectionManager {
public:
    // TODO: Phase 4 实现

private:
    std::unique_ptr<AreaSelection> mCurrentSelection;
};

} // namespace levishematic::selection
