#ifndef MARIO_HPP
#define MARIO_HPP

#include "Character.hpp"
#include "MarioState.hpp"
#include "Fireball.hpp"
#include <vector>
#include <memory>
#include <cmath>

class Mario : public Character {
public:
    enum class AnimState { IDLE, RUN, JUMP, SKID, CROUCH };

    Mario();

    void ChangeState(std::unique_ptr<MarioState> newState, bool triggerPause = true);
    MarioState* GetState() const { return m_State.get(); }
    [[nodiscard]] bool IsInvincible() const { return m_InvincibleTimer > 0.0f; }
    [[nodiscard]] bool IsStarPowered() const { return m_StarTimer > 0.0f; }
    void SetPoleWalkInvisible(bool v) { m_PoleWalkInvisible = v; }
    [[nodiscard]] bool IsPoleWalkInvisible() const { return m_PoleWalkInvisible; }
    void ActivateStarPower(float duration) { m_StarTimer = duration; }
    [[nodiscard]] bool CanBreakBlocks() const override;
    [[nodiscard]] bool IsControlLocked() const;

    void SetCrouching(bool crouching);
    [[nodiscard]] bool IsCrouching() const { return m_IsCrouching; }
    // True if the full standing hitbox is clear of solid blocks, i.e. Mario may
    // rise from a crouch without clipping into a block directly above his head.
    [[nodiscard]] bool CanStandUp(const std::vector<std::shared_ptr<Block>>& blocks) const;

    [[nodiscard]] glm::vec2 GetSize() const override;

    void UpdateTransformation(float deltaTime);
    [[nodiscard]] bool IsTransforming() const { return m_TransformTimer > 0.0f; }

    void UpdateAnimation(float deltaTime, float inputDirection);
    void UpdateRenderPosition(float cameraX, float cameraZoom) override;

    void Update(float deltaTime);
    void TakeDamage();

    void StartDeathAnimation();
    void UpdateDeathAnimation(float deltaTime);

    std::vector<std::shared_ptr<Fireball>> PopSpawnedFireballs();
    void Shoot();

    void Die();
    [[nodiscard]] bool IsDead() const { return m_IsDead; }

    glm::vec2 UpdatePhysics(float deltaTime, float inputDirection, bool isSprinting, bool wantsJump, const std::vector<std::shared_ptr<Block>>& blocks, bool isJumpHeld = true);

private:
    std::unique_ptr<MarioState> m_State;
    bool m_IsCrouching = false;
    // Exact vertical shift applied when entering the current crouch, so standing
    // up reverses precisely that amount regardless of any state change (power
    // up/down) that happened mid-crouch.
    float m_CrouchShift = 0.0f;
    float m_InvincibleTimer = 0.0f;
    float m_StarTimer = 0.0f;
    float m_TransformTimer = 0.0f;
    float m_DeathAnimTimer = 0.0f;
    float m_DeathVelocityY = 0.0f;
    bool  m_DeathBounceStarted = false;
    float m_AnimTimer = 0.0f;
    bool m_IsDead = false;
    bool m_PoleWalkInvisible = false;
    float m_ShootCooldown = 0.0f;
    std::vector<std::shared_ptr<Fireball>> m_SpawnedFireballs;
};

#endif // MARIO_HPP