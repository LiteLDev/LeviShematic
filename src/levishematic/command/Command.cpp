#include "Command.h"

#include "levishematic/core/DataManager.h"
#include "levishematic/render/ProjectionRenderer.h"
#include "levishematic/LeviShematic.h"

#include "ll/api/command/CommandHandle.h"
#include "ll/api/command/CommandRegistrar.h"
#include "ll/api/command/OverloadData.h"
#include "ll/api/service/TargetedBedrock.h"

#include "mc/client/game/ClientInstance.h"
#include "mc/client/renderer/game/LevelRenderer.h"
#include "mc/server/commands/CommandBlockName.h"
#include "mc/server/commands/CommandBlockNameResult.h"
#include "mc/server/commands/CommandOutput.h"
#include "mc/server/commands/CommandPositionFloat.h"
#include "mc/world/level/dimension/Dimension.h"

namespace levishematic::command {

// ================================================================
// 获取渲染层Logger
// ================================================================
static auto& getLogger() {
    return levishematic::LeviShematic::getInstance().getSelf().getLogger();
}

// ================================================================
// 获取当前维度的 RenderChunkCoordinator
// ================================================================
static std::shared_ptr<RenderChunkCoordinator>
getCoordinator(const CommandOrigin& origin) {
    auto client = ll::service::getClientInstance();
    if (!client || !client->getLevelRenderer()) return nullptr;
    auto dimId = origin.getDimension()->getDimensionId();
    return client->getLevelRenderer()->mRenderChunkCoordinators->at(dimId);
}

// ================================================================
// 测试命令参数（/rendert）— 沿用 test.cpp 功能
// ================================================================
struct RenderTestParam {
    CommandBlockName     blockName;
    CommandPositionFloat pos;
    int                  title_date;
    enum class mode { add, single } mode_;
};

// 临时存储（测试用）
static std::vector<render::ProjEntry> sTestEntries;

// ================================================================
// 注册命令
// ================================================================
void registerCommands(bool isClient) {
    auto& dm = core::DataManager::getInstance();

    // ---- /rendert（测试渲染命令，保持与 test.cpp 兼容）----
    auto& renderCmd =
        ll::command::CommandRegistrar::getInstance(isClient).getOrCreateCommand("rendert", "Projection render test");

    // /rendert <pos> <block> <data> <add|single>
    renderCmd.overload<RenderTestParam>()
        .required("pos")
        .required("blockName")
        .required("title_date")
        .required("mode_")
        .execute([&](CommandOrigin const& origin,
                     CommandOutput&       output,
                     RenderTestParam const& param,
                     Command const&       cmd) {
            auto& proj = dm.getProjectionState();
            auto  coord = getCoordinator(origin);

            if (param.mode_ == RenderTestParam::mode::single) {
                proj.setSingle(
                    origin.getExecutePosition(cmd.mVersion, param.pos),
                    param.blockName.resolveBlock(param.title_date).mBlock,
                    mce::Color(0.75f, 0.85f, 1.0f, 0.85f)
                );
                sTestEntries.clear();
            } else {
                sTestEntries.push_back(
                    {BlockPos(origin.getExecutePosition(cmd.mVersion, param.pos)),
                     param.blockName.resolveBlock(param.title_date).mBlock,
                     mce::Color(0.75f, 0.85f, 1.0f, 0.85f)}
                );
                proj.setEntries(sTestEntries);
            }

            dm.triggerRebuild(coord);
            output.success("Projection updated");
        });

    // /rendert（无参数 = 清空）
    renderCmd.overload<RenderTestParam>()
        .execute([&](CommandOrigin const& origin, CommandOutput& output) {
            dm.getProjectionState().clear();
            sTestEntries.clear();
            dm.triggerRebuild(getCoordinator(origin));
            output.success("Projection cleared");
        });

    // ---- /schem（正式命令框架，Phase 2+）----
    auto& schemCmd =
        ll::command::CommandRegistrar::getInstance(isClient).getOrCreateCommand("schem", "LeviSchematic commands");

    // /schem clear
    schemCmd.overload()
        .execute([&](CommandOrigin const& origin, CommandOutput& output) {
            dm.getProjectionState().clear();
            sTestEntries.clear();
            dm.triggerRebuild(getCoordinator(origin));
            output.success("All projections cleared");
        });

    getLogger().info("LeviSchematic commands registered");
}

} // namespace levishematic::command
