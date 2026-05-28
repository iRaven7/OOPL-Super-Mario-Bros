#include "MapManager.hpp"
#include <fstream>
#include "Util/Logger.hpp"
#include "BreakableBlock.hpp"
#include "QuestionBlock.hpp"
#include "Goomba.hpp"
#include "UnbreakableBlock.hpp"
#include "Koopa.hpp"
#include "PiranhaPlant.hpp"

void MapManager::LoadMap(const std::string& filePath,
    std::vector<std::shared_ptr<Block>>& outBlocks,
    std::vector<std::shared_ptr<Enemy>>& outEnemies) {
    std::ifstream file(filePath);

    if (!file.is_open()) {
        LOG_ERROR("無法載入地圖: {}", filePath);
        return;
    }

    std::string line;
    int row = 0;

    float startX = -300.0f;
    float startY = 200.0f;

    while (std::getline(file, line)) {
        for (size_t col = 0; col < line.length(); ++col) {
            char tileType = line[col];

            // 遇到空白直接跳過，提早結束這回合
            if (tileType == '0') {
                continue;
            }

            // 預先計算好該圖塊的絕對世界座標
            glm::vec2 pos = { startX + col * BLOCK_SIZE, startY - row * BLOCK_SIZE };

            // 寫一個小小的輔助函式，統一處理方塊的基本設定
            auto addBlock = [&](const std::shared_ptr<Block>& block) {
                block->SetPosition(pos);
                block->SetZIndex(10);
                outBlocks.push_back(block);
                };

            // 改用 switch 讓邏輯一目了然
            switch (tileType) {
                // 一般方塊系列
            case '1': addBlock(std::make_shared<Block>(RESOURCE_DIR"/Blocks/floor.png")); break;
            case '3': addBlock(std::make_shared<BreakableBlock>(RESOURCE_DIR"/Blocks/breakable_block.png")); break;

                // 問號方塊系列
            case 'M': addBlock(std::make_shared<QuestionBlock>(RESOURCE_DIR"/Blocks/question_block.png", QuestionBlock::ItemType::MUSHROOM)); break;
            case 'C': addBlock(std::make_shared<QuestionBlock>(RESOURCE_DIR"/Blocks/question_block.png", QuestionBlock::ItemType::COIN)); break;
            case 'F': addBlock(std::make_shared<QuestionBlock>(RESOURCE_DIR"/Blocks/question_block.png", QuestionBlock::ItemType::FIREFLOWER)); break;

                // 水管系列
            case 'o': addBlock(std::make_shared<UnbreakableBlock>(RESOURCE_DIR"/Blocks/pipe_tl.png")); break;
            case 'p': addBlock(std::make_shared<UnbreakableBlock>(RESOURCE_DIR"/Blocks/pipe_tr.png")); break;
            case 'k': addBlock(std::make_shared<UnbreakableBlock>(RESOURCE_DIR"/Blocks/pipe_dl.png")); break;
            case 'l': addBlock(std::make_shared<UnbreakableBlock>(RESOURCE_DIR"/Blocks/pipe_dr.png")); break;

                // 敵人系列 (敵人不需要設定 ZIndex，直接丟進陣列)
            case '4': outEnemies.push_back(std::make_shared<Goomba>(pos)); break;
            case '5': outEnemies.push_back(std::make_shared<Koopa>(pos)); break;
            case 'P': outEnemies.push_back(std::make_shared<PiranhaPlant>(pos)); break;

            default: break;
            }
        }
        row++;
    }
}