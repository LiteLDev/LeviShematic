#pragma once
// ============================================================
// SchematicTransform.h
// 投影的坐标变换系统：移动 / 旋转 / 镜像
//
// 设计来源（对应 Litematica Java 实现）：
//   - PositionUtils.getTransformedBlockPos(pos, mirror, rotation)
//   - PositionUtils.getTransformedPlacementPosition(...)
//   - SchematicPlacement / SubRegionPlacement 的 rotation + mirror 组合
//
// 旋转语义（绕 Y 轴，俯视顺时针为正）：
//   NONE          → 原样
//   CW_90         → 顺时针 90°  (x,z) → (z, -x)
//   CW_180        → 顺时针 180° (x,z) → (-x,-z)
//   CCW_90        → 逆时针 90°  (x,z) → (-z, x)
//
// 镜像语义（轴向）：
//   NONE          → 原样
//   MIRROR_X      → 沿 X 轴镜像 (x → -x)
//   MIRROR_Z      → 沿 Z 轴镜像 (z → -z)
//
// 变换顺序（与 Litematica 一致）：先镜像，再旋转
// ============================================================

#include "levishematic/schematic/LitematicSchematic.h"
#include "levishematic/util/BlockUtils.h"

#include "mc/world/level/BlockPos.h"
#include "mc/world/level/block/Block.h"

#include <cmath>
#include <string>
#include <vector>

namespace levishematic::transform {

// ============================================================
// 旋转枚举（绕 Y 轴，对应 Java Rotation）
// ============================================================
enum class Rotation : int {
    NONE   = 0,
    CW_90  = 1,
    CW_180 = 2,
    CCW_90 = 3,
};

// ============================================================
// 镜像枚举（对应 Java Mirror）
// ============================================================
enum class Mirror : int {
    NONE     = 0,
    MIRROR_X = 1,
    MIRROR_Z = 2,
};

// ============================================================
// 旋转工具
// ============================================================
inline Rotation rotateBy(Rotation base, Rotation delta) {
    return static_cast<Rotation>((static_cast<int>(base) + static_cast<int>(delta)) % 4);
}

inline Rotation rotationFromString(const std::string& s) {
    if (s == "CW_90" || s == "CLOCKWISE_90")        return Rotation::CW_90;
    if (s == "CW_180" || s == "CLOCKWISE_180")       return Rotation::CW_180;
    if (s == "CCW_90" || s == "COUNTERCLOCKWISE_90") return Rotation::CCW_90;
    return Rotation::NONE;
}

inline Mirror mirrorFromString(const std::string& s) {
    if (s == "MIRROR_X" || s == "LEFT_RIGHT") return Mirror::MIRROR_X;
    if (s == "MIRROR_Z" || s == "FRONT_BACK") return Mirror::MIRROR_Z;
    return Mirror::NONE;
}

// ============================================================
// 核心变换函数
//
// 先镜像，再旋转（绕局部原点）
// ============================================================
inline BlockPos transformLocalPos(
    BlockPos pos,
    int sizeX, int sizeZ,
    Mirror mirror, Rotation rot)
{
    int x = pos.x, y = pos.y, z = pos.z;

    // Step 1: 镜像
    switch (mirror) {
    case Mirror::MIRROR_X: x = sizeX - 1 - x; break;
    case Mirror::MIRROR_Z: z = sizeZ - 1 - z; break;
    default: break;
    }

    // Step 2: 旋转
    int nx = x, nz = z;
    switch (rot) {
    case Rotation::CW_90:
        nx = sizeZ - 1 - z;
        nz = x;
        break;
    case Rotation::CW_180:
        nx = sizeX - 1 - x;
        nz = sizeZ - 1 - z;
        break;
    case Rotation::CCW_90:
        nx = z;
        nz = sizeX - 1 - x;
        break;
    default:
        nx = x;
        nz = z;
        break;
    }

    return BlockPos{nx, y, nz};
}

// ============================================================
// 变换后 region 的尺寸（旋转 90°/270° 时 X 和 Z 互换）
// ============================================================
inline void transformedSize(int sizeX, int sizeZ, Rotation rot,
                             int& outX, int& outZ) {
    switch (rot) {
    case Rotation::CW_90:
    case Rotation::CCW_90:
        outX = sizeZ;
        outZ = sizeX;
        break;
    default:
        outX = sizeX;
        outZ = sizeZ;
        break;
    }
}

// ============================================================
// 完整的投影变换参数
// ============================================================
struct PlacementTransform {
    BlockPos origin;
    Rotation rotation = Rotation::NONE;
    Mirror   mirror   = Mirror::NONE;

    struct SubRegion {
        std::string name;
        BlockPos    offset;
        Rotation    rotation = Rotation::NONE;
        Mirror      mirror   = Mirror::NONE;
        bool        enabled  = true;
    };
    std::vector<SubRegion> subRegions;

    static PlacementTransform simple(BlockPos origin,
                                     Rotation rot = Rotation::NONE,
                                     Mirror mir   = Mirror::NONE) {
        return PlacementTransform{origin, rot, mir};
    }
};

// ============================================================
// 合并主变换和子区域变换
// ============================================================
inline void combinedTransform(
    Rotation mainRot, Mirror mainMir,
    Rotation subRot,  Mirror subMir,
    Rotation& outRot, Mirror& outMir)
{
    outRot = rotateBy(mainRot, subRot);
    outMir = subMir;
    if (subMir != Mirror::NONE &&
        (mainRot == Rotation::CW_90 || mainRot == Rotation::CCW_90)) {
        outMir = (subMir == Mirror::MIRROR_Z) ? Mirror::MIRROR_X : Mirror::MIRROR_Z;
    }
    if (mainMir != Mirror::NONE) {
        if (outMir == mainMir)            outMir = Mirror::NONE;
        else if (outMir == Mirror::NONE)  outMir = mainMir;
    }
}

// ============================================================
// 用于投影渲染的方块条目
// ============================================================
struct BedrockBlockEntry {
    BlockPos     worldPos;
    const Block* block;
};

// ============================================================
// 对整个 Schematic 应用 PlacementTransform，
// 返回变换后的 BedrockBlockEntry 列表（世界绝对坐标）
// ============================================================
inline std::vector<BedrockBlockEntry> toBedrockBlocksTransformed(
    const schematic::LitematicSchematic& schem,
    const PlacementTransform& transform,
    bool skipAir = true)
{
    std::vector<BedrockBlockEntry> result;

    for (const auto& region : schem.getRegions()) {
        // 查找子区域变换
        Rotation subRot = Rotation::NONE;
        Mirror   subMir = Mirror::NONE;
        BlockPos subOff = region.position;
        bool     enabled = true;

        for (const auto& sr : transform.subRegions) {
            if (sr.name == region.name) {
                subRot  = sr.rotation;
                subMir  = sr.mirror;
                subOff  = sr.offset;
                enabled = sr.enabled;
                break;
            }
        }
        if (!enabled) continue;

        // 合并变换
        Rotation finalRot;
        Mirror   finalMir;
        combinedTransform(transform.rotation, transform.mirror,
                          subRot, subMir, finalRot, finalMir);

        int sx = region.sizeX(), sy = region.sizeY(), sz = region.sizeZ();

        // 调整负 Size 偏移
        BlockPos adjustedOff = region.getAdjustedOffset();

        // 变换后 region 原点
        BlockPos transformedRegionPos =
            transformLocalPos(subOff, sx, sz, transform.mirror, transform.rotation);

        for (int y = 0; y < sy; ++y)
        for (int z = 0; z < sz; ++z)
        for (int x = 0; x < sx; ++x) {
            if (skipAir && region.isAir(x, y, z)) continue;

            const Block* block = region.getBlock(x, y, z);
            if (!block && skipAir) continue;

            // 局部坐标 → 变换后局部坐标
            BlockPos local = transformLocalPos({x, y, z}, sx, sz, finalMir, finalRot);

            // 世界坐标
            BlockPos worldPos{
                transform.origin.x + transformedRegionPos.x + adjustedOff.x + local.x,
                transform.origin.y + transformedRegionPos.y + adjustedOff.y + local.y,
                transform.origin.z + transformedRegionPos.z + adjustedOff.z + local.z
            };

            if (block) {
                result.push_back(BedrockBlockEntry{worldPos, block});
            }
        }
    }
    return result;
}

// ============================================================
// SchematicProjection — 投影状态管理器
//
// 封装「当前加载的 Schematic + 放置变换」
// ============================================================
class SchematicProjection {
public:
    bool load(const std::filesystem::path& path) {
        bool ok = mSchematic.loadFromFile(path);
        mLoaded = ok;
        mDirty  = true;
        return ok;
    }

    bool isLoaded() const { return mLoaded; }

    // ---- 变换设置 ----
    void setOrigin(BlockPos origin) { mTransform.origin = origin; mDirty = true; }
    void move(int dx, int dy, int dz) {
        mTransform.origin.x += dx;
        mTransform.origin.y += dy;
        mTransform.origin.z += dz;
        mDirty = true;
    }
    void setRotation(Rotation rot)  { mTransform.rotation = rot; mDirty = true; }
    void rotateCW()  { mTransform.rotation = rotateBy(mTransform.rotation, Rotation::CW_90);  mDirty = true; }
    void rotateCCW() { mTransform.rotation = rotateBy(mTransform.rotation, Rotation::CCW_90); mDirty = true; }
    void toggleMirrorX() {
        mTransform.mirror = (mTransform.mirror == Mirror::MIRROR_X) ? Mirror::NONE : Mirror::MIRROR_X;
        mDirty = true;
    }
    void toggleMirrorZ() {
        mTransform.mirror = (mTransform.mirror == Mirror::MIRROR_Z) ? Mirror::NONE : Mirror::MIRROR_Z;
        mDirty = true;
    }
    void setMirror(Mirror m) { mTransform.mirror = m; mDirty = true; }
    void resetTransform() {
        BlockPos saved = mTransform.origin;
        mTransform = PlacementTransform::simple(saved);
        mDirty = true;
    }

    // ---- 查询 ----
    BlockPos getOrigin()   const { return mTransform.origin; }
    Rotation getRotation() const { return mTransform.rotation; }
    Mirror   getMirror()   const { return mTransform.mirror; }
    bool     isDirty()     const { return mDirty; }
    void     clearDirty()        { mDirty = false; }

    const schematic::LitematicSchematic& getSchematic() const { return mSchematic; }
    const PlacementTransform& getTransform() const { return mTransform; }

    // ---- 生成方块列表 ----
    const std::vector<BedrockBlockEntry>& getBlocks(bool skipAir = true) {
        if (mDirty || mCache.empty()) {
            mCache = toBedrockBlocksTransformed(mSchematic, mTransform, skipAir);
            mDirty = false;
        }
        return mCache;
    }

    // ---- 变换描述 ----
    std::string describeTransform() const {
        static const char* rotNames[] = {"0°", "CW90°", "180°", "CCW90°"};
        static const char* mirNames[] = {"None", "X", "Z"};
        return std::string("Rot:") + rotNames[static_cast<int>(mTransform.rotation)]
             + " Mir:" + mirNames[static_cast<int>(mTransform.mirror)];
    }

private:
    schematic::LitematicSchematic  mSchematic;
    PlacementTransform             mTransform;
    std::vector<BedrockBlockEntry> mCache;
    bool mLoaded = false;
    bool mDirty  = false;
};

} // namespace levishematic::transform
