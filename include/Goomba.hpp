#ifndef GOOMBA_HPP
#define GOOMBA_HPP

#include "Enemy.hpp"
#include "Mario.hpp"
#include "GameStateManager.hpp"
#include "Util/Logger.hpp"

class Goomba : public Enemy {
public:
    Goomba(glm::vec2 startPos) : Enemy(RESOURCE_DIR"/Entities/Goomba/goomba_walk1.png") {
        SetPosition(startPos);
        m_Velocity.x = -m_WalkSpeed;
        SetZIndex(40);
    }

    void UpdateAI(float deltaTime, const std::vector<std::shared_ptr<Block>>& blocks) override {
        if (!m_IsActive) return;

        // Squash state: show flattened frame for a brief moment then deactivate.
        if (m_IsSquashed) {
            m_SquashTimer -= deltaTime;
            if (m_SquashTimer <= 0.0f) {
                m_IsActive = false;
                m_Visible  = false;
            }
            return;
        }

        // Walk animation: alternate frames at ~8 fps.
        m_AnimTimer += deltaTime;
        int frame = static_cast<int>(m_AnimTimer * 8.0f) % 2;
        SetImage(frame == 0 ? RESOURCE_DIR"/Entities/Goomba/goomba_walk1.png"
                            : RESOURCE_DIR"/Entities/Goomba/goomba_walk2.png");

        float inputDirection = (m_Velocity.x > 0.0f) ? 0.5f : -0.5f;
        UpdatePhysics(deltaTime, inputDirection, false, false, blocks);

        if (m_Velocity.x == 0.0f) {
            m_Velocity.x = -inputDirection * m_WalkSpeed;
        } else {
            m_Velocity.x = inputDirection * m_WalkSpeed;
        }
    }

    void OnStomped(Character* /*hitter*/) override {
        if (!m_IsActive || m_IsSquashed) return;
        m_IsSquashed  = true;
        m_SquashTimer = 0.4f;
        SetImage(RESOURCE_DIR"/Entities/Goomba/goomba_step_on.png");
        GameStateManager::GetInstance().AddScore(100);
        LOG_INFO("Goomba stomped!");
    }

    void OnSideCollision(Character* hitter) override {
        if (!m_IsActive) return;
        Mario* mario = dynamic_cast<Mario*>(hitter);
        if (mario) mario->TakeDamage();
    }

private:
    float m_WalkSpeed   = 70.0f;
    float m_AnimTimer   = 0.0f;
    bool  m_IsSquashed  = false;
    float m_SquashTimer = 0.0f;
};

#endif // GOOMBA_HPP
