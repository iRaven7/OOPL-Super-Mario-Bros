#ifndef MARIO_HPP
#define MARIO_HPP

#include "Character.hpp"
#include "MarioState.hpp"
#include "Util/Logger.hpp"
#include "Fireball.hpp"
#include <vector>
#include <memory>
#include <cmath>

class Mario : public Character {
public:
    enum class AnimState { IDLE, RUN, JUMP, SKID, CROUCH };

    Mario() : Character(RESOURCE_DIR"/Entities/LittleMairo/mario.png") {
        SetZIndex(50);
        ChangeState(std::make_unique<SmallMarioState>());
    }

    void ChangeState(std::unique_ptr<MarioState> newState, bool triggerPause = true) {
        m_State = std::move(newState);
        if (triggerPause) {
            m_TransformTimer = 1.0f;
        }
        else {
            m_TransformTimer = 0.0f;
        }
    }

    bool IsInvincible() const { return m_InvincibleTimer > 0.0f; }
    bool CanBreakBlocks() const override { return m_State ? m_State->CanBreakBlocks() : false; }
    void SetCrouching(bool crouching) {
        if (m_IsCrouching == crouching) return; // 狀態未改變則忽略
        m_IsCrouching = crouching;

        // 進行物理位移補償，確保底部貼齊地面
        if (m_State && m_State->GetHitboxSize().y > 16.0f) {
            if (m_IsCrouching) {
                m_WorldPosition.y -= 8.0f; // 蹲下時中心下移
            }
            else {
                m_WorldPosition.y += 8.0f; // 站起時中心上移
            }
        }
    }
    bool IsCrouching() const { return m_IsCrouching; }
    glm::vec2 GetSize() const override {
        glm::vec2 baseSize = m_State ? m_State->GetHitboxSize() : Character::GetSize();
        if (m_IsCrouching && baseSize.y > 16.0f) {
            return { baseSize.x, baseSize.y * 0.5f }; // 高度減半
        }
        return baseSize;
    }

    // 更新變身暫停邏輯
    void UpdateTransformation(float deltaTime) {
        if (m_TransformTimer > 0.0f) {
            m_TransformTimer -= deltaTime;
            // 變身期間快速閃爍
            m_Visible = (static_cast<int>(m_TransformTimer * 20) % 2 == 0);
        }
        else {
            m_TransformTimer = 0.0f;
            m_Visible = true;
        }
    }
    bool IsTransforming() const { return m_TransformTimer > 0.0f; }

    // 更新動畫邏輯 (傳入 inputDirection 判斷煞車)
    void UpdateAnimation(float deltaTime, float inputDirection) {
        if (!m_State) return;

        if (inputDirection < 0.0f) m_Transform.scale.x = -1.0f;
        else if (inputDirection > 0.0f) m_Transform.scale.x = 1.0f;

        AnimState newState = AnimState::IDLE;

        // 優先級 1：蹲下 (無視是否在空中，只要按著下就是蹲下)
        if (m_IsCrouching) {
            newState = AnimState::CROUCH;
        }
        // 優先級 2：跳躍
        else if (!m_IsGrounded) {
            newState = AnimState::JUMP;
        }
        // 優先級 3：煞車
        else if (inputDirection != 0.0f && std::signbit(m_Velocity.x) != std::signbit(inputDirection) && std::abs(m_Velocity.x) > 50.0f) {
            newState = AnimState::SKID;
        }
        // 優先級 4：跑動
        else if (std::abs(m_Velocity.x) > 10.0f) {
            newState = AnimState::RUN;
        }

        // 2. 更新圖片

        if (newState == AnimState::CROUCH) {
            SetImage(m_State->GetCrouchImage());
        }
        else if (newState == AnimState::JUMP) {
            SetImage(m_State->GetJumpImage());
        }
        else if (newState == AnimState::SKID) {
            SetImage(m_State->GetSkidImage());
            // 如果框架支援圖片翻轉，可以在這裡依據 m_Velocity.x 決定水平翻轉 (Flip)
        }
        else if (newState == AnimState::RUN) {
            // 動畫播放速度與絕對物理速度成正比 (越快切換越快)
            float animSpeed = std::abs(m_Velocity.x) / 150.0f;
            m_AnimTimer += deltaTime * animSpeed;

            auto frames = m_State->GetRunImages();
            if (!frames.empty()) {
                int frameIndex = static_cast<int>(m_AnimTimer * 5.0f) % frames.size();
                SetImage(frames[frameIndex]);
            }
        }
        else {
            SetImage(m_State->GetIdleImage());
            m_AnimTimer = 0.0f;
        }
    }

    void UpdateRenderPosition(float cameraX, float cameraZoom) {
        float yOffset = 0.0f;

        if (m_IsCrouching && m_State && m_State->GetHitboxSize().y > 16.0f) {
            yOffset = 8.0f;
        }

        m_Transform.translation.x = (m_WorldPosition.x - cameraX) * cameraZoom;
        m_Transform.translation.y = (m_WorldPosition.y + yOffset) * cameraZoom;

        // 修正：保留 UpdateAnimation 設定的左右翻轉狀態 (擷取 X 軸的正負號)
        float direction = (m_Transform.scale.x < 0.0f) ? -1.0f : 1.0f;
        m_Transform.scale.x = m_BaseScale.x * cameraZoom * direction;
        m_Transform.scale.y = m_BaseScale.y * cameraZoom;
    }

    // ... 原有的 TakeDamage, Die, Update (無敵計時器) 等邏輯保持不變 ...
    void Update(float deltaTime) {
        if (m_ShootCooldown > 0.0f) m_ShootCooldown -= deltaTime;
        if (m_InvincibleTimer > 0.0f) {
            m_InvincibleTimer -= deltaTime;
            m_Visible = (static_cast<int>(m_InvincibleTimer * 10) % 2 == 0);
        }
        else {
            m_InvincibleTimer = 0.0f;
            m_Visible = true;
        }
    }
    

    void TakeDamage() {
        if (IsInvincible()) return;

        if (dynamic_cast<SmallMarioState*>(m_State.get()) != nullptr) {
            Die();
        }
        else {
            // 受傷時 triggerPause 設為 false，避免遊戲凍結
            ChangeState(std::make_unique<SmallMarioState>(), false);
            m_InvincibleTimer = 2.0f;
        }
    }

    std::vector<std::shared_ptr<Fireball>> PopSpawnedFireballs() {
        auto res = m_SpawnedFireballs;
        m_SpawnedFireballs.clear();
        return res;
    }

    void Shoot() {
        if (m_State && m_State->CanShoot() && !m_IsCrouching && m_ShootCooldown <= 0.0f) {
            float facing = (m_Transform.scale.x > 0.0f) ? 1.0f : -1.0f;

            // 讓火球在瑪利歐稍微靠前與靠上的位置生成
            glm::vec2 spawnPos = { m_WorldPosition.x + facing * 16.0f, m_WorldPosition.y + 8.0f };
            m_SpawnedFireballs.push_back(std::make_shared<Fireball>(spawnPos, facing));

            m_ShootCooldown = 0.3f; // 0.3 秒發射冷卻
            LOG_INFO("發射火球！");
        }
    }

    // 死亡邏輯處理
    void Die() {
        m_IsDead = true;
    }

    // 取得死亡狀態
    bool IsDead() const {
        return m_IsDead;
    }

    
private:
    std::unique_ptr<MarioState> m_State;
    bool m_IsCrouching = false;
    float m_InvincibleTimer = 0.0f;
    float m_TransformTimer = 0.0f; // 變身暫停計時器
    float m_AnimTimer = 0.0f;      // 動畫幀播放計時器
    bool m_IsDead = false;
    float m_ShootCooldown = 0.0f;
    std::vector<std::shared_ptr<Fireball>> m_SpawnedFireballs;
};

#endif //MARIO_HPP