#pragma once
// ============================================================
// SchematicMetadata.h
// Litematica .litematic 文件元数据
//
// 对应 .litematic NBT 结构中的 Metadata 标签：
//   Metadata {
//     Name: string
//     Author: string
//     Description: string
//     TimeCreated: long
//     TimeModified: long
//     TotalBlocks: int
//     TotalVolume: int
//     RegionCount: int
//     EnclosingSize: { x, y, z }
//   }
// ============================================================

#include "mc/world/level/BlockPos.h"

#include <cstdint>
#include <string>

namespace levishematic::schematic {

struct SchematicMetadata {
    std::string name;
    std::string author;
    std::string description;
    int64_t     timeCreated   = 0;
    int64_t     timeModified  = 0;
    int         totalBlocks   = 0;
    int         totalVolume   = 0;
    int         regionCount   = 0;
    BlockPos    enclosingSize = {0, 0, 0};
};

} // namespace levishematic::schematic
