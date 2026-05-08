#ifndef COIN_HPP
#define COIN_HPP

#include "Item.hpp"
#include "GameStateManager.hpp"

class Coin : public Item {
public:
    explicit Coin(glm::vec2 startPos) : Item(RESOURCE_DIR"/Items/coin.png") {
        SetPosition(startPos);
        m_SpawnStartY = startPos.y;
        SetZIndex(5);
        m_Velocity.y = 400.0f; // 給予初始向上彈跳速度
    }

    void Update(float deltaTime, const std::vector<std::shared_ptr<Block>>&) override {
        if (m_State == State::SPAWNING) {
            m_Velocity.y += GRAVITY * deltaTime;
            glm::vec2 pos = GetPosition();
            pos.y += m_Velocity.y * deltaTime;
            SetPosition(pos);

            // 當金幣掉回原本的方塊高度時，便消失 (未來可在此處觸發加分與音效)
            if (m_Velocity.y < 0.0f && pos.y <= m_SpawnStartY + 16.0f) {
                m_IsActive = false;
                m_Visible = false;
            }
        }
    }

    void OnCollect(Mario*) override {
        // 從磚塊頂出的金幣會自動消失，通常不會透過瑪利歐主動「觸碰收集」
        m_IsActive = false;
        m_Visible = false;
        GameStateManager::GetInstance().AddCoin(1);
        GameStateManager::GetInstance().AddScore(200);
    }

private:
    enum class State { SPAWNING };
    State m_State = State::SPAWNING;
    float m_SpawnStartY = 0.0f;
    static constexpr float GRAVITY = -1500.0f;
};

#endif // COIN_HPP