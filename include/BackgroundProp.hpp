#ifndef BACKGROUND_PROP_HPP
#define BACKGROUND_PROP_HPP

#include "Block.hpp"

class BackgroundProp : public Block {
public:
    explicit BackgroundProp(const std::string& imagePath) : Block(imagePath) {
        SetZIndex(1); // 放在背景層，避免擋住瑪利歐
    }

    // 回傳 false，完美騙過碰撞系統
    bool IsActive() const override { return false; }
};

#endif // BACKGROUND_PROP_HPP