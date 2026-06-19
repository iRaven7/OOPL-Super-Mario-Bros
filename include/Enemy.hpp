#ifndef ENEMY_HPP
#define ENEMY_HPP

#include "Character.hpp"
#include "Constants.hpp"
#include <vector>
#include <memory>

class Block;

class Enemy : public Character {
public:
    explicit Enemy(const std::string& imagePath) : Character(imagePath) {}

    virtual ~Enemy() = default;

    virtual void OnStomped(Character* hitter) = 0;
    virtual void OnSideCollision(Character* hitter) = 0;
    virtual void UpdateAI(float deltaTime, const std::vector<std::shared_ptr<Block>>& blocks) = 0;

    virtual bool IsStompable() const { return true; }
    bool IsActive() const { return m_IsActive; }

    // True while the enemy is playing its knock-out death (bounce + fall). Such
    // enemies still render and fall but must no longer interact with Mario,
    // shells, fireballs or each other.
    bool IsDying() const { return m_IsDying; }

    // Fireball / moving shell / star contact "knocks out" the enemy: it pops up
    // a little and then tumbles off the bottom of the screen. Overridden by
    // enemies that should just vanish instead (e.g. Piranha Plant).
    virtual void OnFireballHit() {
        if (m_IsDying || !m_IsActive) return;
        m_IsDying = true;
        m_DeathVelocityY = m_DeathBounceSpeed;   // initial upward bounce
    }

    // Per-frame entry point used by the game loop. Centralises the death
    // animation so every enemy subclass gets it without having to special-case
    // it inside their own UpdateAI.
    void RunAI(float deltaTime, const std::vector<std::shared_ptr<Block>>& blocks) {
        if (!m_IsActive) return;
        if (m_IsDying) { UpdateDeath(deltaTime); return; }
        UpdateAI(deltaTime, blocks);
    }

protected:
    void UpdateDeath(float deltaTime) {
        m_DeathVelocityY += PhysicsConstants::GRAVITY * deltaTime;
        if (m_DeathVelocityY < PhysicsConstants::MAX_FALL_SPEED) {
            m_DeathVelocityY = PhysicsConstants::MAX_FALL_SPEED;
        }
        m_WorldPosition.y += m_DeathVelocityY * deltaTime;
        SetPosition(m_WorldPosition);

        // Once well below the view, retire the enemy for good.
        if (m_WorldPosition.y < -400.0f) {
            m_IsActive = false;
            m_Visible = false;
        }
    }

    bool  m_IsActive = true;
    bool  m_IsDying = false;
    float m_DeathVelocityY = 0.0f;
    float m_DeathBounceSpeed = 250.0f;
    // �o�̭쥻�h�l�� m_BaseScale ���A�����F
};

#endif // ENEMY_HPP
