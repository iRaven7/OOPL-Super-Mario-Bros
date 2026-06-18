#include "Mario.hpp"
#include "MarioState.hpp"
#include "GameStateManager.hpp"
#include "Util/Logger.hpp"
#include "Constants.hpp"

Mario::Mario() : Character(RESOURCE_DIR"/Entities/LittleMario/mario.png") {
    SetZIndex(50);
    ChangeState(std::make_unique<SmallMarioState>());
}

bool Mario::IsControlLocked() const {
    return dynamic_cast<PoleSlideState*>(m_State.get()) != nullptr ||
        dynamic_cast<PipeSlideState*>(m_State.get()) != nullptr ||
        dynamic_cast<PipeExitState*>(m_State.get()) != nullptr;
}

void Mario::ChangeState(std::unique_ptr<MarioState> newState, bool triggerPause) {
    m_State = std::move(newState);
    if (triggerPause) {
        m_TransformTimer = 1.0f;
    }
    else {
        m_TransformTimer = 0.0f;
    }
}

bool Mario::CanBreakBlocks() const {
    return m_State ? m_State->CanBreakBlocks() : false;
}

void Mario::SetCrouching(bool crouching) {
    if (IsControlLocked()) crouching = false;
    if (m_IsCrouching == crouching) return;
    m_IsCrouching = crouching;

    if (m_State && m_State->GetHitboxSize().y > 16.0f) {
        if (m_IsCrouching) {
            m_WorldPosition.y -= 8.0f;
        }
        else {
            m_WorldPosition.y += 8.0f;
        }
    }
}

glm::vec2 Mario::GetSize() const {
    glm::vec2 baseSize = m_State ? m_State->GetHitboxSize() : Character::GetSize();
    if (m_IsCrouching && baseSize.y > 16.0f) {
        return { baseSize.x, baseSize.y * 0.5f };
    }
    return baseSize;
}

void Mario::UpdateTransformation(float deltaTime) {
    if (m_TransformTimer > 0.0f) {
        m_TransformTimer -= deltaTime;
        m_Visible = (static_cast<int>(m_TransformTimer * 20) % 2 == 0);
    }
    else {
        m_TransformTimer = 0.0f;
        m_Visible = true;
    }
}

void Mario::UpdateAnimation(float deltaTime, float inputDirection) {
    if (!m_State) return;

    if (IsControlLocked()) {
        auto poleState = dynamic_cast<PoleSlideState*>(m_State.get());
        if (poleState && !poleState->IsBottomReached()) {
            SetImage(m_State->GetSlideImage());
            m_Transform.scale.x = 1.0f;
        }
        else if (poleState && !poleState->IsPauseCompleted()) {
            SetImage(m_State->GetSlideImage());
            m_Transform.scale.x = 1.0f;
        }
        else if (poleState) {
            m_Transform.scale.x = 1.0f;
            m_AnimTimer += deltaTime;
            auto frames = m_State->GetRunImages();
            if (!frames.empty()) {
                int frameIndex = static_cast<int>(m_AnimTimer * 8.0f) % frames.size();
                SetImage(frames[frameIndex]);
            }
        }
        else if (dynamic_cast<PipeExitState*>(m_State.get())) {
            SetImage(m_State->GetIdleImage());
            m_Transform.scale.x = 1.0f;
        }
        return;
    }

    if (inputDirection < 0.0f) m_Transform.scale.x = -1.0f;
    else if (inputDirection > 0.0f) m_Transform.scale.x = 1.0f;

    AnimState newState = AnimState::IDLE;

    if (m_IsCrouching) {
        newState = AnimState::CROUCH;
    }
    else if (!m_IsGrounded) {
        newState = AnimState::JUMP;
    }
    else if (inputDirection != 0.0f && std::signbit(m_Velocity.x) != std::signbit(inputDirection) && std::abs(m_Velocity.x) > 50.0f) {
        newState = AnimState::SKID;
    }
    else if (std::abs(m_Velocity.x) > 10.0f) {
        newState = AnimState::RUN;
    }

    if (newState == AnimState::CROUCH) {
        SetImage(m_State->GetCrouchImage());
    }
    else if (newState == AnimState::JUMP) {
        SetImage(m_State->GetJumpImage());
    }
    else if (newState == AnimState::SKID) {
        SetImage(m_State->GetSkidImage());
    }
    else if (newState == AnimState::RUN) {
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

void Mario::UpdateRenderPosition(float cameraX, float cameraZoom) {
    // 0.5f shifts the sprite down by half the 1-pixel hitbox reduction so the
    // sprite bottom stays flush with the ground even though the hitbox is shorter.
    float yOffset = 0.5f;

    if (m_IsCrouching && m_State && m_State->GetHitboxSize().y > 16.0f) {
        yOffset += 8.0f;
    }

    m_Transform.translation.x = (m_WorldPosition.x - cameraX) * cameraZoom;
    m_Transform.translation.y = (m_WorldPosition.y + yOffset) * cameraZoom;

    float direction = (m_Transform.scale.x < 0.0f) ? -1.0f : 1.0f;
    m_Transform.scale.x = m_BaseScale.x * cameraZoom * direction;
    m_Transform.scale.y = m_BaseScale.y * cameraZoom;
}

void Mario::Update(float deltaTime) {
    if (m_ShootCooldown > 0.0f) m_ShootCooldown -= deltaTime;

    if (m_InvincibleTimer > 0.0f) {
        m_InvincibleTimer -= deltaTime;
        m_Visible = (static_cast<int>(m_InvincibleTimer * 10) % 2 == 0);
    }
    else {
        m_InvincibleTimer = 0.0f;
        if (!m_PoleWalkInvisible) m_Visible = true;
    }

    if (m_StarTimer > 0.0f) {
        m_StarTimer -= deltaTime;
        if (m_StarTimer <= 0.0f) m_StarTimer = 0.0f;
        m_Visible = (static_cast<int>(m_StarTimer * 20) % 2 == 0);
    }
}

void Mario::TakeDamage() {
    if (IsInvincible() || IsControlLocked() || IsStarPowered()) return;

    if (dynamic_cast<SmallMarioState*>(m_State.get()) != nullptr) {
        Die();
    }
    else {
        ChangeState(std::make_unique<SmallMarioState>(), true);
        m_InvincibleTimer = 2.0f;
    }
}

std::vector<std::shared_ptr<Fireball>> Mario::PopSpawnedFireballs() {
    auto res = m_SpawnedFireballs;
    m_SpawnedFireballs.clear();
    return res;
}

void Mario::Shoot() {
    if (IsControlLocked()) return;
    if (m_State && m_State->CanShoot() && !m_IsCrouching && m_ShootCooldown <= 0.0f) {
        float facing = (m_Transform.scale.x > 0.0f) ? 1.0f : -1.0f;
        glm::vec2 spawnPos = { m_WorldPosition.x + facing * 16.0f, m_WorldPosition.y + 8.0f };
        m_SpawnedFireballs.push_back(std::make_shared<Fireball>(spawnPos, facing));

        m_ShootCooldown = 0.3f;
        LOG_INFO("Fireball fired!");
    }
}

void Mario::Die() {
    m_IsDead = true;
}

void Mario::StartDeathAnimation() {
    SetImage(RESOURCE_DIR"/Entities/LittleMario/mario_dead.png");
    m_Velocity = { 0.0f, 0.0f };
    m_DeathAnimTimer = 0.0f;
    m_DeathVelocityY = 0.0f;
    m_DeathBounceStarted = false;
    m_Visible = true;
}

void Mario::UpdateDeathAnimation(float deltaTime) {
    m_DeathAnimTimer += deltaTime;
    if (m_DeathAnimTimer < 0.5f) return;

    if (!m_DeathBounceStarted) {
        m_DeathBounceStarted = true;
        m_DeathVelocityY = 700.0f;
    }

    m_DeathVelocityY += PhysicsConstants::GRAVITY * deltaTime;
    m_WorldPosition.y += m_DeathVelocityY * deltaTime;
    SetPosition(m_WorldPosition);
}

glm::vec2 Mario::UpdatePhysics(float deltaTime, float inputDirection, bool isSprinting, bool wantsJump, const std::vector<std::shared_ptr<Block>>& blocks, bool isJumpHeld) {
    if (auto poleState = dynamic_cast<PoleSlideState*>(m_State.get())) {
        if (!poleState->IsBottomReached()) {
            m_Velocity.x = 0.0f;
            m_Velocity.y = -poleState->GetSlideSpeed();

            auto res = Character::UpdatePhysics(deltaTime, 0.0f, false, false, blocks);
            m_Velocity.y = -poleState->GetSlideSpeed();

            if (m_IsGrounded) {
                poleState->SetBottomReached(true);
                SetPosition({ GetPosition().x + 20.0f, GetPosition().y });
            }
            return res;
        }
        else if (!poleState->IsPauseCompleted()) {
            poleState->TickPause(deltaTime);
            auto res = Character::UpdatePhysics(deltaTime, 0.0f, false, false, blocks);
            m_Velocity.x = 0.0f;
            return res;
        }
        else {
            auto res = Character::UpdatePhysics(deltaTime, 1.0f, false, false, blocks);
            m_Velocity.x = poleState->GetWalkSpeed();

            if (!m_PoleWalkInvisible && GetPosition().x >= poleState->GetInvisibleThresholdX()) {
                m_PoleWalkInvisible = true;
                m_Visible = false;
            }

            if (GetPosition().x > poleState->GetPoleX() + 300.0f) {
                GameStateManager::GetInstance().SetLevelComplete(true);
            }
            return res;
        }
    }

    if (auto exitState = dynamic_cast<PipeExitState*>(m_State.get())) {
        if (!exitState->IsExitDone()) {
            glm::vec2 pos = GetPosition();
            pos.y += exitState->GetExitSpeed() * deltaTime;
            if (pos.y >= exitState->GetTargetY()) {
                pos.y = exitState->GetTargetY();
                exitState->SetExitDone(true);
            }
            SetPosition(pos);
        }
        return { 0.0f, 0.0f };
    }

    if (auto pipeState = dynamic_cast<PipeSlideState*>(m_State.get())) {
        if (!pipeState->IsDownReached()) {
            glm::vec2 pos = GetPosition();
            const float speed = pipeState->GetSlideSpeed();

            switch (pipeState->GetSlideDir()) {
            case PipeSlideState::SlideDir::Down:
                pos.y -= speed * deltaTime;
                m_Velocity = { 0.0f, -speed };
                if (pos.y <= pipeState->GetTarget())
                    pipeState->SetDownReached(true);
                break;
            case PipeSlideState::SlideDir::Right:
                pos.x += speed * deltaTime;
                m_Velocity = { speed, 0.0f };
                if (pos.x >= pipeState->GetTarget())
                    pipeState->SetDownReached(true);
                break;
            case PipeSlideState::SlideDir::Left:
                pos.x -= speed * deltaTime;
                m_Velocity = { -speed, 0.0f };
                if (pos.x <= pipeState->GetTarget())
                    pipeState->SetDownReached(true);
                break;
            }
            SetPosition(pos);
        }
        return { 0.0f, 0.0f };
    }

    return Character::UpdatePhysics(deltaTime, inputDirection, isSprinting, wantsJump, blocks, isJumpHeld);
}

void PoleSlideState::Enter(Mario* mario) {
    auto pos = mario->GetPosition();
    mario->SetPosition({ m_PoleX, pos.y });
    mario->SetVelocity({ 0.0f, 0.0f });
}

void PipeSlideState::Enter(Mario* mario) {
    mario->SetVelocity({ 0.0f, 0.0f });
    mario->SetZIndex(5);
}

void PipeExitState::Enter(Mario* mario) {
    mario->SetVelocity({ 0.0f, 0.0f });
    mario->SetZIndex(50);
}
