#ifndef MARIO_HPP
#define MARIO_HPP

#include "Character.hpp"
#include "MarioState.hpp"
#include "Util/Logger.hpp"
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
    void SetCrouching(bool crouching) { m_IsCrouching = crouching; }
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

        // 1. 處理水平渲染翻轉 (X 軸鏡像)
        if (inputDirection < 0.0f) {
            m_Transform.scale.x = -1.0f; // 面向左側
        }
        else if (inputDirection > 0.0f) {
            m_Transform.scale.x = 1.0f;  // 面向右側 (預設)
        }

        AnimState newState = AnimState::IDLE;
        if (m_IsCrouching && m_IsGrounded) {
            newState = AnimState::CROUCH;
        }

        // 1. 狀態判定優先級：跳躍 > 煞車 > 跑動 > 靜止
        if (!m_IsGrounded) {
            newState = AnimState::JUMP;
        }
        // 煞車判定：有輸入方向，且輸入方向與目前物理速度方向相反，且速度大於一定閾值
        else if (inputDirection != 0.0f && std::signbit(m_Velocity.x) != std::signbit(inputDirection) && std::abs(m_Velocity.x) > 50.0f) {
            newState = AnimState::SKID;
        }
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

    void UpdateRenderPosition(float cameraX) {
        float standardHeight = 32.0f;
        float currentHeight = GetSize().y;

        // 計算位移補償：將算繪中心固定在「腳底往上 16 單位」的位置
        // 這樣不論 hitbox 多高，圖片底部都會對齊地板
        float yOffset = (standardHeight - currentHeight) / 2.0f;

        m_Transform.translation = { m_WorldPosition.x - cameraX, m_WorldPosition.y + yOffset };
    }

    // ... 原有的 TakeDamage, Die, Update (無敵計時器) 等邏輯保持不變 ...
    void Update(float deltaTime) {
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

    // 死亡邏輯處理
    void Die() {
        m_IsDead = true;
    }

    // 取得死亡狀態
    bool IsDead() const {
        return m_IsDead;
    }

    //
    //void Shoot() {
        // 檢查狀態機是否為火力型態 (FireMarioState)
     //   if (m_State && m_State->CanShoot()) {
     //       LOG_INFO("發射火球！");
           // 此處未來將實例化 Fireball 並加入 m_Items 陣列
    //    }
    //}
private:
    std::unique_ptr<MarioState> m_State;
    bool m_IsCrouching = false;
    float m_InvincibleTimer = 0.0f;
    float m_TransformTimer = 0.0f; // 變身暫停計時器
    float m_AnimTimer = 0.0f;      // 動畫幀播放計時器
    bool m_IsDead = false;
};

#endif //MARIO_HPP