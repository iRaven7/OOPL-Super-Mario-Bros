#ifndef PIRANHA_PLANT_HPP
#define PIRANHA_PLANT_HPP

#include "Enemy.hpp"
#include "Mario.hpp"
#include "Util/Logger.hpp"
#include "Util/Image.hpp"

class PiranhaPlant : public Enemy {
public:
    enum class State {
        Hiding,     // 躲在水管內
        Emerging,   // 正在上升
        Exposed,    // 完全露出水管
        Retracting  // 正在下降
    };

    bool IsStompable() const override { return false; }

    // 請確保圖片路徑與檔名完全符合您實際放置的位置
    PiranhaPlant(glm::vec2 pipeTopCenterPos) : Enemy(RESOURCE_DIR"/Entities/PiranhaPlant/piranha_plant.png") {
        m_BaseScale = { 1.0f, 1.0f };
        m_Transform.scale = m_BaseScale;

        // 設定運動範圍：假設食人花圖片高度約為 32 像素
        float verticalOffset = 24.0f;
        m_ExposeY = pipeTopCenterPos.y;
        m_HideY = pipeTopCenterPos.y - verticalOffset;

        // 初始狀態設為隱藏
        m_WorldPosition = { pipeTopCenterPos.x - 8.0f , m_HideY };
        SetPosition(m_WorldPosition);

        m_State = State::Hiding;
        m_WaitTimer = m_HideDuration;

        // 【關鍵】：將 Z-Index 設為 30，確保其繪製於水管 (通常為 50) 的後方
        SetZIndex(5);
    }

    void UpdateRenderPosition(float cameraX, float cameraZoom) override {
        m_Transform.translation.x = (m_WorldPosition.x - cameraX) * cameraZoom;
        m_Transform.translation.y = m_WorldPosition.y * cameraZoom;
        m_Transform.scale = m_BaseScale * cameraZoom;
    }

    void UpdateAI(float deltaTime, const std::vector<std::shared_ptr<Block>>& /*blocks*/) override {
        if (!m_IsActive) return;

        // 食人花的 FSM：不受重力影響，純依靠計時器切換狀態
        switch (m_State) {
        case State::Hiding:
            m_WaitTimer -= deltaTime;
            if (m_WaitTimer <= 0.0f) {
                m_State = State::Emerging;
            }
            break;
        case State::Emerging:
            m_WorldPosition.y += m_MoveSpeed * deltaTime;
            if (m_WorldPosition.y >= m_ExposeY) {
                m_WorldPosition.y = m_ExposeY;
                m_State = State::Exposed;
                m_WaitTimer = m_ExposeDuration;
            }
            break;
        case State::Exposed:
            m_WaitTimer -= deltaTime;
            if (m_WaitTimer <= 0.0f) {
                m_State = State::Retracting;
            }
            break;
        case State::Retracting:
            m_WorldPosition.y -= m_MoveSpeed * deltaTime;
            if (m_WorldPosition.y <= m_HideY) {
                m_WorldPosition.y = m_HideY;
                m_State = State::Hiding;
                m_WaitTimer = m_HideDuration;
            }
            break;
        }
        SetPosition(m_WorldPosition); // 同步物理碰撞箱的座標
    }

    // 覆寫碰撞介面：無論是由上踩踏或側面碰撞，皆視為瑪利歐受傷
    void OnStomped(Character* hitter) override {
        if (!m_IsActive) return;
        DealDamageToMario(hitter);
    }

    void OnSideCollision(Character* hitter) override {
        if (!m_IsActive) return;
        DealDamageToMario(hitter);
    }

private:
    void DealDamageToMario(Character* hitter) {
        Mario* mario = dynamic_cast<Mario*>(hitter);
        if (mario) {
            mario->TakeDamage();
        }
    }

    State m_State;
    float m_MoveSpeed = 50.0f;
    float m_HideY = 0.0f;
    float m_ExposeY = 0.0f;

    float m_WaitTimer = 0.0f;
    float m_HideDuration = 2.0f;   // 在水管內潛伏的時間 (秒)
    float m_ExposeDuration = 2.0f; // 露出在外咬人的時間 (秒)
};

#endif // PIRANHA_PLANT_HPP