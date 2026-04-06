#include "TickHook.h"

#include "levishematic/app/AppKernel.h"

#include "ll/api/memory/Hook.h"

#include "mc/client/player/LocalPlayer.h"
#include "mc/scripting/ServerScriptManager.h"
#include "mc/server/ServerInstance.h"

namespace levishematic::hook {

namespace {

bool gTickHooksRegistered = false;

}

LL_AUTO_TYPE_INSTANCE_HOOK(
    ServerStartCommandRegistration,
    HookPriority::Highest,
    ServerScriptManager,
    &ServerScriptManager::$onServerThreadStarted,
    EventResult,
    ::ServerInstance& ins
) {
    auto res = origin(ins);

    if (app::hasAppKernel()) {
        app::getAppKernel().onServerThreadStarted();
    }

    return res;
}

LL_TYPE_INSTANCE_HOOK(
    LocalPlayerFireDimensionChangedHook,
    ll::memory::HookPriority::Normal,
    LocalPlayer,
    &LocalPlayer::$_fireDimensionChanged,
    void
) {
    origin();

    if (app::hasAppKernel()) {
        app::getAppKernel().verifier().handleDimensionChanged();
    }
}

using TickHookRegistrar = ll::memory::HookRegistrar<
    ServerStartCommandRegistration,
    LocalPlayerFireDimensionChangedHook>;

void registerTickHooks() {
    if (gTickHooksRegistered) {
        return;
    }

    TickHookRegistrar::hook();
    gTickHooksRegistered = true;
}

void unregisterTickHooks() {
    if (!gTickHooksRegistered) {
        return;
    }

    TickHookRegistrar::unhook();
    gTickHooksRegistered = false;
}

} // namespace levishematic::hook
