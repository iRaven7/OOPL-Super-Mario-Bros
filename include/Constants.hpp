// include/Constants.hpp
#ifndef CONSTANTS_HPP
#define CONSTANTS_HPP

namespace PhysicsConstants {
    constexpr float GRAVITY = -2400.0f;
    constexpr float MAX_FALL_SPEED = -900.0f;
    constexpr float JUMP_FORCE = 850.0f;
    constexpr float JUMP_CUT_SPEED = 200.0f;   // vy cap when jump key released early
    constexpr float WALK_ACCEL = 400.0f;
    constexpr float SPRINT_ACCEL = 700.0f;
    constexpr float MAX_WALK_SPEED = 300.0f;
    constexpr float MAX_SPRINT_SPEED = 550.0f;
    constexpr float FRICTION = 600.0f;
    constexpr float SKID_DECEL = 1800.0f;
}

#endif // CONSTANTS_HPP