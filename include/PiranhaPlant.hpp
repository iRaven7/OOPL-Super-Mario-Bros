#ifndef PIRANHA_PLANT_HPP
#define PIRANHA_PLANT_HPP

#include "Enemy.hpp"
#include "Mario.hpp"
#include "Util/Logger.hpp"
#include "Util/Image.hpp"

class PiranhaPlant : public Enemy {
public:
    enum class State {
        Hiding,
        Emerging,
        Exposed,
        Retracting
    };

    bool IsStompable() const override { return false; }

    PiranhaPlant(glm::vec2 pipeTopCenterPos) : Enemy(RESOURCE_DIR"/Entities/PiranhaPlant/piranha_plant.png") {
        m_BaseScale = { 1.0f, 1.0f };
        m_Transform.scale = m_BaseScale;

        float verticalOffset = 24.0f;
        m_ExposeY = pipeTopCenterPos.y;
        m_HideY = pipeTopCenterPos.y - verticalOffset;

        m_WorldPosition = { pipeTopCenterPos.x - 8.0f, m_HideY };
        SetPosition(m_WorldPosition);

        m_State = State::Hiding;
        m_WaitTimer = m_HideDuration;

        SetZIndex(5);
    }

    void UpdateRenderPosition(float cameraX, float cameraZoom) override {
        m_Transform.translation.x = (m_WorldPosition.x - cameraX) * cameraZoom;
        m_Transform.translation.y = m_WorldPosition.y * cameraZoom;
        m_Transform.scale = m_BaseScale * cameraZoom;
    }

    // Called by App each frame before UpdateAI so the plant knows where the player is.
    void SetPlayerPos(const glm::vec2& pos) { m_PlayerPos = pos; }

    void UpdateAI(float deltaTime, const std::vector<std::shared_ptr<Block>>& /*blocks*/) override {
        if (!m_IsActive) return;

        m_AnimTimer += deltaTime;

        // Cycle mouth-open / mouth-closed frames while visible.
        if (m_State != State::Hiding) {
            int frame = static_cast<int>(m_AnimTimer * 4.0f) % 2;
            SetImage(frame == 0 ? RESOURCE_DIR"/Entities/PiranhaPlant/piranha_plant.png"
                                : RESOURCE_DIR"/Entities/PiranhaPlant/piranha_plant1.png");
        }

        switch (m_State) {
        case State::Hiding:
            m_WaitTimer -= deltaTime;
            if (m_WaitTimer <= 0.0f) {
                if (IsMarioBlockingPipe()) {
                    m_WaitTimer = 0.3f;  // stay hidden, re-check shortly
                } else {
                    m_State = State::Emerging;
                }
            }
            break;
        case State::Emerging:
            // If Mario hops onto the pipe mid-rise, duck back down instead of
            // shoving him / dealing a hit.
            if (IsMarioBlockingPipe()) {
                m_State = State::Retracting;
                break;
            }
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
        SetPosition(m_WorldPosition);
    }

    void OnStomped(Character* hitter) override {
        if (!m_IsActive) return;
        DealDamageToMario(hitter);
    }

    void OnSideCollision(Character* hitter) override {
        if (!m_IsActive) return;
        DealDamageToMario(hitter);
    }

    // Piranha Plants are excluded from the bounce-and-fall death; they simply
    // disappear (mirrors them retracting into the pipe).
    void OnFireballHit() override {
        m_IsActive = false;
        m_Visible = false;
    }

private:
    // True when Mario is standing on / hovering just above this plant's pipe mouth,
    // so the plant must not pop up. m_WorldPosition.x is the true pipe center; Mario's
    // standing center sits roughly between m_ExposeY and m_ExposeY+8 (pipe-top surface),
    // while ground-level Mario nearby is well below m_ExposeY-16.
    bool IsMarioBlockingPipe() const {
        bool horizontallyOver = std::abs(m_PlayerPos.x - m_WorldPosition.x) < 20.0f;
        bool abovePipeMouth   = m_PlayerPos.y > (m_ExposeY - 16.0f)
                             && m_PlayerPos.y < (m_ExposeY + 48.0f);
        return horizontallyOver && abovePipeMouth;
    }

    void DealDamageToMario(Character* hitter) {
        Mario* mario = dynamic_cast<Mario*>(hitter);
        if (mario) {
            mario->TakeDamage();
        }
    }

    glm::vec2 m_PlayerPos = { -99999.0f, -99999.0f };

    State m_State;
    float m_MoveSpeed    = 35.0f;
    float m_HideY        = 0.0f;
    float m_ExposeY      = 0.0f;

    float m_WaitTimer    = 0.0f;
    float m_HideDuration = 2.0f;
    float m_ExposeDuration = 2.0f;
    float m_AnimTimer    = 0.0f;
};

#endif // PIRANHA_PLANT_HPP
