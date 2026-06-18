#ifndef KOOPA_HPP
#define KOOPA_HPP

#include "Enemy.hpp"
#include "Mario.hpp"
#include "Util/Logger.hpp"
#include "Util/Image.hpp"

class Koopa : public Enemy {
public:
    enum class State {
        Walking,
        ShellIdle,
        ShellMoving
    };

    Koopa(glm::vec2 startPos) : Enemy(RESOURCE_DIR"/Entities/Koopa/koopa1.png") {
        m_BaseScale = { 1.0f, 1.0f };
        m_Transform.scale = m_BaseScale;

        startPos.y += 12.0f;
        SetPosition(startPos);

        m_Velocity.x = -m_WalkSpeed;
        m_State = State::Walking;
        SetZIndex(35);
    }

    void UpdateRenderPosition(float cameraX, float cameraZoom) override {
        float yOffset = 0.0f;

        if (m_State == State::Walking) {
            yOffset = 8.0f;
        }

        m_Transform.translation.x = (m_WorldPosition.x - cameraX) * cameraZoom;
        m_Transform.translation.y = (m_WorldPosition.y + yOffset) * cameraZoom;

        float direction = (m_Velocity.x > 0.0f) ? -1.0f : 1.0f;
        m_Transform.scale.x = m_BaseScale.x * cameraZoom * direction;
        m_Transform.scale.y = m_BaseScale.y * cameraZoom;
    }

    void UpdateAI(float deltaTime, const std::vector<std::shared_ptr<Block>>& blocks) override {
        if (!m_IsActive) return;

        if (m_State == State::ShellIdle) {
            UpdatePhysics(deltaTime, 0.0f, false, false, blocks);
            m_Velocity.x = 0.0f;
            return;
        }

        float currentSpeed = (m_State == State::ShellMoving) ? m_ShellSpeed : m_WalkSpeed;
        float inputDirection = (m_Velocity.x > 0.0f) ? 0.5f : -0.5f;

        UpdatePhysics(deltaTime, inputDirection, false, false, blocks);

        if (m_Velocity.x == 0.0f) {
            m_Velocity.x = -inputDirection * currentSpeed;
        }
        else {
            m_Velocity.x = inputDirection * currentSpeed;
        }
    }

    void OnStomped(Character* hitter) override {
        if (!m_IsActive) return;

        if (m_State == State::Walking) {
            m_State = State::ShellIdle;
            m_Velocity.x = 0.0f;

            SetDrawable(std::make_shared<Util::Image>(RESOURCE_DIR"/Entities/Koopa/shell.png"));

            LOG_INFO("Koopa entered shell state");
        }
        else if (m_State == State::ShellMoving) {
            m_State = State::ShellIdle;
            m_Velocity.x = 0.0f;
            LOG_INFO("Moving shell stopped");
        }
        else if (m_State == State::ShellIdle) {
            KickShell(hitter);
        }
    }

    void OnSideCollision(Character* hitter) override {
        if (!m_IsActive) return;

        Mario* mario = dynamic_cast<Mario*>(hitter);

        if (m_State == State::Walking || m_State == State::ShellMoving) {
            if (mario) {
                mario->TakeDamage();
            }
        }
        else if (m_State == State::ShellIdle) {
            KickShell(hitter);
        }
    }

    State GetState() const { return m_State; }

private:
    void KickShell(Character* hitter) {
        m_State = State::ShellMoving;

        if (hitter->GetPosition().x < GetPosition().x) {
            m_Velocity.x = m_ShellSpeed;
        }
        else {
            m_Velocity.x = -m_ShellSpeed;
        }
        LOG_INFO("Shell kicked!");
    }

    State m_State;
    float m_WalkSpeed = 35.0f;
    float m_ShellSpeed = 200.0f;
};

#endif // KOOPA_HPP
