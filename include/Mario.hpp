#ifndef MARIO_HPP
#define MARIO_HPP

#include "Character.hpp"
#include "MarioState.hpp"
#include "Util/Logger.hpp"
#include <memory>
#include <cmath>

class Mario : public Character {
public:
    enum class AnimState { IDLE, RUN, JUMP, SKID };

    Mario() : Character(RESOURCE_DIR"/Entities/LittleMairo/mario.png") {
        SetZIndex(50);
        ChangeState(std::make_unique<SmallMarioState>());
    }

    void ChangeState(std::unique_ptr<MarioState> newState) {
        m_State = std::move(newState);
        m_TransformTimer = 1.0f; // 設定變身暫停時間為 1 秒
    }

    bool CanBreakBlocks() const override { return m_State ? m_State->CanBreakBlocks() : false; }
    glm::vec2 GetSize() const override { return m_State ? m_State->GetHitboxSize() : Character::GetSize(); }

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

        AnimState newState = AnimState::IDLE;

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
        if (newState == AnimState::JUMP) {
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

    // ... 省略其餘不變之實作
private:
    std::unique_ptr<MarioState> m_State;
    float m_InvincibleTimer = 0.0f;
    float m_TransformTimer = 0.0f; // 變身暫停計時器
    float m_AnimTimer = 0.0f;      // 動畫幀播放計時器
    bool m_IsDead = false;
};

#endif //MARIO_HPP