#pragma once
// ============================================================
// LitematicSchematic.h
// 完整的 .litematic 文件解析器
//
// .litematic NBT 顶层结构：
//   root {
//     Version: int (5 or 6)
//     MinecraftDataVersion: int
//     SubVersion: int (optional, v6)
//     Metadata: {
//       Name, Author, Description, TimeCreated, TimeModified,
//       TotalBlocks, TotalVolume, RegionCount, EnclosingSize
//     }
//     Regions: {
//       "region_name": {
//         Position: { x, y, z }     ← 子区域原点偏移
//         Size: { x, y, z }         ← 子区域尺寸（可为负数！）
//         BlockStatePalette: TAG_List[TAG_Compound]
//         BlockStates: TAG_Long_Array (位压缩 palette ID)
//         Entities: TAG_List (可选)
//         TileEntities: TAG_List (可选)
//         PendingBlockTicks: TAG_List (可选)
//         PendingFluidTicks: TAG_List (可选)
//       }
//     }
//   }
//
// 注意事项：
//   1. Size 可以为负数（表示区域在原点另一侧），
//      实际尺寸取绝对值，方块坐标需要调整。
//   2. BlockStates 是 TAG_Long_Array（Java NBT type 12），
//      BE 的 NBT 系统不原生支持此类型，需要手动解析。
//   3. Litematica v5 和 v6 格式略有差异（v6 有 SubVersion）。
// ============================================================

#include "levishematic/schematic/BlockStateContainer.h"
#include "levishematic/schematic/BlockStatePalette.h"
#include "levishematic/schematic/SchematicMetadata.h"

#include "mc/nbt/CompoundTag.h"
#include "mc/world/level/BlockPos.h"
#include "mc/world/level/block/Block.h"

#include <filesystem>
#include <string>
#include <unordered_map>
#include <vector>

namespace levishematic::schematic {

// ================================================================
// 子区域数据
// ================================================================
struct RegionData {
    std::string        name;
    BlockPos           position;  // 区域原点偏移（相对于 Schematic 原点）
    BlockPos           rawSize;   // 原始 Size（可能为负数）

    BlockStatePalette  palette;
    BlockStateContainer blockStates;

    // 实际尺寸（取绝对值）
    int sizeX() const { return std::abs(rawSize.x); }
    int sizeY() const { return std::abs(rawSize.y); }
    int sizeZ() const { return std::abs(rawSize.z); }

    // 总方块数
    int totalBlocks() const { return sizeX() * sizeY() * sizeZ(); }

    // 获取指定局部坐标的 palette ID
    int getPaletteId(int x, int y, int z) const {
        return blockStates.getPaletteId(x, y, z, sizeX(), sizeZ());
    }

    // 获取指定局部坐标的 BE Block*
    const Block* getBlock(int x, int y, int z) const {
        int pid = getPaletteId(x, y, z);
        return palette.getBlock(pid);
    }

    // 判断指定坐标是否是空气
    bool isAir(int x, int y, int z) const {
        int pid = getPaletteId(x, y, z);
        return palette.getEntry(pid).isAir;
    }

    // 获取调整后的坐标偏移（处理负 Size）
    // Litematica 中负尺寸意味着方块从 position 向负方向延伸
    // 实际方块的世界坐标 = position + adjustedOffset + localPos
    BlockPos getAdjustedOffset() const {
        int offX = (rawSize.x < 0) ? (rawSize.x + 1) : 0;
        int offY = (rawSize.y < 0) ? (rawSize.y + 1) : 0;
        int offZ = (rawSize.z < 0) ? (rawSize.z + 1) : 0;
        return {offX, offY, offZ};
    }
};

// ================================================================
// LitematicSchematic — 完整的 .litematic 文件表示
// ================================================================
class LitematicSchematic {
public:
    LitematicSchematic() = default;

    // 从文件加载
    // 返回 true 表示加载成功
    bool loadFromFile(const std::filesystem::path& path);

    // 从 CompoundTag 加载（NBT 已解析）
    // longArrays 是手动解析出的 BlockStates 数据
    // key = region name
    bool loadFromNBT(
        const CompoundTag& root,
        const std::unordered_map<std::string, std::vector<int64_t>>& longArrays
    );

    // ---- 查询 ----
    bool isLoaded() const { return mLoaded; }
    int  getVersion() const { return mVersion; }

    const SchematicMetadata& getMetadata() const { return mMetadata; }
    const std::vector<RegionData>& getRegions() const { return mRegions; }

    // 通过名称查找区域
    const RegionData* findRegion(const std::string& name) const;

    // 获取所有区域名称
    std::vector<std::string> getRegionNames() const;

    // 总方块数（所有区域）
    int getTotalNonAirBlocks() const;

private:
    // 解析 Metadata 标签
    void parseMetadata(const CompoundTag& metaTag);

    // 解析单个 Region
    RegionData parseRegion(
        const std::string& name,
        const CompoundTag& regionTag,
        const std::vector<int64_t>& blockStatesData
    );

    SchematicMetadata       mMetadata;
    std::vector<RegionData> mRegions;
    int                     mVersion = 0;
    bool                    mLoaded  = false;
};

// ================================================================
// 文件加载辅助函数
//
// .litematic 文件中的 TAG_Long_Array（Java NBT type 12）
// 不被 LL 的 CompoundTag::fromBinaryNbt 原生支持。
//
// 此函数进行两阶段解析：
//   1. GZip 解压
//   2. 手动遍历 NBT 二进制数据，提取 TAG_Long_Array 字段
//   3. 将剩余数据修补后交给 CompoundTag::fromBinaryNbt
//
// 或者更简单的方案：
//   手动实现完整的 Big-Endian NBT 解析（包含 type 12）
// ================================================================

// 从原始 NBT 二进制数据中提取 TAG_Long_Array 字段
// 返回 { regionName -> { fieldName -> long_array } }
struct LongArrayExtractResult {
    std::unordered_map<std::string, std::vector<int64_t>> blockStates;
    // 修补后的 NBT 数据（TAG_Long_Array 替换为 TAG_End 或跳过）
    // 供 CompoundTag::fromBinaryNbt 使用
};

// 完整的 .litematic 文件加载（GZip + NBT + Long Array 提取）
// 返回已加载的 LitematicSchematic
LitematicSchematic loadLitematicFile(const std::filesystem::path& path);

} // namespace levishematic::schematic
