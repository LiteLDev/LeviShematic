#pragma once
// ============================================================
// BlockStateContainer.h
// Litematica .litematic 方块状态位压缩存储
//
// .litematic 格式使用位压缩 long[] 数组存储每个方块的 palette ID：
//
// NBT 结构：
//   Region {
//     BlockStates: TAG_Long_Array  ← 位压缩的 palette 索引
//     BlockStatePalette: TAG_List  ← 调色板（在 BlockStatePalette.h 中处理）
//   }
//
// 位压缩方案（Litematica 格式）：
//   - 每个方块占 bitsPerBlock 位（由调色板大小决定，最小 2）
//   - 按 Y→Z→X 顺序排列（index = y * sizeX * sizeZ + z * sizeX + x）
//   - long 数组中，低位在前（little-endian bit packing）
//   - 跨 long 边界：一个值可能横跨两个 long
//
// 注意：Litematica 与 Vanilla 的位压缩有差异！
//   - Vanilla（1.16+）不允许跨 long 边界
//   - Litematica 允许跨 long 边界（紧密打包）
// ============================================================

#include <cstdint>
#include <vector>

namespace levishematic::schematic {

class BlockStateContainer {
public:
    BlockStateContainer() = default;

    // 初始化容器
    // longArray: NBT 中的 BlockStates long[] 数据
    // bitsPerBlock: 每个方块占的位数
    // totalBlocks: 方块总数（sizeX * sizeY * sizeZ）
    void init(const std::vector<int64_t>& longArray, int bitsPerBlock, int totalBlocks);

    // 获取指定索引处的 palette ID
    // index = y * sizeX * sizeZ + z * sizeX + x
    int getPaletteId(int index) const;

    // 获取指定坐标的 palette ID
    // 调用者负责确保 x, y, z 在范围内
    int getPaletteId(int x, int y, int z, int sizeX, int sizeZ) const;

    // 数据是否有效
    bool isValid() const { return !mData.empty() && mBitsPerBlock > 0; }

    // 获取位数
    int getBitsPerBlock() const { return mBitsPerBlock; }

    // 获取总方块数
    int getTotalBlocks() const { return mTotalBlocks; }

private:
    std::vector<int64_t> mData;          // 原始 long[] 数据
    int                  mBitsPerBlock = 0;
    int                  mTotalBlocks  = 0;
    int64_t              mMask         = 0; // (1 << bitsPerBlock) - 1
};

} // namespace levishematic::schematic
