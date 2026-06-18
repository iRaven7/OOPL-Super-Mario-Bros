// include/Constants.hpp
#ifndef CONSTANTS_HPP
#define CONSTANTS_HPP

namespace PhysicsConstants {
    constexpr float GRAVITY = -780.0f;
    constexpr float MAX_FALL_SPEED = -430.0f;
    constexpr float JUMP_FORCE = 363.0f; // do not change this
    constexpr float JUMP_CUT_SPEED = 170.0f;   // vy cap when jump key released early
    constexpr float WALK_ACCEL = 250.0f;// do not change this
    constexpr float SPRINT_ACCEL = 350.0f;// do not change this
    constexpr float MAX_WALK_SPEED = 100.0f;// do not change this
    constexpr float MAX_SPRINT_SPEED = 180.0f;// do not change this
    constexpr float FRICTION = 500.0f;// do not change this
    constexpr float SKID_DECEL = 600.0f;// do not change this
}

#endif // CONSTANTS_HPP