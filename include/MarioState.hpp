#ifndef MARIO_STATE_HPP
#define MARIO_STATE_HPP

#include <glm/glm.hpp>

// 狀態基底介面
class MarioState {
public:
    virtual ~MarioState() = default;

    // 定義不同狀態下的專屬數值與能力
    virtual glm::vec2 GetHitboxSize() const = 0;
    virtual bool CanBreakBlocks() const = 0;
};

// 小型態實作
class SmallMarioState : public MarioState {
public:
    glm::vec2 GetHitboxSize() const override {
        return { 16.0f, 16.0f };
    }
    bool CanBreakBlocks() const override {
        return false;
    }
};

// 大型態實作
class BigMarioState : public MarioState {
public:
    glm::vec2 GetHitboxSize() const override {
        // 註：大型瑪利歐碰撞框變高為 32.0f
        return { 16.0f, 32.0f };
    }
    bool CanBreakBlocks() const override {
        return true;
    }
};

#endif // MARIO_STATE_HPP