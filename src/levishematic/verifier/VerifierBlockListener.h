#pragma once

#include "mc/world/level/LevelListener.h"

namespace levishematic::verifier_block_listener {

class VerifierBlockListener : public LevelListener {
public:
    virtual void onBlockChanged(
        ::BlockSource&                 source,
        ::BlockPos const&              pos,
        uint                           layer,
        ::Block const&                 block,
        ::Block const&                 oldBlock,
        int                            updateFlags,
        ::ActorBlockSyncMessage const* syncMsg,
        ::BlockChangedEventTarget      eventTarget,
        ::Actor*                       blockChangeSource
    );
};


} // namespace levishematic::verifier_block_listener