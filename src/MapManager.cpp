#include "MapManager.hpp"
#include <fstream>
#include "Util/Logger.hpp"
#include "BreakableBlock.hpp"

std::vector<std::shared_ptr<Block>> MapManager::LoadMap(const std::string& filePath) {
    std::vector<std::shared_ptr<Block>> blocks;
    std::ifstream file(filePath);

    if (!file.is_open()) {
        LOG_ERROR("無法載入地圖: {}", filePath);
        return blocks;
    }

    std::string line;
    int row = 0;

    // 假設螢幕中心為原點，可依據實際視窗大小與遊戲機制調整起始座標
    float startX = -300.0f;
    float startY = 200.0f;

    while (std::getline(file, line)) {
        for (size_t col = 0; col < line.length(); ++col) {
            char tileType = line[col];

            // 基礎判斷邏輯，1 對應地板
            if (tileType == '1') {
                auto block = std::make_shared<Block>(RESOURCE_DIR"/Blocks/floor.png");
                block->SetPosition({ startX + col * BLOCK_SIZE, startY - row * BLOCK_SIZE });
                block->SetZIndex(10);
                blocks.push_back(block);
            }
            else if (tileType == '3') {
                auto block = std::make_shared<BreakableBlock>(RESOURCE_DIR"/Blocks/breakable_block.png");
                block->SetPosition({ startX + col * BLOCK_SIZE, startY - row * BLOCK_SIZE });
                block->SetZIndex(10);
                blocks.push_back(block); // 向上轉型為 shared_ptr<Block> 儲存
            }
            // 未來可擴充其他代碼，例如 '2' 為水管，'3' 為問號方塊
        }
        row++;
    }

    return blocks;
}