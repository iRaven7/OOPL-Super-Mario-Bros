#include "Character.hpp"
#include "Util/Image.hpp"
#include <algorithm>
#include <cmath>
//aa
Character::Character(const std::string& ImagePath) {
    SetImage(ImagePath);
}

void Character::SetImage(const std::string& ImagePath) {
    m_ImagePath = ImagePath;

    // 將圖片資源載入，並賦值給繼承自 GameObject 的 m_Drawable
    m_Drawable = std::make_shared<Util::Image>(m_ImagePath);
}

glm::vec2 Character::UpdatePhysics(float deltaTime, float inputDirection, bool isSprinting, bool wantsJump) {
    // 1. 水平物理運算 (X軸)
    float currentAccel = isSprinting ? SPRINT_ACCEL : WALK_ACCEL;
    float maxSpeed = isSprinting ? MAX_SPRINT_SPEED : MAX_WALK_SPEED;

    if (inputDirection != 0.0f) {
        // 轉向滑行判定
        if (m_Velocity.x != 0.0f && std::signbit(m_Velocity.x) != std::signbit(inputDirection)) {
            m_Velocity.x += inputDirection * SKID_DECEL * deltaTime;
        }
        else {
            m_Velocity.x += inputDirection * currentAccel * deltaTime;
        }
        // 限制最高速度
        m_Velocity.x = std::clamp(m_Velocity.x, -maxSpeed, maxSpeed);
    }
    else {
        // 摩擦力減速
        if (m_Velocity.x > 0.0f) {
            m_Velocity.x = std::max(0.0f, m_Velocity.x - FRICTION * deltaTime);
        }
        else if (m_Velocity.x < 0.0f) {
            m_Velocity.x = std::min(0.0f, m_Velocity.x + FRICTION * deltaTime);
        }
    }

    // 2. 垂直物理運算 (Y軸)
    if (m_IsGrounded && wantsJump) {
        m_Velocity.y = JUMP_FORCE; // 賦予跳躍初始速度
        m_IsGrounded = false;      // 脫離地面
    }

    if (!m_IsGrounded) {
        m_Velocity.y += GRAVITY * deltaTime; // 施加重力
        m_Velocity.y = std::max(m_Velocity.y, MAX_FALL_SPEED); // 限制終端速度
    }

    // 3. 積分計算：更新座標
    glm::vec2 currentPos = GetPosition();
    currentPos += m_Velocity * deltaTime;

    // 4. 臨時地板限制 (Week 5 AABB 碰撞前使用)
    float tempFloorY = -150.0f;
    if (currentPos.y <= tempFloorY) {
        currentPos.y = tempFloorY;
        m_Velocity.y = 0.0f;
        m_IsGrounded = true;
    }
    else {
        // 確保如果走出平台邊緣，狀態會正確切換為空中
        m_IsGrounded = false;
    }

    // 5. 將結果寫回物件
    SetPosition(currentPos);

    return m_Velocity;
}