#ifndef MUSHROOM_HPP
#define MUSHROOM_HPP

#include "Item.hpp"
#include "GameStateManager.hpp"
#include "Mario.hpp"      // �ɤW�o��
#include "MarioState.hpp" // �ɤW�o��

class Mushroom : public Item {
public:
    explicit Mushroom(glm::vec2 startPos) : Item(RESOURCE_DIR"/Items/mushroom.png") {
        SetPosition(startPos);
        m_SpawnStartY = startPos.y;
        SetZIndex(5);
    }

    void Update(float deltaTime, const std::vector<std::shared_ptr<Block>>& blocks) override {
        if (m_State == State::SPAWNING) {
            glm::vec2 pos = GetPosition();
            pos.y += 50.0f * deltaTime;
            SetPosition(pos);

            if (pos.y >= m_SpawnStartY + 16.0f) {
                m_State = State::MOVING;
                SetZIndex(40);
                m_Velocity.x = 100.0f;
            }
        }
        else if (m_State == State::MOVING) {
            m_Velocity.y += GRAVITY * deltaTime;
            glm::vec2 currentPos = GetPosition();
            glm::vec2 mySize = GetSize();

            currentPos.x += m_Velocity.x * deltaTime;
            for (const auto& block : blocks) {
                if (!block->IsActive()) continue;
                if (CheckAABB(currentPos, mySize, block->GetCollisionPosition(), block->GetSize())) {
                    m_Velocity.x = -m_Velocity.x;
                    if (m_Velocity.x > 0) {
                        currentPos.x = block->GetCollisionPosition().x + (block->GetSize().x / 2.0f) + (mySize.x / 2.0f);
                    }
                    else {
                        currentPos.x = block->GetCollisionPosition().x - (block->GetSize().x / 2.0f) - (mySize.x / 2.0f);
                    }
                }
            }

            currentPos.y += m_Velocity.y * deltaTime;
            for (const auto& block : blocks) {
                if (!block->IsActive()) continue;
                if (CheckAABB(currentPos, mySize, block->GetCollisionPosition(), block->GetSize())) {
                    if (m_Velocity.y < 0.0f) {
                        currentPos.y = block->GetCollisionPosition().y + (block->GetSize().y / 2.0f) + (mySize.y / 2.0f);
                        m_Velocity.y = 0.0f;
                    }
                }
            }
            SetPosition(currentPos);
        }
    }

    void OnCollect(Mario* mario) override {
        m_IsActive = false;
        m_Visible = false;
        GameStateManager::GetInstance().AddScore(1000);
        if (dynamic_cast<SmallMarioState*>(mario->GetState())) {
            mario->ChangeState(std::make_unique<BigMarioState>());
        }
    }

private:
    bool CheckAABB(const glm::vec2& posA, const glm::vec2& sizeA, const glm::vec2& posB, const glm::vec2& sizeB) const {
        return std::abs(posA.x - posB.x) < (sizeA.x + sizeB.x) / 2.0f &&
            std::abs(posA.y - posB.y) < (sizeA.y + sizeB.y) / 2.0f;
    }

    enum class State { SPAWNING, MOVING };
    State m_State = State::SPAWNING;
    float m_SpawnStartY = 0.0f;
    static constexpr float GRAVITY = -1500.0f;
};

#endif