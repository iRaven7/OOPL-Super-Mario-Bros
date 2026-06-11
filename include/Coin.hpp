#ifndef COIN_HPP
#define COIN_HPP

#include "Item.hpp"
#include "GameStateManager.hpp"

class Coin : public Item {
public:
    explicit Coin(glm::vec2 startPos, bool isStatic = false) : Item(RESOURCE_DIR"/Items/coin.png") {
        SetPosition(startPos);
        m_SpawnStartY = startPos.y;
        SetZIndex(5);
        if (!isStatic) {
            m_State = State::SPAWNING;
            m_Velocity.y = 400.0f;
        }
    }

    void Update(float deltaTime, const std::vector<std::shared_ptr<Block>>&) override {
        if (m_State == State::SPAWNING) {
            m_Velocity.y += GRAVITY * deltaTime;
            glm::vec2 pos = GetPosition();
            pos.y += m_Velocity.y * deltaTime;
            SetPosition(pos);

            if (m_Velocity.y < 0.0f && pos.y <= m_SpawnStartY + 16.0f) {
                m_IsActive = false;
                m_Visible = false;
            }
        }
    }

    void OnCollect(Mario*) override {
        m_IsActive = false;
        m_Visible = false;
        GameStateManager::GetInstance().AddCoin(1);
        GameStateManager::GetInstance().AddScore(200);
    }

private:
    enum class State { IDLE, SPAWNING };
    State m_State = State::IDLE;
    float m_SpawnStartY = 0.0f;
    static constexpr float GRAVITY = -1500.0f;
};

#endif // COIN_HPP