#pragma once
// ============================================================
// BlockUtils.h
// Java Edition → Bedrock Edition 方块名映射
//
// .litematic 文件使用 Java Edition 方块 ID（如 minecraft:grass_block）
// BE 使用不同的名称（如 minecraft:grass_block 在某些版本中有差异）
//
// 此模块提供：
//   - javaToBedrock(javaName) → BE 方块名
//   - Java blockstate 属性 → BE Block* 查找
//   - 常见差异方块的映射表
// ============================================================

#include "mc/world/level/block/Block.h"

#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>

namespace levishematic::util {

// ================================================================
// Java → BE 方块名映射
//
// 对于大多数方块，Java 和 BE 使用相同的 ID。
// 此表只包含有差异的方块。
//
// 映射来源：
//   - Minecraft Wiki 方块 ID 对照表
//   - GeyserMC 映射数据
// ================================================================

// 获取映射表的单例引用
const std::unordered_map<std::string, std::string>& getJavaToBedrockMap();

// 将 Java 方块名转换为 BE 方块名
// 输入格式：不含属性的方块名，如 "minecraft:grass_block"
// 若无映射则原样返回
std::string javaToBedrock(std::string_view javaName);

// ================================================================
// Java blockstate 属性解析
//
// Java blockstate 格式：
//   "minecraft:oak_log[axis=y]"
//   "minecraft:oak_stairs[facing=north,half=bottom,shape=straight,waterlogged=false]"
//
// 解析为：
//   name = "minecraft:oak_log"
//   properties = { "axis": "y" }
// ================================================================
struct JavaBlockState {
    std::string                                   name;       // 如 "minecraft:oak_log"
    std::unordered_map<std::string, std::string>  properties; // 如 { "axis": "y" }

    // 从完整的 blockstate 字符串解析
    // 如 "minecraft:oak_log[axis=y]"
    static JavaBlockState parse(std::string_view fullState);

    // 判断是否是空气
    bool isAir() const;
};

// ================================================================
// 通过 Java blockstate 查找对应的 BE Block*
//
// 流程：
//   1. 解析 Java blockstate → name + properties
//   2. javaToBedrock(name) → BE name
//   3. 通过 BlockTypeRegistry 查找 BE Block
//   4. 若有属性，尝试匹配最合适的 Block 状态
//
// 返回 nullptr 表示未找到对应方块（回退为 air）
// ================================================================
const Block* resolveJavaBlockState(const JavaBlockState& state);

// 便利函数：直接从 blockstate 字符串解析并查找
const Block* resolveJavaBlockState(std::string_view fullState);

} // namespace levishematic::util
