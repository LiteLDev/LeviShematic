#pragma once
// ============================================================
// NbtReader.h
// .litematic GZip 解压 + NBT 加载
//
// .litematic 格式：
//   GZip 压缩的 Big-Endian NBT 二进制流，根节点为 TAG_Compound
//
// 解析策略：
//   1. 用 zlib 解压 GZip → 原始 NBT 字节
//   2. 调用 LeviLamina 的 CompoundTag::fromBinaryNbt(data, false)
//      （第二个参数 false = Big-Endian，Java Edition 格式）
//   3. 返回 CompoundTag，后续全部通过 LL NBT API 访问
//
// 注意：
//   LL 的 CompoundTag::fromBinaryNbt 不支持 TAG_Long_Array (type 12)，
//   .litematic 的 BlockStates 字段使用此类型。
//   完整的 .litematic 解析请使用 LitematicSchematic::loadFromFile()，
//   该函数包含自定义的 NBT 解析器。
//
// 依赖：
//   - zlib（LeviLamina xmake 工程通常已经通过 zlib-ng 或 zlib 提供）
//   - mc/nbt/CompoundTag.h（LeviLamina）
// ============================================================

#include "mc/nbt/CompoundTag.h"

#include <filesystem>

namespace levishematic::nbtreader {

// ============================================================
// GZip 解压
// ============================================================
std::string gzipDecompress(const void* src, size_t srcLen);

// ============================================================
// 从文件路径加载 NBT → CompoundTag
//
// 注意：此函数使用 LL 的 fromBinaryNbt，
//       不支持 TAG_Long_Array。
//       仅适用于不含 TAG_Long_Array 的 NBT 文件。
// ============================================================
CompoundTag loadNbtFromFile(const std::filesystem::path& path);

} // namespace levishematic::nbtreader
