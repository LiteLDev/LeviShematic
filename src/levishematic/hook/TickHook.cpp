#include "TickHook.h"

#include "levishematic/app/AppKernel.h"

#include "ll/api/memory/Hook.h"
#include "ll/api/service/TargetedBedrock.h"

#include "mc/client/player/LocalPlayer.h"
#include "mc/scripting/ServerScriptManager.h"
#include "mc/server/ServerInstance.h"
#include "mc/client/gui/screens/InGamePlayScreen.h"

#include "mc/client/gui/Font.h"
#include "mc/client/gui/screens/ScreenContext.h"
#include "mc/client/game/ClientInstance.h"
#include "mc/client/gui/FontHandle.h"
#include "mc/client/gui/GuiData.h"

namespace levishematic::hook {

namespace {

bool gTickHooksRegistered = false;

std::string g_overlayText = "test text";

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
    RenderLevelHook,
    HookPriority::Highest,
    InGamePlayScreen,
    &InGamePlayScreen::$_renderLevel,
    void,
    ScreenContext& screenContext,
    FrameRenderObject const& renderObj
) {
    //渲染level，可以把需要频繁渲染检测的放这
    origin(screenContext, renderObj);
    // 通过 GuiData 获取实际 UI 屏幕尺寸（会随窗口大小变化自动更新）
    auto& guiData = screenContext.guiData;

    // float screenWidth  = guiData.mScreenSizeData->clientScreenSize->x;   // UI 坐标系宽度
    float screenHeight = guiData->mScreenSizeData->clientScreenSize->y;   // UI 坐标系高度

    // 左下角：x=2（留一点边距），y = 屏幕高度 - 字体高度 - 边距
    constexpr float margin = 4.0f;
    constexpr float lineHeight = 9.0f; // MC 默认字体高度约为 9

    ll::service::getClientInstance()->getFontHandle().mDefaultFont->drawShadow(
        screenContext,
        g_overlayText,
        margin,
        screenHeight - lineHeight - margin,  // 左下角
        mce::Color::WHITE(),
        false,
        nullptr,
        1.0f
    );
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
