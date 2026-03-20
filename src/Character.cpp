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

    // 2. 垂直物理 (加入跳躍與重力)

    // 1. 宣告並取得當前座標
    glm::vec2 currentPos = GetPosition();

    // 2. 積分計算：位移 = 速度 * 時間
    currentPos.x += m_Velocity.x * deltaTime;
    currentPos.y += m_Velocity.y * deltaTime;

    // 處理跳躍輸入：必須在地面上才能跳
    if (m_IsGrounded && wantsJump) {
        m_Velocity.y = JUMP_FORCE; // 給予瞬間向上的速度
        m_IsGrounded = false;      // 離開地面
    }

    // 應用重力 (非接地狀態)
    if (!m_IsGrounded) {
        m_Velocity.y += GRAVITY * deltaTime;
        if (m_Velocity.y < MAX_FALL_SPEED) {
            m_Velocity.y = MAX_FALL_SPEED;
        }


        // --- 臨時：Week 5 碰撞實作前的地板 HACK ---
        // 假設地圖管理器生成的 Block Y 座標在 200.0f (依據 MapManager.cpp)
        // 這裡我們硬編碼一個地板高度，例如 150.0f (考慮角色高度)
        float tempFloorY = -150.0f;
        if (currentPos.y <= tempFloorY) {
            currentPos.y = tempFloorY;
            m_Velocity.y = 0.0f;
            m_IsGrounded = true;
        }

        SetPosition(currentPos);

        return m_Velocity; // 回傳速度供外部參考


        // 限制最高速度
        if (m_Velocity.x > maxSpeed) m_Velocity.x = maxSpeed;
        if (m_Velocity.x < -maxSpeed) m_Velocity.x = -maxSpeed;
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
        currentPos = GetPosition();
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
}