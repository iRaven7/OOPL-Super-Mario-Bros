#ifndef COLLISION_MANAGER_HPP
#define COLLISION_MANAGER_HPP

#include <vector>
#include <memory>
#include "Mario.hpp"
#include "Block.hpp"
#include "Item.hpp"
#include "Enemy.hpp" // 新增 Enemy 標頭檔

class CollisionManager {
public:
    // 修正函式簽章：加入 enemies 參數
    void ProcessInteractions(Mario* mario,
        const std::vector<std::shared_ptr<Block>>& blocks,
        std::vector<std::shared_ptr<Item>>& items,
        std::vector<std::shared_ptr<Enemy>>& enemies);

private:
    bool CheckAABB(const glm::vec2& posA, const glm::vec2& sizeA, const glm::vec2& posB, const glm::vec2& sizeB) const;
};

#endif // COLLISION_MANAGER_HPP