#pragma once
// ============================================================
// AreaSelection.h
// 区域选区（Phase 4 实现）
// ============================================================

#include "levishematic/selection/Box.h"

#include <map>
#include <string>

namespace levishematic::selection {

class AreaSelection {
public:
    std::string              name;
    std::map<std::string, Box> subRegionBoxes;

    // TODO: Phase 4 实现
};

} // namespace levishematic::selection
