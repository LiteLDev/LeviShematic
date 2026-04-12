#include "levishematic/LeviShematic.h"

#include "levishematic/app/AppKernel.h"
#include "levishematic/hook/RenderHook.h"

#include "ll/api/mod/RegisterHelper.h"

namespace levishematic {

LeviShematic& LeviShematic::getInstance() {
    static LeviShematic instance;
    return instance;
}

bool LeviShematic::load() {
    getSelf().getLogger().debug("Loading...");
    app::load();
    return true;
}

bool LeviShematic::enable() {
    getSelf().getLogger().debug("Enabling...");
    return true;
}

bool LeviShematic::disable() {
    getSelf().getLogger().debug("Disabling...");
    app::stop();
    return true;
}

} // namespace levishematic

LL_REGISTER_MOD(levishematic::LeviShematic, levishematic::LeviShematic::getInstance());
