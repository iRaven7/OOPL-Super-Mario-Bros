#include "Character.hpp"
#include "Constants.hpp"
#include "Util/Image.hpp"
#include "Block.hpp"
#include <algorithm>
#include <cmath>

Character::Character(const std::string& ImagePath) {
    SetImage(ImagePath);
}

void Character::SetImage(const std::string& ImagePath) {
    m_ImagePath = ImagePath;
    m_Drawable = std::make_shared<Util::Image>(m_ImagePath);
}

glm::vec2 Character::UpdatePhysics(float deltaTime, float inputDirection, bool isSprinting, bool wantsJump, const std::vector<std::shared_ptr<Block>>& blocks, bool isJumpHeld) {
    float currentAccel = isSprinting ? PhysicsConstants::SPRINT_ACCEL : PhysicsConstants::WALK_ACCEL;
    float maxSpeed = isSprinting ? PhysicsConstants::MAX_SPRINT_SPEED : PhysicsConstants::MAX_WALK_SPEED;

    if (inputDirection != 0.0f) {
        if (m_Velocity.x != 0.0f && std::signbit(m_Velocity.x) != std::signbit(inputDirection)) {
            m_Velocity.x += inputDirection * PhysicsConstants::SKID_DECEL * deltaTime;
        }
        else {
            m_Velocity.x += inputDirection * currentAccel * deltaTime;
        }
        m_Velocity.x = std::clamp(m_Velocity.x, -maxSpeed, maxSpeed);
    }
    else {
        if (m_Velocity.x > 0.0f) {
            m_Velocity.x = std::max(0.0f, m_Velocity.x - PhysicsConstants::FRICTION * deltaTime);
        }
        else if (m_Velocity.x < 0.0f) {
            m_Velocity.x = std::min(0.0f, m_Velocity.x + PhysicsConstants::FRICTION * deltaTime);
        }
    }

    if (m_IsGrounded && wantsJump) {
        m_Velocity.y = PhysicsConstants::JUMP_FORCE;
        m_IsGrounded = false;
    }

    m_Velocity.y += PhysicsConstants::GRAVITY * deltaTime;
    if (!isJumpHeld && m_Velocity.y > PhysicsConstants::JUMP_CUT_SPEED) {
        m_Velocity.y = PhysicsConstants::JUMP_CUT_SPEED;
    }
    m_Velocity.y = std::max(m_Velocity.y, PhysicsConstants::MAX_FALL_SPEED);

    glm::vec2 currentPos = GetPosition();
    glm::vec2 mySize = GetSize();

    // X axis: shrink hitbox height by 0.2 to prevent corner-sticking
    currentPos.x += m_Velocity.x * deltaTime;

    glm::vec2 xHitboxSize = { mySize.x, mySize.y - 0.2f };

    for (const auto& block : blocks) {
        if (!block->IsActive() || !block->IsCollidable() || !block->IsXCollidable()) continue;

        if (CheckAABB(currentPos, xHitboxSize, block->GetCollisionPosition(), block->GetSize())) {
            if (m_Velocity.x > 0.0f) {
                currentPos.x = block->GetCollisionPosition().x - (block->GetSize().x / 2.0f) - (mySize.x / 2.0f);
            }
            else if (m_Velocity.x < 0.0f) {
                currentPos.x = block->GetCollisionPosition().x + (block->GetSize().x / 2.0f) + (mySize.x / 2.0f);
            }
            m_Velocity.x = 0.0f;
        }
    }

    // Y axis: shrink hitbox width by 0.2 to prevent corner-sticking
    currentPos.y += m_Velocity.y * deltaTime;
    m_IsGrounded = false;

    glm::vec2 yHitboxSize = { mySize.x - 0.2f, mySize.y };

    for (const auto& block : blocks) {
        if (!block->IsActive() || !block->IsCollidable()) continue;

        if (CheckAABB(currentPos, yHitboxSize, block->GetCollisionPosition(), block->GetSize())) {
            if (m_Velocity.y < 0.0f) {
                currentPos.y = block->GetCollisionPosition().y + (block->GetSize().y / 2.0f) + (mySize.y / 2.0f);
                m_IsGrounded = true;
                m_Velocity.y = std::min(0.0f, block->GetVelocityY());
            }
            else if (m_Velocity.y > 0.0f) {
                currentPos.y = block->GetCollisionPosition().y - (block->GetSize().y / 2.0f) - (mySize.y / 2.0f);
                block->OnHit(this);
                m_Velocity.y = 0.0f;
            }
        }
    }

    SetPosition(currentPos);
    return m_Velocity;
}
