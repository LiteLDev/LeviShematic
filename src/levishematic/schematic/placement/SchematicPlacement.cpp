#include "SchematicPlacement.h"

namespace levishematic::placement {

// ================================================================
// 静态 ID 生成器
// ================================================================
SchematicPlacement::Id SchematicPlacement::sNextId = 1;

SchematicPlacement::SchematicPlacement(
    std::vector<LocalBlockEntry> localBlocks,
    BlockPos                    size,
    BlockPos                    origin,
    const std::string&          name
)
    : mId(sNextId++)
    , mName(name)
    , mLocalBlocks(std::move(localBlocks))
    , mSize(size)
    , mOrigin(origin)
{}

SchematicPlacement::Rotation SchematicPlacement::rotateBy(Rotation base, Rotation delta) {
    return static_cast<Rotation>((static_cast<int>(base) + static_cast<int>(delta)) % 4);
}

BlockPos SchematicPlacement::transformLocalPos(
    BlockPos pos,
    int      sizeX,
    int      sizeZ,
    Mirror   mirror,
    Rotation rot
) {
    int x = pos.x;
    int y = pos.y;
    int z = pos.z;

    switch (mirror) {
    case Mirror::MIRROR_X:
        x = sizeX - 1 - x;
        break;
    case Mirror::MIRROR_Z:
        z = sizeZ - 1 - z;
        break;
    case Mirror::NONE:
    default:
        break;
    }

    switch (rot) {
    case Rotation::CW_90:
        return {sizeZ - 1 - z, y, x};
    case Rotation::CW_180:
        return {sizeX - 1 - x, y, sizeZ - 1 - z};
    case Rotation::CCW_90:
        return {z, y, sizeX - 1 - x};
    case Rotation::NONE:
    default:
        return {x, y, z};
    }
}

// ================================================================
// 变换操作
// ================================================================
void SchematicPlacement::setOrigin(BlockPos origin) {
    mOrigin = origin;
    markDirty();
}

void SchematicPlacement::move(int dx, int dy, int dz) {
    mOrigin.x += dx;
    mOrigin.y += dy;
    mOrigin.z += dz;
    markDirty();
}

void SchematicPlacement::setRotation(Rotation rot) {
    mRotation = rot;
    markDirty();
}

void SchematicPlacement::rotateCW90() {
    mRotation = rotateBy(mRotation, Rotation::CW_90);
    markDirty();
}

void SchematicPlacement::rotateCCW90() {
    mRotation = rotateBy(mRotation, Rotation::CCW_90);
    markDirty();
}

void SchematicPlacement::rotate180() {
    mRotation = rotateBy(mRotation, Rotation::CW_180);
    markDirty();
}

void SchematicPlacement::setMirror(Mirror mir) {
    mMirror = mir;
    markDirty();
}

void SchematicPlacement::toggleMirrorX() {
    mMirror = (mMirror == Mirror::MIRROR_X) ? Mirror::NONE : Mirror::MIRROR_X;
    markDirty();
}

void SchematicPlacement::toggleMirrorZ() {
    mMirror = (mMirror == Mirror::MIRROR_Z) ? Mirror::NONE : Mirror::MIRROR_Z;
    markDirty();
}

void SchematicPlacement::resetTransform() {
    mRotation = Rotation::NONE;
    mMirror   = Mirror::NONE;
    markDirty();
}

// ================================================================
// 投影块生成 — 默认颜色
// ================================================================
static constexpr mce::Color kDefaultProjectionColor{0.75f, 0.85f, 1.0f, 0.85f};

std::vector<render::ProjEntry>
SchematicPlacement::generateProjEntries(bool skipAir) const {
    return generateProjEntries(kDefaultProjectionColor, skipAir);
}

// ================================================================
// 投影块生成 — 自定义颜色
//
// 直接对本地方块列表应用旋转/镜像，再转换为世界坐标
// ================================================================
std::vector<render::ProjEntry>
SchematicPlacement::generateProjEntries(mce::Color color, bool skipAir) const {
    std::vector<render::ProjEntry> result;

    if (mLocalBlocks.empty() || !mEnabled || !mRenderEnabled) {
        return result;
    }

    result.reserve(mLocalBlocks.size());
    for (const auto& entry : mLocalBlocks) {
        if (!entry.block) continue;
        if (skipAir && entry.block->isAir()) continue;

        auto local = transformLocalPos(entry.localPos, mSize.x, mSize.z, mMirror, mRotation);
        result.push_back(render::ProjEntry{
            BlockPos{
                mOrigin.x + local.x,
                mOrigin.y + local.y,
                mOrigin.z + local.z,
            },
            entry.block,
            color
        });
    }

    return result;
}

// ================================================================
// 包围盒计算
// ================================================================
std::pair<BlockPos, BlockPos> SchematicPlacement::getEnclosingBox() const {
    if (mLocalBlocks.empty()) {
        return {mOrigin, mOrigin};
    }

    auto entries = generateProjEntries(true);
    if (entries.empty()) {
        return {mOrigin, mOrigin};
    }

    BlockPos minPos = entries[0].pos;
    BlockPos maxPos = entries[0].pos;

    for (const auto& e : entries) {
        if (e.pos.x < minPos.x) minPos.x = e.pos.x;
        if (e.pos.y < minPos.y) minPos.y = e.pos.y;
        if (e.pos.z < minPos.z) minPos.z = e.pos.z;
        if (e.pos.x > maxPos.x) maxPos.x = e.pos.x;
        if (e.pos.y > maxPos.y) maxPos.y = e.pos.y;
        if (e.pos.z > maxPos.z) maxPos.z = e.pos.z;
    }

    return {minPos, maxPos};
}

// ================================================================
// 变换描述
// ================================================================
std::string SchematicPlacement::describeTransform() const {
    static const char* rotNames[] = {"0°", "CW90°", "180°", "CCW90°"};
    static const char* mirNames[] = {"None", "X", "Z"};

    std::string desc = "Origin:(" + std::to_string(mOrigin.x) + ","
                     + std::to_string(mOrigin.y) + "," + std::to_string(mOrigin.z) + ")"
                     + " Rot:" + rotNames[static_cast<int>(mRotation)]
                     + " Mir:" + mirNames[static_cast<int>(mMirror)];
    return desc;
}

// ================================================================
// 统计
// ================================================================
int SchematicPlacement::getTotalNonAirBlocks() const {
    return static_cast<int>(mLocalBlocks.size());
}

} // namespace levishematic::placement
