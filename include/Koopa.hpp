#ifndef KOOPA_HPP
#define KOOPA_HPP

#include "Enemy.hpp"
#include "Mario.hpp"
#include "Util/Logger.hpp"

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

    // Hitbox height is a fixed 24px in every state (walking and shell alike).
    glm::vec2 GetSize() const override {
        return { 12.0f, 24.0f };
    }

    void UpdateRenderPosition(float cameraX, float cameraZoom) override {
        // Lift the sprite ~4px above the hitbox centre so the taller koopa art
        // sits correctly on the ground (purely visual; hitbox is unchanged).
        constexpr float kSpriteYOffset = 4.0f;
        m_Transform.translation.x = (m_WorldPosition.x - cameraX) * cameraZoom;
        m_Transform.translation.y = (m_WorldPosition.y + kSpriteYOffset) * cameraZoom;

        float direction = (m_Velocity.x > 0.0f) ? -1.0f : 1.0f;
        m_Transform.scale.x = m_BaseScale.x * cameraZoom * direction;
        m_Transform.scale.y = m_BaseScale.y * cameraZoom;
    }

    void UpdateAI(float deltaTime, const std::vector<std::shared_ptr<Block>>& blocks) override {
        if (!m_IsActive) return;

        if (m_KickGraceTimer > 0.0f) m_KickGraceTimer -= deltaTime;

        m_AnimTimer += deltaTime;
        switch (m_State) {
        case State::Walking: {
            int frame = static_cast<int>(m_AnimTimer * 8.0f) % 2;
            SetImage(frame == 0 ? RESOURCE_DIR"/Entities/Koopa/koopa1.png"
                                : RESOURCE_DIR"/Entities/Koopa/koopa2.png");
            break;
        }
        case State::ShellMoving: {
            static const char* spinFrames[] = {
                RESOURCE_DIR"/Entities/Koopa/koopa_spinning1.png",
                RESOURCE_DIR"/Entities/Koopa/koopa_spinning2.png",
                RESOURCE_DIR"/Entities/Koopa/koopa_spinning3.png"
            };
            SetImage(spinFrames[static_cast<int>(m_AnimTimer * 15.0f) % 3]);
            break;
        }
        case State::ShellIdle:
            break; // static shell.png set in OnStomped
        }

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

            SetImage(RESOURCE_DIR"/Entities/Koopa/shell.png");

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

        if (m_State == State::Walking) {
            if (mario) {
                mario->TakeDamage();
            }
        }
        else if (m_State == State::ShellMoving) {
            // Grace window right after a kick: Mario (especially at sprint speed)
            // is still overlapping the shell he just kicked — don't punish him for it.
            if (m_KickGraceTimer <= 0.0f && mario) {
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
        m_AnimTimer = 0.0f;
        m_KickGraceTimer = m_KickGrace;

        // Push the shell clear of Mario's hitbox in the kick direction so it can't
        // re-collide as a "moving shell" on the very next frame.
        float clearance = (hitter->GetSize().x + GetSize().x) / 2.0f + 2.0f;
        glm::vec2 hitterPos = hitter->GetPosition();

        if (hitterPos.x < GetPosition().x) {
            m_Velocity.x = m_ShellSpeed;
            SetPosition({ hitterPos.x + clearance, GetPosition().y });
        }
        else {
            m_Velocity.x = -m_ShellSpeed;
            SetPosition({ hitterPos.x - clearance, GetPosition().y });
        }
        LOG_INFO("Shell kicked!");
    }

    State m_State;
    float m_WalkSpeed       = 35.0f;
    float m_ShellSpeed      = 200.0f;
    float m_AnimTimer       = 0.0f;
    float m_KickGraceTimer  = 0.0f;
    float m_KickGrace       = 0.25f;  // seconds of no-damage right after a kick
};

#endif // KOOPA_HPP
