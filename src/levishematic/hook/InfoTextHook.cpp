#include "InfoTextHook.h"

#include "ll/api/memory/Hook.h"
#include "ll/api/service/TargetedBedrock.h"

#include "mc/client/gui/Font.h"
#include "mc/client/gui/screens/ScreenContext.h"
#include "mc/client/renderer/screen/MinecraftUIRenderContext.h"
#include "mc/client/gui/screens/ScreenView.h"
#include "mc/client/game/ClientInstance.h"
#include "mc/client/gui/FontHandle.h"


namespace levishematic::hook {

// LL_TYPE_INSTANCE_HOOK(
//     InfoTextHook,
//     HookPriority::Normal,
//     ScreenView,
//     &ScreenView::render,
//     void,
//     ::UIRenderContext& uiRenderContext
// ) {
//     origin(uiRenderContext);

//     MinecraftUIRenderContext& screenContext = reinterpret_cast<MinecraftUIRenderContext&>(uiRenderContext);


//     ll::service::getClientInstance()->getFontHandle().mDefaultFont->drawShadow(
//         screenContext.mScreenContext,
//         g_overlayText,
//         2,
//         screenContext.mScreenContext.ym - 12.0f,
//         mce::Color::WHITE(),
//         false,
//         nullptr,
//         1.0f
//     );
// }

// using InfoTextHooks = ll::memory::HookRegistrar<
//     InfoTextHook>;

// void registerInfoTextHooks() {
//     InfoTextHooks::hook();
// }

// void unregisterInfoTextHooks() {
//     InfoTextHooks::unhook();
// }
} // namespace levishematic::hook