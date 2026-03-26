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

glm::vec2 Character::UpdatePhysics(float deltaTime, float inputDirection, bool isSprinting, bool wantsJump, const std::vector<std::shared_ptr<Block>>& blocks) {
    // 1. 計算水平與垂直加速度
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

    if (m_IsGrounded && wantsJump) {
        m_Velocity.y = JUMP_FORCE;
        m_IsGrounded = false;
    }

    if (!m_IsGrounded) {
        m_Velocity.y += GRAVITY * deltaTime;
        m_Velocity.y = std::max(m_Velocity.y, MAX_FALL_SPEED);
    }

    glm::vec2 currentPos = GetPosition();
    glm::vec2 mySize = GetSize();

    // 2. X 軸獨立移動與碰撞解析 (這裡使用了 blocks 參數)
    currentPos.x += m_Velocity.x * deltaTime;
    for (const auto& block : blocks) {
        if (CheckAABB(currentPos, mySize, block->GetPosition(), block->GetSize())) {
            if (m_Velocity.x > 0.0f) { // 向右撞擊
                currentPos.x = block->GetPosition().x - (block->GetSize().x / 2.0f) - (mySize.x / 2.0f);
            }
            else if (m_Velocity.x < 0.0f) { // 向左撞擊
                currentPos.x = block->GetPosition().x + (block->GetSize().x / 2.0f) + (mySize.x / 2.0f);
            }
            m_Velocity.x = 0.0f;
        }
    }

    // 3. Y 軸獨立移動與碰撞解析
    currentPos.y += m_Velocity.y * deltaTime;
    m_IsGrounded = false; // 每幀重置接地狀態

    for (const auto& block : blocks) {
        if (CheckAABB(currentPos, mySize, block->GetPosition(), block->GetSize())) {
            if (m_Velocity.y < 0.0f) { // 向下掉落接觸方塊 (踩到地板)
                currentPos.y = block->GetPosition().y + (block->GetSize().y / 2.0f) + (mySize.y / 2.0f);
                m_IsGrounded = true;
            }
            else if (m_Velocity.y > 0.0f) { // 向上跳躍接觸方塊 (頂磚塊)
                currentPos.y = block->GetPosition().y - (block->GetSize().y / 2.0f) - (mySize.y / 2.0f);
            }
            m_Velocity.y = 0.0f;
        }
    }

    // 4. 更新最終座標
    SetPosition(currentPos);
    return m_Velocity;
}