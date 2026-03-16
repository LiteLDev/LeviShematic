#include "LitematicSchematic.h"

#include "levishematic/schematic/NBTReader.h"
#include "levishematic/util/BlockUtils.h"

#include "mc/nbt/CompoundTag.h"
#include "mc/nbt/CompoundTagVariant.h"
#include "mc/nbt/Int64Tag.h"
#include "mc/nbt/IntTag.h"
#include "mc/nbt/ListTag.h"
#include "mc/nbt/StringTag.h"

#include "zlib.h"

#include <cstdint>
#include <cstring>
#include <fstream>
#include <stdexcept>
#include <vector>

namespace levishematic::schematic {

// ================================================================
// Big-Endian 读取工具（Java NBT 是 Big-Endian）
// ================================================================
namespace be {

inline uint8_t readU8(const uint8_t*& p) {
    return *p++;
}

inline int16_t readI16(const uint8_t*& p) {
    int16_t v = static_cast<int16_t>((static_cast<uint16_t>(p[0]) << 8) | p[1]);
    p += 2;
    return v;
}

inline int32_t readI32(const uint8_t*& p) {
    int32_t v = (static_cast<int32_t>(p[0]) << 24)
              | (static_cast<int32_t>(p[1]) << 16)
              | (static_cast<int32_t>(p[2]) << 8)
              | static_cast<int32_t>(p[3]);
    p += 4;
    return v;
}

inline int64_t readI64(const uint8_t*& p) {
    int64_t v = (static_cast<int64_t>(p[0]) << 56)
              | (static_cast<int64_t>(p[1]) << 48)
              | (static_cast<int64_t>(p[2]) << 40)
              | (static_cast<int64_t>(p[3]) << 32)
              | (static_cast<int64_t>(p[4]) << 24)
              | (static_cast<int64_t>(p[5]) << 16)
              | (static_cast<int64_t>(p[6]) << 8)
              | static_cast<int64_t>(p[7]);
    p += 8;
    return v;
}

inline float readF32(const uint8_t*& p) {
    int32_t i = readI32(p);
    float f;
    std::memcpy(&f, &i, 4);
    return f;
}

inline double readF64(const uint8_t*& p) {
    int64_t i = readI64(p);
    double d;
    std::memcpy(&d, &i, 8);
    return d;
}

inline std::string readString(const uint8_t*& p) {
    uint16_t len = static_cast<uint16_t>(readI16(p));
    std::string s(reinterpret_cast<const char*>(p), len);
    p += len;
    return s;
}

} // namespace be

// ================================================================
// 递归 NBT 解析器（支持 TAG_Long_Array = type 12）
//
// 策略：
//   完全手动解析 Big-Endian NBT 二进制流。
//   遇到 TAG_Long_Array 时记录路径和数据，
//   其他所有类型正常解析为 CompoundTag 树。
//
// TAG_Long_Array 格式：
//   type(1 byte) = 12
//   name(2+N bytes) = string
//   length(4 bytes) = int32 (元素数量)
//   data(8*length bytes) = int64[]
// ================================================================

// 前向声明
static void skipTag(int type, const uint8_t*& p, const uint8_t* end);
static CompoundTagVariant readTagPayload(int type, const uint8_t*& p, const uint8_t* end,
    const std::string& currentPath,
    std::unordered_map<std::string, std::vector<int64_t>>& longArrays);

static void skipTag(int type, const uint8_t*& p, const uint8_t* end) {
    switch (type) {
    case 0: break; // End
    case 1: p += 1; break; // Byte
    case 2: p += 2; break; // Short
    case 3: p += 4; break; // Int
    case 4: p += 8; break; // Int64
    case 5: p += 4; break; // Float
    case 6: p += 8; break; // Double
    case 7: { // ByteArray
        int32_t len = be::readI32(p);
        p += len;
        break;
    }
    case 8: { // String
        int16_t len = be::readI16(p);
        p += len;
        break;
    }
    case 9: { // List
        int8_t elemType = be::readU8(p);
        int32_t count = be::readI32(p);
        for (int32_t i = 0; i < count; ++i) {
            skipTag(elemType, p, end);
        }
        break;
    }
    case 10: { // Compound
        while (true) {
            int8_t childType = be::readU8(p);
            if (childType == 0) break;
            // skip name
            int16_t nameLen = be::readI16(p);
            p += nameLen;
            skipTag(childType, p, end);
        }
        break;
    }
    case 11: { // IntArray
        int32_t len = be::readI32(p);
        p += 4 * len;
        break;
    }
    case 12: { // LongArray
        int32_t len = be::readI32(p);
        p += 8 * len;
        break;
    }
    default:
        throw std::runtime_error("Unknown NBT tag type: " + std::to_string(type));
    }
}

static CompoundTagVariant readTagPayload(int type, const uint8_t*& p, const uint8_t* end,
    const std::string& currentPath,
    std::unordered_map<std::string, std::vector<int64_t>>& longArrays)
{
    switch (type) {
    case 1: { // Byte
        int8_t v = static_cast<int8_t>(be::readU8(p));
        return CompoundTagVariant(ByteTag(v));
    }
    case 2: { // Short
        int16_t v = be::readI16(p);
        return CompoundTagVariant(ShortTag(v));
    }
    case 3: { // Int
        int32_t v = be::readI32(p);
        return CompoundTagVariant(IntTag(v));
    }
    case 4: { // Int64
        int64_t v = be::readI64(p);
        return CompoundTagVariant(Int64Tag(v));
    }
    case 5: { // Float
        float v = be::readF32(p);
        return CompoundTagVariant(FloatTag(v));
    }
    case 6: { // Double
        double v = be::readF64(p);
        return CompoundTagVariant(DoubleTag(v));
    }
    case 7: { // ByteArray
        int32_t len = be::readI32(p);
        ByteArrayTag arr;
        arr.resize(len);
        for (int32_t i = 0; i < len; ++i) {
            arr[i] = static_cast<unsigned char>(be::readU8(p));
        }
        return CompoundTagVariant(std::move(arr));
    }
    case 8: { // String
        std::string s = be::readString(p);
        return CompoundTagVariant(StringTag(std::move(s)));
    }
    case 9: { // List
        int8_t elemType = be::readU8(p);
        int32_t count = be::readI32(p);
        ListTag list;
        for (int32_t i = 0; i < count; ++i) {
            std::string childPath = currentPath + "[" + std::to_string(i) + "]";
            auto variant = readTagPayload(elemType, p, end, childPath, longArrays);
            list.add(std::move(variant).toUnique());
        }
        list.mType = static_cast<Tag::Type>(elemType);
        return CompoundTagVariant(std::move(list));
    }
    case 10: { // Compound
        CompoundTag compound;
        while (true) {
            int8_t childType = be::readU8(p);
            if (childType == 0) break; // End tag
            std::string childName = be::readString(p);
            std::string childPath = currentPath + "." + childName;

            if (childType == 12) {
                // TAG_Long_Array — 手动提取并存储到 longArrays
                int32_t len = be::readI32(p);
                std::vector<int64_t> arr(len);
                for (int32_t i = 0; i < len; ++i) {
                    arr[i] = be::readI64(p);
                }
                longArrays[currentPath + "." + childName] = std::move(arr);
                // 在 CompoundTag 中放入一个占位的 IntTag(0)
                compound.mTags[childName] = CompoundTagVariant(IntTag(0));
            } else {
                auto variant = readTagPayload(childType, p, end, childPath, longArrays);
                compound.mTags[childName] = std::move(variant);
            }
        }
        return CompoundTagVariant(std::move(compound));
    }
    case 11: { // IntArray
        int32_t len = be::readI32(p);
        IntArrayTag arr;
        arr.resize(len);
        for (int32_t i = 0; i < len; ++i) {
            arr[i] = be::readI32(p);
        }
        return CompoundTagVariant(std::move(arr));
    }
    case 12: { // LongArray (should not reach here for named tags)
        int32_t len = be::readI32(p);
        std::vector<int64_t> arr(len);
        for (int32_t i = 0; i < len; ++i) {
            arr[i] = be::readI64(p);
        }
        longArrays[currentPath] = std::move(arr);
        return CompoundTagVariant(IntTag(0)); // 占位
    }
    default:
        throw std::runtime_error("Unknown NBT tag type in payload: " + std::to_string(type));
    }
}

// ================================================================
// 解析完整的 NBT 流（从根 TAG_Compound 开始）
//
// Java NBT 二进制格式：
//   byte    tagType  (必须是 10 = Compound)
//   string  rootName (通常是空字符串)
//   ...     compound payload
// ================================================================
struct FullNBTParseResult {
    CompoundTag root;
    std::unordered_map<std::string, std::vector<int64_t>> longArrays;
};

static FullNBTParseResult parseFullNBT(const std::string& nbtData) {
    const uint8_t* p   = reinterpret_cast<const uint8_t*>(nbtData.data());
    const uint8_t* end = p + nbtData.size();

    // 读取根标签类型
    int8_t rootType = be::readU8(p);
    if (rootType != 10) {
        throw std::runtime_error("Root tag is not Compound (type=" + std::to_string(rootType) + ")");
    }

    // 读取根标签名称（通常为空）
    std::string rootName = be::readString(p);

    // 解析根 Compound
    FullNBTParseResult result;
    auto variant = readTagPayload(10, p, end, "", result.longArrays);
    result.root = std::move(variant.get<CompoundTag>());

    return result;
}

// ================================================================
// GZip 解压（复用 NBTReader 中的实现）
// ================================================================
static std::string gzipDecompress(const void* src, size_t srcLen) {
    std::string out;
    out.resize(srcLen * 4);

    z_stream strm{};
    strm.next_in  = reinterpret_cast<Bytef*>(const_cast<void*>(src));
    strm.avail_in = static_cast<uInt>(srcLen);

    if (inflateInit2(&strm, 15 + 16) != Z_OK) {
        throw std::runtime_error("inflateInit2 failed");
    }

    while (true) {
        strm.next_out  = reinterpret_cast<Bytef*>(out.data() + strm.total_out);
        strm.avail_out = static_cast<uInt>(out.size() - strm.total_out);

        int ret = inflate(&strm, Z_NO_FLUSH);
        if (ret == Z_STREAM_END) break;
        if (ret == Z_BUF_ERROR || strm.avail_out == 0) {
            out.resize(out.size() * 2);
        } else if (ret != Z_OK) {
            inflateEnd(&strm);
            throw std::runtime_error(std::string("inflate failed: ") + (strm.msg ? strm.msg : "unknown"));
        }
    }
    out.resize(strm.total_out);
    inflateEnd(&strm);
    return out;
}

// ================================================================
// LitematicSchematic 实现
// ================================================================

bool LitematicSchematic::loadFromFile(const std::filesystem::path& path) {
    try {
        // 1. 读取原始字节
        std::ifstream file(path, std::ios::binary | std::ios::ate);
        if (!file.is_open()) {
            throw std::runtime_error("Cannot open file: " + path.string());
        }

        auto fileSize = static_cast<size_t>(file.tellg());
        file.seekg(0, std::ios::beg);

        std::vector<uint8_t> rawData(fileSize);
        if (!file.read(reinterpret_cast<char*>(rawData.data()), static_cast<std::streamsize>(fileSize))) {
            throw std::runtime_error("Failed to read file: " + path.string());
        }

        // 2. GZip 解压
        std::string nbtData = gzipDecompress(rawData.data(), rawData.size());

        // 3. 手动解析 NBT（支持 TAG_Long_Array）
        auto parseResult = parseFullNBT(nbtData);

        // 4. 提取 BlockStates long arrays（按 region 名称索引）
        // longArrays 的 key 格式为 ".Regions.regionName.BlockStates"
        std::unordered_map<std::string, std::vector<int64_t>> regionBlockStates;
        for (const auto& [path_key, data] : parseResult.longArrays) {
            // 查找 ".Regions." 开头的路径
            const std::string prefix = ".Regions.";
            auto pos = path_key.find(prefix);
            if (pos == std::string::npos) continue;

            auto rest = path_key.substr(pos + prefix.size());
            auto dotPos = rest.find('.');
            if (dotPos == std::string::npos) continue;

            std::string regionName = rest.substr(0, dotPos);
            std::string fieldName = rest.substr(dotPos + 1);

            if (fieldName == "BlockStates") {
                regionBlockStates[regionName] = data;
            }
        }

        // 5. 加载
        return loadFromNBT(parseResult.root, regionBlockStates);

    } catch (const std::exception&) {
        mLoaded = false;
        return false;
    }
}

bool LitematicSchematic::loadFromNBT(
    const CompoundTag& root,
    const std::unordered_map<std::string, std::vector<int64_t>>& longArrays)
{
    try {
        // 读取版本
        if (root.contains("Version", Tag::Type::Int)) {
            mVersion = static_cast<int>(root["Version"].get<IntTag>());
        }

        // 解析 Metadata
        if (root.contains("Metadata", Tag::Type::Compound)) {
            parseMetadata(root["Metadata"].get<CompoundTag>());
        }

        // 解析 Regions
        mRegions.clear();
        if (root.contains("Regions", Tag::Type::Compound)) {
            const auto& regionsTag = root["Regions"].get<CompoundTag>();

            for (const auto& [regionName, regionVariant] : regionsTag.mTags) {
                if (regionVariant.index() != Tag::Type::Compound) continue;

                const auto& regionTag = regionVariant.get<CompoundTag>();

                // 查找该 region 的 BlockStates long array
                std::vector<int64_t> blockStatesData;
                auto it = longArrays.find(regionName);
                if (it != longArrays.end()) {
                    blockStatesData = it->second;
                }

                mRegions.push_back(parseRegion(regionName, regionTag, blockStatesData));
            }
        }

        mLoaded = !mRegions.empty();
        return mLoaded;

    } catch (const std::exception&) {
        mLoaded = false;
        return false;
    }
}

void LitematicSchematic::parseMetadata(const CompoundTag& metaTag) {
    auto getString = [&](const char* key) -> std::string {
        if (metaTag.contains(key, Tag::Type::String)) {
            return static_cast<std::string>(metaTag[key].get<StringTag>());
        }
        return {};
    };

    auto getInt = [&](const char* key) -> int {
        if (metaTag.contains(key, Tag::Type::Int)) {
            return static_cast<int>(metaTag[key].get<IntTag>());
        }
        return 0;
    };

    auto getLong = [&](const char* key) -> int64_t {
        if (metaTag.contains(key, Tag::Type::Int64)) {
            return static_cast<int64_t>(metaTag[key].get<Int64Tag>());
        }
        return 0;
    };

    mMetadata.name         = getString("Name");
    mMetadata.author       = getString("Author");
    mMetadata.description  = getString("Description");
    mMetadata.timeCreated  = getLong("TimeCreated");
    mMetadata.timeModified = getLong("TimeModified");
    mMetadata.totalBlocks  = getInt("TotalBlocks");
    mMetadata.totalVolume  = getInt("TotalVolume");
    mMetadata.regionCount  = getInt("RegionCount");

    // EnclosingSize
    if (metaTag.contains("EnclosingSize", Tag::Type::Compound)) {
        const auto& sizeTag = metaTag["EnclosingSize"].get<CompoundTag>();
        mMetadata.enclosingSize = {
            sizeTag.contains("x") ? static_cast<int>(sizeTag["x"].get<IntTag>()) : 0,
            sizeTag.contains("y") ? static_cast<int>(sizeTag["y"].get<IntTag>()) : 0,
            sizeTag.contains("z") ? static_cast<int>(sizeTag["z"].get<IntTag>()) : 0
        };
    }
}

RegionData LitematicSchematic::parseRegion(
    const std::string& name,
    const CompoundTag& regionTag,
    const std::vector<int64_t>& blockStatesData)
{
    RegionData region;
    region.name = name;

    // Position
    if (regionTag.contains("Position", Tag::Type::Compound)) {
        const auto& posTag = regionTag["Position"].get<CompoundTag>();
        region.position = {
            posTag.contains("x") ? static_cast<int>(posTag["x"].get<IntTag>()) : 0,
            posTag.contains("y") ? static_cast<int>(posTag["y"].get<IntTag>()) : 0,
            posTag.contains("z") ? static_cast<int>(posTag["z"].get<IntTag>()) : 0
        };
    }

    // Size
    if (regionTag.contains("Size", Tag::Type::Compound)) {
        const auto& sizeTag = regionTag["Size"].get<CompoundTag>();
        region.rawSize = {
            sizeTag.contains("x") ? static_cast<int>(sizeTag["x"].get<IntTag>()) : 0,
            sizeTag.contains("y") ? static_cast<int>(sizeTag["y"].get<IntTag>()) : 0,
            sizeTag.contains("z") ? static_cast<int>(sizeTag["z"].get<IntTag>()) : 0
        };
    }

    // BlockStatePalette
    if (regionTag.contains("BlockStatePalette", Tag::Type::List)) {
        const auto& paletteList = regionTag["BlockStatePalette"].get<ListTag>();
        region.palette.loadFromNBT(paletteList);
    }

    // BlockStates（从手动提取的 long array）
    if (!blockStatesData.empty() && !region.palette.empty()) {
        int bitsPerBlock = region.palette.getBitsPerBlock();
        int totalBlocks  = region.totalBlocks();
        region.blockStates.init(blockStatesData, bitsPerBlock, totalBlocks);
    }

    return region;
}

const RegionData* LitematicSchematic::findRegion(const std::string& name) const {
    for (const auto& region : mRegions) {
        if (region.name == name) return &region;
    }
    return nullptr;
}

std::vector<std::string> LitematicSchematic::getRegionNames() const {
    std::vector<std::string> names;
    names.reserve(mRegions.size());
    for (const auto& region : mRegions) {
        names.push_back(region.name);
    }
    return names;
}

int LitematicSchematic::getTotalNonAirBlocks() const {
    int count = 0;
    for (const auto& region : mRegions) {
        int sx = region.sizeX(), sy = region.sizeY(), sz = region.sizeZ();
        for (int y = 0; y < sy; ++y)
        for (int z = 0; z < sz; ++z)
        for (int x = 0; x < sx; ++x) {
            if (!region.isAir(x, y, z)) ++count;
        }
    }
    return count;
}

// ================================================================
// 便利加载函数
// ================================================================
LitematicSchematic loadLitematicFile(const std::filesystem::path& path) {
    LitematicSchematic schem;
    schem.loadFromFile(path);
    return schem;
}

} // namespace levishematic::schematic
