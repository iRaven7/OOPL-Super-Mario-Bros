#ifndef ENEMY_HPP
#define ENEMY_HPP

#include "Character.hpp"

class Enemy : public Character {
public:
    explicit Enemy(const std::string& imagePath) : Character(imagePath) {}

    virtual ~Enemy() = default;

    // 戰鬥互動介面
    virtual void OnStomped(Character* hitter) = 0;       // 被由上往下踩踏
    virtual void OnSideCollision(Character* hitter) = 0; // 側面相撞
    virtual void UpdateAI(float deltaTime, const std::vector<std::shared_ptr<Block>>& blocks) = 0;

    virtual bool IsStompable() const { return true; }
    bool IsActive() const { return m_IsActive; }
    virtual void OnFireballHit() {
        m_IsActive = false;
        m_Visible = false; // 隱藏圖片
        // 可在此處呼叫 GameStateManager 加分
    }

protected:
    bool m_IsActive = true;
    glm::vec2 m_BaseScale;
};

#endif // ENEMY_HPP