#include "BlockStateContainer.h"

namespace levishematic::schematic {

void BlockStateContainer::init(const std::vector<int64_t>& longArray, int bitsPerBlock, int totalBlocks) {
    mData         = longArray;
    mBitsPerBlock = bitsPerBlock;
    mTotalBlocks  = totalBlocks;
    mMask         = (static_cast<int64_t>(1) << bitsPerBlock) - 1;
}

// ================================================================
// 位解包（Litematica 紧密打包格式）
//
// Litematica 使用紧密打包（tight packing），
// 一个值可能横跨两个 long。
//
// 算法：
//   startBit = index * bitsPerBlock
//   longIndex = startBit / 64
//   bitOffset = startBit % 64
//
//   if (bitOffset + bitsPerBlock <= 64):
//     value = (data[longIndex] >>> bitOffset) & mask
//   else:
//     // 跨 long 边界
//     bitsInFirst = 64 - bitOffset
//     bitsInSecond = bitsPerBlock - bitsInFirst
//     value = (data[longIndex] >>> bitOffset)        // 低位部分
//           | (data[longIndex+1] << bitsInFirst)     // 高位部分
//     value &= mask
//
// 注意：Java 的 long 是 signed 64-bit，
//       右移使用 unsigned（>>>）等价于 C++ 的 static_cast<uint64_t>() >> shift
// ================================================================
int BlockStateContainer::getPaletteId(int index) const {
    if (index < 0 || index >= mTotalBlocks || mData.empty() || mBitsPerBlock <= 0) {
        return 0;
    }

    int64_t startBit  = static_cast<int64_t>(index) * mBitsPerBlock;
    int     longIndex = static_cast<int>(startBit / 64);
    int     bitOffset = static_cast<int>(startBit % 64);

    if (longIndex < 0 || static_cast<size_t>(longIndex) >= mData.size()) {
        return 0;
    }

    // 使用 unsigned 算术避免符号扩展
    uint64_t uLong = static_cast<uint64_t>(mData[longIndex]);
    uint64_t uMask = static_cast<uint64_t>(mMask);

    int value;
    if (bitOffset + mBitsPerBlock <= 64) {
        // 不跨边界
        value = static_cast<int>((uLong >> bitOffset) & uMask);
    } else {
        // 跨 long 边界
        int bitsInFirst  = 64 - bitOffset;
        int bitsInSecond = mBitsPerBlock - bitsInFirst;

        if (static_cast<size_t>(longIndex + 1) >= mData.size()) {
            return 0; // 数据不完整
        }

        uint64_t uLongNext = static_cast<uint64_t>(mData[longIndex + 1]);

        uint64_t lopart = uLong >> bitOffset;
        uint64_t hipart = uLongNext << bitsInFirst;

        value = static_cast<int>((lopart | hipart) & uMask);
    }

    return value;
}

int BlockStateContainer::getPaletteId(int x, int y, int z, int sizeX, int sizeZ) const {
    // Litematica 索引顺序：Y→Z→X
    int index = y * sizeX * sizeZ + z * sizeX + x;
    return getPaletteId(index);
}

} // namespace levishematic::schematic
