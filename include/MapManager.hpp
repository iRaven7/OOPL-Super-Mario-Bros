#ifndef MAP_MANAGER_HPP
#define MAP_MANAGER_HPP

#include <vector>
#include <string>
#include <memory>
#include "Block.hpp"
#include "Enemy.hpp"
#include "Item.hpp"

// Pipe warp relationship for a single level.
// Set subMapLevel >= 0 if the level has a 'W' (enter) pipe.
// Set parentLevel >= 0 if the level has a 'w' (exit) pipe.
struct LevelPipeConfig {
    int   subMapLevel  = -1;       // level loaded when entering the 'W' pipe
    float subMapSpawnX = -300.0f;  // Mario spawn X inside that sub-map
    int   parentLevel  = -1;       // level returned to when exiting the 'w' pipe
    float returnSpawnX = -300.0f;  // Mario spawn X in the parent level on exit
    // world-X where the end-of-level pole walk stops and Mario hides into the
    // castle (defaults to level 1's column 219).
    float flagStopX    = 3188.0f;
};

class MapManager {
public:
    void LoadMap(const std::string& filePath,
        std::vector<std::shared_ptr<Block>>& outBlocks,
        std::vector<std::shared_ptr<Enemy>>& outEnemies,
        std::vector<std::shared_ptr<Item>>& outItems,
        const LevelPipeConfig& pipeConfig = {});

private:
    const float BLOCK_SIZE = 16.0f;
};

#endif
