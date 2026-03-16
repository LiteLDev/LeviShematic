#include "BlockStatePalette.h"

#include "mc/nbt/CompoundTag.h"
#include "mc/nbt/CompoundTagVariant.h"
#include "mc/nbt/ListTag.h"
#include "mc/nbt/StringTag.h"

#include <cmath>
#include <stdexcept>

namespace levishematic::schematic {

// ================================================================
// 从 NBT ListTag 构建调色板
//
// NBT 结构（.litematic）：
//   TAG_List("BlockStatePalette") [
//     TAG_Compound {
//       "Name": TAG_String("minecraft:air")
//     },
//     TAG_Compound {
//       "Name": TAG_String("minecraft:oak_log")
//       "Properties": TAG_Compound {
//         "axis": TAG_String("y")
//       }
//     },
//     ...
//   ]
//
// LL API：
//   ListTag extends std::vector<UniqueTagPtr>
//   CompoundTag has mTags (std::map<string, CompoundTagVariant>)
//   StringTag extends std::string
// ================================================================
void BlockStatePalette::loadFromNBT(const ListTag& paletteList) {
    mEntries.clear();
    mEntries.reserve(paletteList.size());

    for (size_t i = 0; i < paletteList.size(); ++i) {
        // ListTag[i] 是 UniqueTagPtr，通过 get(int) 获取 Tag*
        const auto* tag = paletteList.get(static_cast<int>(i));
        if (!tag || tag->getId() != Tag::Type::Compound) {
            // 异常条目：填充为 air
            PaletteEntry entry;
            entry.javaState.name = "minecraft:air";
            entry.bedrockBlock   = nullptr;
            entry.isAir          = true;
            mEntries.push_back(entry);
            continue;
        }

        const auto& compound = static_cast<const CompoundTag&>(*tag);
        PaletteEntry entry;

        // 读取 Name
        if (compound.contains("Name", Tag::Type::String)) {
            const auto& nameVariant = compound["Name"];
            entry.javaState.name = static_cast<std::string>(nameVariant.get<StringTag>());
        } else {
            entry.javaState.name = "minecraft:air";
        }

        // 读取 Properties（可选）
        bool hasProps = compound.contains("Properties", Tag::Type::Compound);
        if (hasProps) {
            const auto& propsVariant = compound["Properties"];
            const auto& propsCompound = propsVariant.get<CompoundTag>();

            // 构建 "name[key=value,key2=value2]" 格式的完整 blockstate 字符串
            std::string fullState = entry.javaState.name + "[";
            bool first = true;

            // CompoundTag::mTags 是 std::map<string, CompoundTagVariant>
            for (const auto& [key, value] : propsCompound.mTags) {
                if (!first) fullState += ",";
                first = false;

                // Properties 中的值通常是 StringTag
                if (value.index() == Tag::Type::String) {
                    fullState += key + "=" + static_cast<std::string>(value.get<StringTag>());
                } else {
                    fullState += key + "=" + value.get().toString();
                }
            }
            fullState += "]";

            entry.javaState = util::JavaBlockState::parse(fullState);
        }

        // 判断是否是空气
        entry.isAir = entry.javaState.isAir();

        // 解析为 BE Block*
        entry.bedrockBlock = util::resolveJavaBlockState(entry.javaState);

        mEntries.push_back(entry);
    }
}

const PaletteEntry& BlockStatePalette::getEntry(int index) const {
    if (index < 0 || static_cast<size_t>(index) >= mEntries.size()) {
        static const PaletteEntry airEntry = {
            util::JavaBlockState{"minecraft:air", {}},
            nullptr,
            true
        };
        return airEntry;
    }
    return mEntries[static_cast<size_t>(index)];
}

const Block* BlockStatePalette::getBlock(int index) const {
    return getEntry(index).bedrockBlock;
}

int BlockStatePalette::getBitsPerBlock() const {
    if (mEntries.size() <= 1) return 2; // Litematica 最小为 2 bits
    int bits = static_cast<int>(std::ceil(std::log2(static_cast<double>(mEntries.size()))));
    return std::max(bits, 2); // 最小 2 bits
}

} // namespace levishematic::schematic
