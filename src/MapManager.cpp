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
            // 預先計算好該圖塊的絕對世界座標
            glm::vec2 pos = { startX + col * BLOCK_SIZE, startY - row * BLOCK_SIZE };

            if (tileType == '0') {
                continue;
            }
            else if (tileType == '1') {
                auto block = std::make_shared<Block>(RESOURCE_DIR"/Blocks/floor.png");
                block->SetPosition(pos);
                block->SetZIndex(10);
                outBlocks.push_back(block);
            }
            else if (tileType == 'M') { // 蘑菇方塊
                auto block = std::make_shared<QuestionBlock>(RESOURCE_DIR"/Blocks/question_block.png", QuestionBlock::ItemType::MUSHROOM);
                block->SetPosition(pos); block->SetZIndex(10); outBlocks.push_back(block);
            }
            // --- 新增：金幣與火焰花磚塊 ---
            else if (tileType == 'C') { // 金幣方塊
                auto block = std::make_shared<QuestionBlock>(RESOURCE_DIR"/Blocks/question_block.png", QuestionBlock::ItemType::COIN);
                block->SetPosition(pos); block->SetZIndex(10); outBlocks.push_back(block);
            }
            else if (tileType == 'F') { // 火焰花方塊
                auto block = std::make_shared<QuestionBlock>(RESOURCE_DIR"/Blocks/question_block.png", QuestionBlock::ItemType::FIREFLOWER);
                block->SetPosition(pos); block->SetZIndex(10); outBlocks.push_back(block);
            }
            else if (tileType == '3') {
                auto block = std::make_shared<BreakableBlock>(RESOURCE_DIR"/Blocks/breakable_block.png");
                block->SetPosition(pos);
                block->SetZIndex(10);
                outBlocks.push_back(block);
            }
            else if (tileType == '4') {
                // 生成栗寶寶 (Goomba)
                auto enemy = std::make_shared<Goomba>(pos);
                outEnemies.push_back(enemy);
            }
            else if (tileType == '5') {
                // 生成 Koopa
                auto enemy = std::make_shared<Koopa>(pos);
                outEnemies.push_back(enemy);
            }
            else if (tileType == 'P') {
                // 生成 Koopa
                auto enemy = std::make_shared<PiranhaPlant>(pos);
                outEnemies.push_back(enemy);
            }
            else if (tileType == 'o') {
                auto block = std::make_shared<UnbreakableBlock>(RESOURCE_DIR"/Blocks/pipe_tl.png");
                block->SetPosition(pos);
                block->SetZIndex(10);
                outBlocks.push_back(block);
            }
            else if (tileType == 'p') {
                auto block = std::make_shared<UnbreakableBlock>(RESOURCE_DIR"/Blocks/pipe_tr.png");
                block->SetPosition(pos);
                block->SetZIndex(10);
                outBlocks.push_back(block);
            }
            else if (tileType == 'k') {
                auto block = std::make_shared<UnbreakableBlock>(RESOURCE_DIR"/Blocks/pipe_dl.png");
                block->SetPosition(pos);
                block->SetZIndex(10);
                outBlocks.push_back(block);
            }
            else if (tileType == 'l') {
                auto block = std::make_shared<UnbreakableBlock>(RESOURCE_DIR"/Blocks/pipe_dr.png");
                block->SetPosition(pos);
                block->SetZIndex(10);
                outBlocks.push_back(block);
            }
        }
        row++;
    }
}