#ifndef MARIO_HPP
#define MARIO_HPP

#include "Character.hpp"
#include "MarioState.hpp"
#include <memory>

class Mario : public Character {
public:
    Mario() : Character(RESOURCE_DIR"/Entities/Player/mario.png") {
        SetZIndex(50);
        // 預設為小型瑪利歐
        ChangeState(std::make_unique<SmallMarioState>());
    }

    // 切換狀態的對外介面
    void ChangeState(std::unique_ptr<MarioState> newState) {
        m_State = std::move(newState);
    }

    // 委派給當前狀態物件
    bool CanBreakBlocks() const override {
        return m_State ? m_State->CanBreakBlocks() : false;
    }

    // 委派給當前狀態物件
    glm::vec2 GetSize() const override {
        return m_State ? m_State->GetHitboxSize() : Character::GetSize();
    }

private:
    std::unique_ptr<MarioState> m_State;
};

#endif //MARIO_HPP