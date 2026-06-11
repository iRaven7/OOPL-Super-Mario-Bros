#include "Mario.hpp"
#include "MarioState.hpp"      
#include "GameStateManager.hpp" 
#include "Util/Logger.hpp"

Mario::Mario() : Character(RESOURCE_DIR"/Entities/LittleMario/mario.png") {
    SetZIndex(50);
    ChangeState(std::make_unique<SmallMarioState>());
}

bool Mario::IsControlLocked() const {
    return dynamic_cast<PoleSlideState*>(m_State.get()) != nullptr ||
        dynamic_cast<PipeSlideState*>(m_State.get()) != nullptr;
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
            SetImage(m_State->GetIdleImage());
            m_Transform.scale.x = 1.0f;
        }
        else if (poleState) {
            m_Transform.scale.x = 1.0f;
            float animSpeed = std::abs(m_Velocity.x) / 150.0f;
            m_AnimTimer += deltaTime * animSpeed;
            auto frames = m_State->GetRunImages();
            if (!frames.empty()) {
                int frameIndex = static_cast<int>(m_AnimTimer * 5.0f) % frames.size();
                SetImage(frames[frameIndex]);
            }
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
    float yOffset = 0.0f;

    if (m_IsCrouching && m_State && m_State->GetHitboxSize().y > 16.0f) {
        yOffset = 8.0f;
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
        m_Visible = true;
    }
}

void Mario::TakeDamage() {
    if (IsInvincible() || IsControlLocked()) return;

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
        LOG_INFO("�o�g���y�I");
    }
}

void Mario::Die() {
    m_IsDead = true;
}

glm::vec2 Mario::UpdatePhysics(float deltaTime, float inputDirection, bool isSprinting, bool wantsJump, const std::vector<std::shared_ptr<Block>>& blocks) {
    // �ˬd�X���޿�
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
        else {
            auto res = Character::UpdatePhysics(deltaTime, 1.0f, false, false, blocks);
            m_Velocity.x = poleState->GetWalkSpeed();

            if (GetPosition().x > poleState->GetPoleX() + 300.0f) {
                GameStateManager::GetInstance().SetLevelComplete(true);
            }
            return res;
        }
    } // �o�̤j�A���n���T�����I

    if (auto pipeState = dynamic_cast<PipeSlideState*>(m_State.get())) {
        if (!pipeState->IsDownReached()) {
            glm::vec2 pos = GetPosition();
            pos.y -= pipeState->GetSlideSpeed() * deltaTime;
            SetPosition(pos);
            m_Velocity = { 0.0f, -pipeState->GetSlideSpeed() };

            if (pos.y <= pipeState->GetTargetY()) {
                pipeState->SetDownReached(true);
            }
        }
        return { 0.0f, 0.0f };
    }

    // ���`���A
    return Character::UpdatePhysics(deltaTime, inputDirection, isSprinting, wantsJump, blocks);
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