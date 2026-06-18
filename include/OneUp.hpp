#ifndef ONE_UP_HPP
#define ONE_UP_HPP

#include "Item.hpp"
#include "GameStateManager.hpp"
#include "Mario.hpp"

class OneUp : public Item {
public:
    explicit OneUp(glm::vec2 startPos) : Item(RESOURCE_DIR"/Items/1up.png") {
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
                m_Velocity.x = 60.0f;
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
                    currentPos.x = (m_Velocity.x > 0)
                        ? block->GetCollisionPosition().x + (block->GetSize().x + mySize.x) / 2.0f
                        : block->GetCollisionPosition().x - (block->GetSize().x + mySize.x) / 2.0f;
                }
            }

            currentPos.y += m_Velocity.y * deltaTime;
            for (const auto& block : blocks) {
                if (!block->IsActive()) continue;
                if (CheckAABB(currentPos, mySize, block->GetCollisionPosition(), block->GetSize())) {
                    if (m_Velocity.y < 0.0f) {
                        currentPos.y = block->GetCollisionPosition().y + (block->GetSize().y + mySize.y) / 2.0f;
                        m_Velocity.y = 0.0f;
                    }
                }
            }
            SetPosition(currentPos);
        }
    }

    void OnCollect(Mario*) override {
        m_IsActive = false;
        m_Visible = false;
        GameStateManager::GetInstance().AddLife(1);
        GameStateManager::GetInstance().AddScore(1000);
    }

private:
    bool CheckAABB(const glm::vec2& a, const glm::vec2& sa, const glm::vec2& b, const glm::vec2& sb) const {
        return std::abs(a.x - b.x) < (sa.x + sb.x) / 2.0f &&
               std::abs(a.y - b.y) < (sa.y + sb.y) / 2.0f;
    }

    enum class State { SPAWNING, MOVING };
    State m_State = State::SPAWNING;
    float m_SpawnStartY = 0.0f;
    static constexpr float GRAVITY = -1500.0f;
};

#endif // ONE_UP_HPP
