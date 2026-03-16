#include "levishematic/LeviShematic.h"

#include "ll/api/mod/RegisterHelper.h"

namespace levishematic {

LeviShematic& LeviShematic::getInstance() {
    static LeviShematic instance;
    return instance;
}

bool LeviShematic::load() {
    getSelf().getLogger().debug("Loading...");
    // Code for loading the mod goes here.
    return true;
}

bool LeviShematic::enable() {
    getSelf().getLogger().debug("Enabling...");
    // Code for enabling the mod goes here.
    return true;
}

bool LeviShematic::disable() {
    getSelf().getLogger().debug("Disabling...");
    // Code for disabling the mod goes here.
    return true;
}

} // namespace levishematic

LL_REGISTER_MOD(levishematic::LeviShematic, levishematic::LeviShematic::getInstance());
