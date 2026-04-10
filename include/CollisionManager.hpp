#ifndef COLLISION_MANAGER_HPP
#define COLLISION_MANAGER_HPP

#include <vector>
#include <memory>
#include "Mario.hpp"
#include "Block.hpp"
#include "Item.hpp"

class CollisionManager {
public:
    // 處理遊戲中所有動態實體之間的互動 (收集、觸發)
    void ProcessInteractions(Mario* mario, const std::vector<std::shared_ptr<Block>>& blocks, std::vector<std::shared_ptr<Item>>& items);

private:
    // 基礎的 AABB 幾何檢查
    bool CheckAABB(const glm::vec2& posA, const glm::vec2& sizeA, const glm::vec2& posB, const glm::vec2& sizeB) const;
};

#endif // COLLISION_MANAGER_HPP