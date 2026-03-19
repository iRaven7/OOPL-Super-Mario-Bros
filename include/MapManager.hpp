#ifndef MAP_MANAGER_HPP
#define MAP_MANAGER_HPP

#include <vector>
#include <string>
#include <memory>
#include "Block.hpp"

class MapManager {
public:
    // 解析指定路徑的文本並回傳方塊陣列
    std::vector<std::shared_ptr<Block>> LoadMap(const std::string& filePath);

private:
    const float BLOCK_SIZE = 16.0f; // 依據 floor.png 的實際像素調整
};

#endif