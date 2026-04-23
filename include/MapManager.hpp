#ifndef MAP_MANAGER_HPP
#define MAP_MANAGER_HPP

#include <vector>
#include <string>
#include <memory>
#include "Block.hpp"
#include "Enemy.hpp" // 引入敵人介面

class MapManager {
public:
    // 改為透過參考傳遞，使地圖管理器能同時實例化不同類型的實體
    void LoadMap(const std::string& filePath,
        std::vector<std::shared_ptr<Block>>& outBlocks,
        std::vector<std::shared_ptr<Enemy>>& outEnemies);

private:
    const float BLOCK_SIZE = 16.0f;
};

#endif