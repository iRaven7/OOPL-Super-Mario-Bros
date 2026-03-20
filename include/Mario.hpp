#ifndef MARIO_HPP
#define MARIO_HPP

#include "Character.hpp"

class Mario : public Character {
public:
    // 構造函數直接指定圖片
    Mario() : Character(RESOURCE_DIR"/Entities/Player/mario.png") {
        SetZIndex(16); // 確保在方塊前面
    }

    // 未來 Week 7 的狀態機將在此擴充 (例如 EatMushroom(), TakeDamage())
};

#endif