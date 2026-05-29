#ifndef ENEMY_HPP
#define ENEMY_HPP

#include "Character.hpp"
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
    virtual void OnFireballHit() {
        m_IsActive = false;
        m_Visible = false;
    }

protected:
    bool m_IsActive = true;
    // 這裡原本多餘的 m_BaseScale 幫你拿掉了
};

#endif // ENEMY_HPP