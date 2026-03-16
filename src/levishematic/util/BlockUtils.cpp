#include "BlockUtils.h"

#include "mc/world/level/block/registry/BlockTypeRegistry.h"

#include <sstream>

namespace levishematic::util {

// ================================================================
// Java → Bedrock 方块名差异映射表
//
// 此表仅包含名称不同的方块。
// 格式：{ "java_name", "bedrock_name" }
//
// 参考来源：
//   - GeyserMC mappings
//   - Minecraft Wiki
//   - 实际测试
//
// 注：随着版本更新，BE 也在逐步统一命名，
//     部分旧映射可能不再需要。
// ================================================================
const std::unordered_map<std::string, std::string>& getJavaToBedrockMap() {
    static const std::unordered_map<std::string, std::string> map = {
        // ---- 基础方块 ----
        {"minecraft:grass_block",            "minecraft:grass_block"},
        {"minecraft:dirt_path",              "minecraft:grass_path"},
        {"minecraft:rooted_dirt",            "minecraft:dirt_with_roots"},
        {"minecraft:red_sand",               "minecraft:sand"},  // data value 1
        {"minecraft:podzol",                 "minecraft:podzol"},

        // ---- 石头变种 ----
        {"minecraft:infested_stone",              "minecraft:monster_egg"},
        {"minecraft:infested_cobblestone",         "minecraft:monster_egg"},
        {"minecraft:infested_stone_bricks",        "minecraft:monster_egg"},
        {"minecraft:infested_mossy_stone_bricks",  "minecraft:monster_egg"},
        {"minecraft:infested_cracked_stone_bricks","minecraft:monster_egg"},
        {"minecraft:infested_chiseled_stone_bricks","minecraft:monster_egg"},
        {"minecraft:infested_deepslate",           "minecraft:infested_deepslate"},

        // ---- 木头 / 原木 ----
        {"minecraft:stripped_oak_log",       "minecraft:stripped_oak_log"},
        {"minecraft:stripped_spruce_log",    "minecraft:stripped_spruce_log"},
        {"minecraft:stripped_birch_log",     "minecraft:stripped_birch_log"},
        {"minecraft:stripped_jungle_log",    "minecraft:stripped_jungle_log"},
        {"minecraft:stripped_acacia_log",    "minecraft:stripped_acacia_log"},
        {"minecraft:stripped_dark_oak_log",  "minecraft:stripped_dark_oak_log"},

        // ---- 叶子 ----
        {"minecraft:oak_leaves",             "minecraft:oak_leaves"},
        {"minecraft:spruce_leaves",          "minecraft:spruce_leaves"},
        {"minecraft:birch_leaves",           "minecraft:birch_leaves"},
        {"minecraft:jungle_leaves",          "minecraft:jungle_leaves"},

        // ---- 花 / 植物 ----
        {"minecraft:dandelion",              "minecraft:yellow_flower"},
        {"minecraft:poppy",                  "minecraft:red_flower"},
        {"minecraft:blue_orchid",            "minecraft:red_flower"},
        {"minecraft:allium",                 "minecraft:red_flower"},
        {"minecraft:azure_bluet",            "minecraft:red_flower"},
        {"minecraft:red_tulip",              "minecraft:red_flower"},
        {"minecraft:orange_tulip",           "minecraft:red_flower"},
        {"minecraft:white_tulip",            "minecraft:red_flower"},
        {"minecraft:pink_tulip",             "minecraft:red_flower"},
        {"minecraft:oxeye_daisy",            "minecraft:red_flower"},
        {"minecraft:cornflower",             "minecraft:red_flower"},
        {"minecraft:lily_of_the_valley",     "minecraft:red_flower"},
        {"minecraft:tall_grass",             "minecraft:double_plant"},
        {"minecraft:large_fern",             "minecraft:double_plant"},
        {"minecraft:sunflower",              "minecraft:double_plant"},
        {"minecraft:lilac",                  "minecraft:double_plant"},
        {"minecraft:rose_bush",              "minecraft:double_plant"},
        {"minecraft:peony",                  "minecraft:double_plant"},
        {"minecraft:short_grass",            "minecraft:short_grass"},
        {"minecraft:fern",                   "minecraft:fern"},
        {"minecraft:dead_bush",              "minecraft:deadbush"},
        {"minecraft:sugar_cane",             "minecraft:reeds"},
        {"minecraft:lily_pad",               "minecraft:waterlily"},

        // ---- 红石 ----
        {"minecraft:redstone_wall_torch",    "minecraft:redstone_torch"},
        {"minecraft:wall_torch",             "minecraft:torch"},
        {"minecraft:piston_head",            "minecraft:pistonarmcollision"},
        {"minecraft:moving_piston",          "minecraft:movingblock"},
        {"minecraft:redstone_wire",          "minecraft:redstone_wire"},
        {"minecraft:repeater",               "minecraft:unpowered_repeater"},
        {"minecraft:comparator",             "minecraft:unpowered_comparator"},
        {"minecraft:note_block",             "minecraft:noteblock"},
        {"minecraft:jack_o_lantern",         "minecraft:lit_pumpkin"},

        // ---- 光源 ----
        {"minecraft:torch",                  "minecraft:torch"},
        {"minecraft:lantern",                "minecraft:lantern"},
        {"minecraft:soul_lantern",           "minecraft:soul_lantern"},
        {"minecraft:soul_torch",             "minecraft:soul_torch"},

        // ---- 水/岩浆 ----
        {"minecraft:water",                  "minecraft:water"},
        {"minecraft:lava",                   "minecraft:lava"},

        // ---- 台阶（双台阶差异）----
        {"minecraft:smooth_stone_slab",      "minecraft:stone_block_slab"},

        // ---- 告示牌 ----
        {"minecraft:oak_sign",               "minecraft:standing_sign"},
        {"minecraft:oak_wall_sign",          "minecraft:wall_sign"},
        {"minecraft:spruce_sign",            "minecraft:spruce_standing_sign"},
        {"minecraft:spruce_wall_sign",       "minecraft:spruce_wall_sign"},
        {"minecraft:birch_sign",             "minecraft:birch_standing_sign"},
        {"minecraft:birch_wall_sign",        "minecraft:birch_wall_sign"},
        {"minecraft:jungle_sign",            "minecraft:jungle_standing_sign"},
        {"minecraft:jungle_wall_sign",       "minecraft:jungle_wall_sign"},
        {"minecraft:acacia_sign",            "minecraft:acacia_standing_sign"},
        {"minecraft:acacia_wall_sign",       "minecraft:acacia_wall_sign"},
        {"minecraft:dark_oak_sign",          "minecraft:darkoak_standing_sign"},
        {"minecraft:dark_oak_wall_sign",     "minecraft:darkoak_wall_sign"},

        // ---- 墙 ----
        {"minecraft:cobblestone_wall",       "minecraft:cobblestone_wall"},

        // ---- 门 / 活板门 ----
        {"minecraft:iron_door",              "minecraft:iron_door"},

        // ---- 矿车/铁轨 ----
        {"minecraft:powered_rail",           "minecraft:golden_rail"},
        {"minecraft:detector_rail",          "minecraft:detector_rail"},
        {"minecraft:activator_rail",         "minecraft:activator_rail"},

        // ---- 末地 ----
        {"minecraft:end_stone_brick_slab",   "minecraft:end_brick"},
        {"minecraft:end_portal_frame",       "minecraft:end_portal_frame"},
        {"minecraft:ender_chest",            "minecraft:ender_chest"},

        // ---- 地狱 ----
        {"minecraft:nether_bricks",          "minecraft:nether_brick"},
        {"minecraft:nether_brick_fence",     "minecraft:nether_brick_fence"},
        {"minecraft:nether_brick_stairs",    "minecraft:nether_brick_stairs"},
        {"minecraft:nether_wart",            "minecraft:nether_wart"},
        {"minecraft:nether_portal",          "minecraft:portal"},

        // ---- 附魔/工作台等 ----
        {"minecraft:enchanting_table",       "minecraft:enchanting_table"},
        {"minecraft:crafting_table",         "minecraft:crafting_table"},
        {"minecraft:brewing_stand",          "minecraft:brewing_stand"},

        // ---- 头颅 ----
        {"minecraft:skeleton_skull",         "minecraft:skull"},
        {"minecraft:skeleton_wall_skull",    "minecraft:skull"},
        {"minecraft:wither_skeleton_skull",  "minecraft:skull"},
        {"minecraft:wither_skeleton_wall_skull", "minecraft:skull"},
        {"minecraft:zombie_head",            "minecraft:skull"},
        {"minecraft:zombie_wall_head",       "minecraft:skull"},
        {"minecraft:player_head",            "minecraft:skull"},
        {"minecraft:player_wall_head",       "minecraft:skull"},
        {"minecraft:creeper_head",           "minecraft:skull"},
        {"minecraft:creeper_wall_head",      "minecraft:skull"},
        {"minecraft:dragon_head",            "minecraft:skull"},
        {"minecraft:dragon_wall_head",       "minecraft:skull"},

        // ---- 旗帜 ----
        {"minecraft:white_banner",           "minecraft:standing_banner"},
        {"minecraft:white_wall_banner",      "minecraft:wall_banner"},

        // ---- 混凝土粉末 ----
        {"minecraft:white_concrete_powder",  "minecraft:concrete_powder"},
        {"minecraft:orange_concrete_powder", "minecraft:concrete_powder"},
        {"minecraft:magenta_concrete_powder","minecraft:concrete_powder"},

        // ---- 床 ----
        {"minecraft:white_bed",              "minecraft:bed"},
        {"minecraft:orange_bed",             "minecraft:bed"},
        {"minecraft:magenta_bed",            "minecraft:bed"},
        {"minecraft:light_blue_bed",         "minecraft:bed"},
        {"minecraft:yellow_bed",             "minecraft:bed"},
        {"minecraft:lime_bed",              "minecraft:bed"},
        {"minecraft:pink_bed",              "minecraft:bed"},
        {"minecraft:gray_bed",              "minecraft:bed"},
        {"minecraft:light_gray_bed",        "minecraft:bed"},
        {"minecraft:cyan_bed",              "minecraft:bed"},
        {"minecraft:purple_bed",            "minecraft:bed"},
        {"minecraft:blue_bed",              "minecraft:bed"},
        {"minecraft:brown_bed",             "minecraft:bed"},
        {"minecraft:green_bed",             "minecraft:bed"},
        {"minecraft:red_bed",               "minecraft:bed"},
        {"minecraft:black_bed",             "minecraft:bed"},

        // ---- 染色玻璃 ----
        {"minecraft:white_stained_glass",    "minecraft:stained_glass"},
        {"minecraft:orange_stained_glass",   "minecraft:stained_glass"},
        {"minecraft:magenta_stained_glass",  "minecraft:stained_glass"},

        // ---- 羊毛 ----
        {"minecraft:white_wool",             "minecraft:wool"},
        {"minecraft:orange_wool",            "minecraft:wool"},
        {"minecraft:magenta_wool",           "minecraft:wool"},
        {"minecraft:light_blue_wool",        "minecraft:wool"},
        {"minecraft:yellow_wool",            "minecraft:wool"},
        {"minecraft:lime_wool",             "minecraft:wool"},
        {"minecraft:pink_wool",             "minecraft:wool"},
        {"minecraft:gray_wool",             "minecraft:wool"},
        {"minecraft:light_gray_wool",       "minecraft:wool"},
        {"minecraft:cyan_wool",             "minecraft:wool"},
        {"minecraft:purple_wool",           "minecraft:wool"},
        {"minecraft:blue_wool",             "minecraft:wool"},
        {"minecraft:brown_wool",            "minecraft:wool"},
        {"minecraft:green_wool",            "minecraft:wool"},
        {"minecraft:red_wool",              "minecraft:wool"},
        {"minecraft:black_wool",            "minecraft:wool"},

        // ---- 其他 ----
        {"minecraft:spawner",                "minecraft:mob_spawner"},
        {"minecraft:cobweb",                 "minecraft:web"},
        {"minecraft:farmland",               "minecraft:farmland"},
        {"minecraft:snow_block",             "minecraft:snow"},
        {"minecraft:snow",                   "minecraft:snow_layer"},
        {"minecraft:cave_air",               "minecraft:air"},
        {"minecraft:void_air",               "minecraft:air"},
        {"minecraft:wall_sign",              "minecraft:wall_sign"},
        {"minecraft:melon",                  "minecraft:melon_block"},
        {"minecraft:bricks",                 "minecraft:brick_block"},
        {"minecraft:chiseled_quartz_block",  "minecraft:quartz_block"},
        {"minecraft:smooth_quartz",          "minecraft:quartz_block"},
        {"minecraft:map",                    "minecraft:filled_map"},
    };
    return map;
}

// ================================================================
// javaToBedrock 实现
// ================================================================
std::string javaToBedrock(std::string_view javaName) {
    const auto& map = getJavaToBedrockMap();
    std::string key(javaName);
    auto it = map.find(key);
    if (it != map.end()) {
        return it->second;
    }
    return key; // 无映射，原样返回
}

// ================================================================
// JavaBlockState 解析实现
// ================================================================
JavaBlockState JavaBlockState::parse(std::string_view fullState) {
    JavaBlockState result;

    // 查找 '[' 位置
    auto bracketPos = fullState.find('[');
    if (bracketPos == std::string_view::npos) {
        // 没有属性
        result.name = std::string(fullState);
        return result;
    }

    result.name = std::string(fullState.substr(0, bracketPos));

    // 解析 [key=value,key=value,...] 部分
    auto propsStr = fullState.substr(bracketPos + 1);
    // 去掉末尾的 ']'
    if (!propsStr.empty() && propsStr.back() == ']') {
        propsStr = propsStr.substr(0, propsStr.size() - 1);
    }

    // 按 ',' 分割
    size_t start = 0;
    while (start < propsStr.size()) {
        auto commaPos = propsStr.find(',', start);
        auto pairStr = propsStr.substr(start, commaPos == std::string_view::npos
                                                   ? std::string_view::npos
                                                   : commaPos - start);

        auto eqPos = pairStr.find('=');
        if (eqPos != std::string_view::npos) {
            std::string key(pairStr.substr(0, eqPos));
            std::string value(pairStr.substr(eqPos + 1));
            result.properties[key] = value;
        }

        if (commaPos == std::string_view::npos) break;
        start = commaPos + 1;
    }

    return result;
}

bool JavaBlockState::isAir() const {
    return name == "minecraft:air"
        || name == "minecraft:cave_air"
        || name == "minecraft:void_air";
}

// ================================================================
// resolveJavaBlockState 实现
// ================================================================
const Block* resolveJavaBlockState(const JavaBlockState& state) {
    if (state.isAir()) {
        auto opt = Block::tryGetFromRegistry(HashedString("minecraft:air"));
        return opt ? &(*opt) : nullptr;
    }

    // 1. 转换名称
    std::string beName = javaToBedrock(state.name);

    // 2. 尝试从注册表获取默认状态
    auto opt = Block::tryGetFromRegistry(HashedString(beName));
    if (opt) {
        return &(*opt);
    }

    // 3. 未找到 → 尝试不带 minecraft: 前缀
    if (beName.starts_with("minecraft:")) {
        auto shortName = beName.substr(10); // 去掉 "minecraft:"
        opt = Block::tryGetFromRegistry(HashedString("minecraft:" + shortName));
        if (opt) return &(*opt);
    }

    // 4. 最终回退：返回 nullptr（调用方应处理为 air）
    return nullptr;
}

const Block* resolveJavaBlockState(std::string_view fullState) {
    return resolveJavaBlockState(JavaBlockState::parse(fullState));
}

} // namespace levishematic::util
