#ifndef MARIO_HPP
#define MARIO_HPP

#include "Character.hpp"

class Mario : public Character {
public:
    Mario() : Character(RESOURCE_DIR"/Entities/Player/mario.png") {
        SetZIndex(50);
    }
};

#endif //MARIO_HPP