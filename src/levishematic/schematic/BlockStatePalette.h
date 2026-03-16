#pragma once
// ============================================================
// BlockStatePalette.h
// Java blockstate 调色板 → BE Block* 映射
//
// .litematic 每个 Region 都有一个 BlockStatePalette：
//   TAG_List("BlockStatePalette") [
//     TAG_Compound {
//       "Name": "minecraft:oak_log"
//       "Properties": TAG_Compound {
//         "axis": "y"
//       }
//     },
//     ...
//   ]
//
// 每个条目包含：
//   - Java blockstate 名称 + 属性
//   - 对应的 BE Block* 指针（通过 BlockUtils 解析）
//
// 索引 0 通常是 air（但不保证）。
// ============================================================

#include "levishematic/util/BlockUtils.h"

#include "mc/nbt/CompoundTag.h"
#include "mc/world/level/block/Block.h"

#include <string>
#include <vector>

namespace levishematic::schematic {

// ================================================================
// 调色板条目
// ================================================================
struct PaletteEntry {
    util::JavaBlockState javaState;    // Java blockstate（名称 + 属性）
    const Block*         bedrockBlock; // 对应的 BE Block*（可能为 nullptr）
    bool                 isAir;        // 是否是空气方块
};

// ================================================================
// BlockStatePalette
//
// 从 NBT 的 BlockStatePalette 列表构建。
// 索引对应 BlockStateContainer 中的 palette ID。
// ================================================================
class BlockStatePalette {
public:
    BlockStatePalette() = default;

    // 从 NBT ListTag 构建调色板
    // paletteList 是 Region NBT 中的 "BlockStatePalette" 列表
    void loadFromNBT(const ListTag& paletteList);

    // 通过索引获取条目
    const PaletteEntry& getEntry(int index) const;

    // 通过索引获取 BE Block*
    const Block* getBlock(int index) const;

    // 调色板大小
    size_t size() const { return mEntries.size(); }

    // 计算存储每个 palette ID 所需的最小位数
    // bits = max(2, ceil(log2(size)))
    int getBitsPerBlock() const;

    // 是否为空
    bool empty() const { return mEntries.empty(); }

    // 获取所有条目
    const std::vector<PaletteEntry>& getEntries() const { return mEntries; }

private:
    std::vector<PaletteEntry> mEntries;
};

} // namespace levishematic::schematic
