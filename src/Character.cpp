#include "Character.hpp"
#include <algorithm>
#include <cmath>

Character::Character(const std::string& ImagePath) {
    SetImage(ImagePath);
}

void Character::SetImage(const std::string& ImagePath) {
    m_ImagePath = ImagePath;
    // 假設利用 Util::Image 進行資源載入
    // m_Drawable = std::make_shared<Util::Image>(m_ImagePath);
}

void Character::UpdatePhysics(float deltaTime, float inputDirection, bool isSprinting) {
    // 1. 水平物理運算 (X軸)
    float currentAccel = isSprinting ? SPRINT_ACCEL : WALK_ACCEL;
    float maxSpeed = isSprinting ? MAX_SPRINT_SPEED : MAX_WALK_SPEED;

    if (inputDirection != 0.0f) { // 玩家有輸入方向鍵 (1.0 或 -1.0)
        // 判斷是否為「轉向滑行」：輸入方向與當前速度方向相反
        if (m_Velocity.x != 0.0f && std::signbit(m_Velocity.x) != std::signbit(inputDirection)) {
            // 施加強大的反向阻力
            m_Velocity.x += inputDirection * SKID_DECEL * deltaTime;
        }
        else {
            // 正常加速
            m_Velocity.x += inputDirection * currentAccel * deltaTime;
        }

        // 限制最高速度
        if (m_Velocity.x > maxSpeed) m_Velocity.x = maxSpeed;
        if (m_Velocity.x < -maxSpeed) m_Velocity.x = -maxSpeed;

    }
    else { // 玩家無輸入，施加自然摩擦力使其緩慢停止
        if (m_Velocity.x > 0.0f) {
            m_Velocity.x -= FRICTION * deltaTime;
            if (m_Velocity.x < 0.0f) m_Velocity.x = 0.0f;
        }
        else if (m_Velocity.x < 0.0f) {
            m_Velocity.x += FRICTION * deltaTime;
            if (m_Velocity.x > 0.0f) m_Velocity.x = 0.0f;
        }
    }

    // 2. 垂直物理運算 (Y軸) - 引入重力
    if (!m_IsGrounded) {
        m_Velocity.y += GRAVITY * deltaTime;
        // 限制最大下墜速度 (Terminal Velocity)
        if (m_Velocity.y < MAX_FALL_SPEED) {
            m_Velocity.y = MAX_FALL_SPEED;
        }
    }

    // 3. 積分計算：將速度轉換為位移並更新座標
    glm::vec2 currentPos = GetPosition();
    currentPos.x += m_Velocity.x * deltaTime;
    currentPos.y += m_Velocity.y * deltaTime;

    SetPosition(currentPos);

    // 註：在 Week 5 的碰撞偵測實作前，為防止角色無限掉落，可暫時加入地板高度硬限制
    if (currentPos.y <= -200.0f) { // 假設 -200.0f 為地板 Y 座標
        currentPos.y = -200.0f;
        m_Velocity.y = 0.0f;
        m_IsGrounded = true;
        SetPosition(currentPos);
    }
    else {
        m_IsGrounded = false;
    }
}