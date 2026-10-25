#pragma once
#include "game/sdk.h"

namespace aimbot {
    void Tick(float dt);
    bool IsTargeting();
    int  CurrentTargetIndex();
}