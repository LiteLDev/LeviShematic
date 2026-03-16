#include "TickHook.h"

#include "levishematic/command/Command.h"
#include "levishematic/core/DataManager.h"

#include "ll/api/memory/Hook.h"

#include "mc/scripting/ServerScriptManager.h"
#include "mc/server/ServerInstance.h"

namespace levishematic::hook {

// ================================================================
// Hook: ServerScriptManager::onServerThreadStarted
//
// 在服务端脚本管理器启动时注册命令。
// 这是命令注册的正确时机——CommandRegistrar 已初始化。
// ================================================================
LL_AUTO_TYPE_INSTANCE_HOOK(
    ServerStartCommandRegistration,
    HookPriority::Highest,
    ServerScriptManager,
    &ServerScriptManager::$onServerThreadStarted,
    EventResult,
    ::ServerInstance& ins
) {
    auto res = origin(ins);

    // 初始化 DataManager
    core::DataManager::getInstance().init();

    // 注册命令
    command::registerCommands(false);

    return res;
}

} // namespace levishematic::hook
