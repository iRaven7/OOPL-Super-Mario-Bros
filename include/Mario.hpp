#ifndef MARIO_HPP
#define MARIO_HPP

#include "Character.hpp"

enum class MarioState { SMALL, BIG, FIRE };

class Mario : public Character {
public:
    Mario() : Character(RESOURCE_DIR"/Entities/Player/mario.png") { SetZIndex(50); }

    void SetMarioState(MarioState state) { m_State = state; }
    MarioState GetMarioState() const { return m_State; }

    bool CanBreakBlocks() const override {
        return m_State == MarioState::BIG || m_State == MarioState::FIRE;
    }

private:
    MarioState m_State = MarioState::SMALL; // 預設為小型態
};

#endif //MARIO_HPP