// include/CollisionManager.hpp
#ifndef COLLISION_MANAGER_HPP
#define COLLISION_MANAGER_HPP

#include <vector>
#include <memory>
#include <glm/glm.hpp>

// 前置宣告，取代原本的 include
class Mario;
class Block;
class Item;
class Enemy;
class Fireball;

class CollisionManager {
public:
    void ProcessInteractions(Mario* mario,
        const std::vector<std::shared_ptr<Block>>& blocks,
        std::vector<std::shared_ptr<Item>>& items,
        std::vector<std::shared_ptr<Enemy>>& enemies,
        std::vector<std::shared_ptr<Fireball>>& fireballs);

private:
    bool CheckAABB(const glm::vec2& posA, const glm::vec2& sizeA, const glm::vec2& posB, const glm::vec2& sizeB) const;

    // 這些是拆分出來的小區塊
    void HandleItemCollection(Mario* mario, std::vector<std::shared_ptr<Item>>& items);
    void HandleEnemyEnemyCollisions(std::vector<std::shared_ptr<Enemy>>& enemies);
    void HandleBlockItemInteractions(const std::vector<std::shared_ptr<Block>>& blocks, std::vector<std::shared_ptr<Item>>& items);
    void HandleMarioEnemyCollisions(Mario* mario, std::vector<std::shared_ptr<Enemy>>& enemies);
    void HandleEnemyRebound(std::vector<std::shared_ptr<Enemy>>& enemies);
    void HandleFireballEnemyCollisions(std::vector<std::shared_ptr<Fireball>>& fireballs, std::vector<std::shared_ptr<Enemy>>& enemies);
};

#endif // COLLISION_MANAGER_HPP