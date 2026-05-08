#ifndef MUSHROOM_HPP
#define MUSHROOM_HPP

#include "Item.hpp"
#include "GameStateManager.hpp"

class Mushroom : public Item {
public:
    explicit Mushroom(glm::vec2 startPos) : Item(RESOURCE_DIR"/Items/mushroom.png") {
        SetPosition(startPos);
        m_SpawnStartY = startPos.y;
        SetZIndex(5); // 生成時 ZIndex 較低，使其被方塊遮擋 (方塊為 10)
    }

    void Update(float deltaTime, const std::vector<std::shared_ptr<Block>>& blocks) override {
        if (m_State == State::SPAWNING) {
            glm::vec2 pos = GetPosition();
            pos.y += 50.0f * deltaTime; // 緩慢上升
            SetPosition(pos);

            // 當上升超過一個方塊的高度 (16.0f) 時，切換為移動狀態
            if (pos.y >= m_SpawnStartY + 16.0f) {
                m_State = State::MOVING;
                SetZIndex(40); // 移至前景
                m_Velocity.x = 100.0f; // 初始向右移動
            }
        }
        else if (m_State == State::MOVING) {
            m_Velocity.y += GRAVITY * deltaTime;
            glm::vec2 currentPos = GetPosition();
            glm::vec2 mySize = GetSize();

            // X 軸移動與碰撞 (碰到牆壁反彈)
            currentPos.x += m_Velocity.x * deltaTime;
            for (const auto& block : blocks) {
                if (!block->IsActive()) continue;
                if (CheckAABB(currentPos, mySize, block->GetCollisionPosition(), block->GetSize())) {
                    m_Velocity.x = -m_Velocity.x; // 反轉速度方向
                    if (m_Velocity.x > 0) {
                        currentPos.x = block->GetCollisionPosition().x + (block->GetSize().x / 2.0f) + (mySize.x / 2.0f);
                    }
                    else {
                        currentPos.x = block->GetCollisionPosition().x - (block->GetSize().x / 2.0f) - (mySize.x / 2.0f);
                    }
                }
            }

            // Y 軸移動與碰撞 (落地)
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
        // 吃到蘑菇切換為大型態
        mario->ChangeState(std::make_unique<BigMarioState>());
        GameStateManager::GetInstance().AddScore(1000);
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