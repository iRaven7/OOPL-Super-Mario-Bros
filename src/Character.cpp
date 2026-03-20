#include "Character.hpp"
#include "Util/Image.hpp"
#include <algorithm>
#include <cmath>

Character::Character(const std::string& ImagePath) {
    SetImage(ImagePath);
}

void Character::SetImage(const std::string& ImagePath) {
    m_ImagePath = ImagePath;
    m_Drawable = std::make_shared<Util::Image>(m_ImagePath);
}

glm::vec2 Character::UpdatePhysics(float deltaTime, float inputDirection, bool isSprinting, bool wantsJump) {
    // 1. 水平物理運算
    float currentAccel = isSprinting ? SPRINT_ACCEL : WALK_ACCEL;
    float maxSpeed = isSprinting ? MAX_SPRINT_SPEED : MAX_WALK_SPEED;

    if (inputDirection != 0.0f) {
        if (m_Velocity.x != 0.0f && std::signbit(m_Velocity.x) != std::signbit(inputDirection)) {
            m_Velocity.x += inputDirection * SKID_DECEL * deltaTime;
        }
        else {
            m_Velocity.x += inputDirection * currentAccel * deltaTime;
        }
        m_Velocity.x = std::clamp(m_Velocity.x, -maxSpeed, maxSpeed);
    }
    else {
        if (m_Velocity.x > 0.0f) {
            m_Velocity.x = std::max(0.0f, m_Velocity.x - FRICTION * deltaTime);
        }
        else if (m_Velocity.x < 0.0f) {
            m_Velocity.x = std::min(0.0f, m_Velocity.x + FRICTION * deltaTime);
        }
    }

    // 2. 垂直物理運算
    if (m_IsGrounded && wantsJump) {
        m_Velocity.y = JUMP_FORCE;
        m_IsGrounded = false;
    }

    if (!m_IsGrounded) {
        m_Velocity.y += GRAVITY * deltaTime;
        m_Velocity.y = std::max(m_Velocity.y, MAX_FALL_SPEED);
    }

    // 3. 座標積分與更新
    glm::vec2 currentPos = GetPosition();
    currentPos += m_Velocity * deltaTime;

    // 臨時地板限制 (供 Week 5 碰撞實作前測試使用)
    float tempFloorY = -150.0f;
    if (currentPos.y <= tempFloorY) {
        currentPos.y = tempFloorY;
        m_Velocity.y = 0.0f;
        m_IsGrounded = true;
    }
    else {
        m_IsGrounded = false;
    }

    SetPosition(currentPos);
    return m_Velocity;
}