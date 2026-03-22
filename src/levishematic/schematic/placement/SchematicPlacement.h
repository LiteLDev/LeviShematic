#pragma once
// ============================================================
// SchematicPlacement.h
// 投影放置管理 — 单个 Schematic 的放置实例
//
// 封装：
//   - 展开后的本地方块列表（来自 .mcstructure）
//   - 放置原点（世界坐标）
//   - 主旋转 / 主镜像
//   - enabled / renderEnabled 开关
//   - 包围盒计算
//   - 生成 ProjEntry 列表（供 ProjectionState 使用）
//
// Phase 3 核心组件
// ============================================================

#include "levishematic/render/ProjectionRenderer.h"

#include "mc/world/level/BlockPos.h"

#include <string>
#include <vector>

namespace levishematic::placement {

// ================================================================
// SchematicPlacement — 单个投影放置实例
// ================================================================
class SchematicPlacement {
public:
    struct LocalBlockEntry {
        BlockPos     localPos;
        const Block* block;
    };

    enum class Rotation : int {
        NONE   = 0,
        CW_90  = 1,
        CW_180 = 2,
        CCW_90 = 3,
    };

    enum class Mirror : int {
        NONE     = 0,
        MIRROR_X = 1,
        MIRROR_Z = 2,
    };

    // 唯一 ID（自增）
    using Id = uint32_t;

    SchematicPlacement() = default;

    SchematicPlacement(
        std::vector<LocalBlockEntry> localBlocks,
        BlockPos                    size,
        BlockPos                    origin,
        const std::string&          name = ""
    );

    // ---- 基本信息 ----
    Id                  getId()       const { return mId; }
    const std::string&  getName()     const { return mName; }
    const std::string&  getFilePath() const { return mFilePath; }
    void                setName(const std::string& name) { mName = name; }
    void                setFilePath(const std::string& path) { mFilePath = path; }

    // ---- 开关 ----
    bool isEnabled()       const { return mEnabled; }
    bool isRenderEnabled() const { return mRenderEnabled; }
    void setEnabled(bool v)       { mEnabled = v; markDirty(); }
    void setRenderEnabled(bool v) { mRenderEnabled = v; markDirty(); }
    void toggleEnabled()          { mEnabled = !mEnabled; markDirty(); }
    void toggleRender()           { mRenderEnabled = !mRenderEnabled; markDirty(); }

    // ---- 变换操作 ----
    BlockPos  getOrigin()   const { return mOrigin; }
    Rotation  getRotation() const { return mRotation; }
    Mirror    getMirror()   const { return mMirror; }

    void setOrigin(BlockPos origin);
    void move(int dx, int dy, int dz);
    void setRotation(Rotation rot);
    void rotateCW90();
    void rotateCCW90();
    void rotate180();
    void setMirror(Mirror mir);
    void toggleMirrorX();
    void toggleMirrorZ();
    void resetTransform();

    // ---- 投影块生成 ----
    // 生成 ProjEntry 列表（世界绝对坐标），skipAir = true 跳过空气块
    // 使用默认投影颜色
    std::vector<render::ProjEntry> generateProjEntries(bool skipAir = true) const;

    // 带自定义颜色
    std::vector<render::ProjEntry> generateProjEntries(mce::Color color, bool skipAir = true) const;

    // ---- 包围盒 ----
    // 计算放置后的世界坐标包围盒（minPos, maxPos）
    std::pair<BlockPos, BlockPos> getEnclosingBox() const;

    // ---- 脏标志（用于缓存失效）----
    bool isDirty() const { return mDirty; }
    void clearDirty()    { mDirty = false; }

    // ---- 变换描述 ----
    std::string describeTransform() const;

    // ---- 统计信息 ----
    int getTotalNonAirBlocks() const;

private:
    void markDirty() { mDirty = true; }
    static Rotation rotateBy(Rotation base, Rotation delta);
    static BlockPos transformLocalPos(BlockPos pos, int sizeX, int sizeZ, Mirror mirror, Rotation rot);

    // ---- 数据成员 ----
    static Id sNextId;

    Id          mId            = 0;
    std::string mName;
    std::string mFilePath;

    std::vector<LocalBlockEntry>                mLocalBlocks;
    BlockPos                                    mOrigin   = {0, 0, 0};
    BlockPos                                    mSize     = {0, 0, 0};
    Rotation                                    mRotation = Rotation::NONE;
    Mirror                                      mMirror   = Mirror::NONE;

    bool mEnabled       = true;
    bool mRenderEnabled = true;
    bool mDirty         = true;
};

} // namespace levishematic::placement
