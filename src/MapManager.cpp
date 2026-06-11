#include "MapManager.hpp"
#include <fstream>
#include "Util/Logger.hpp"
#include "BreakableBlock.hpp"
#include "QuestionBlock.hpp"
#include "Goomba.hpp"
#include "UnbreakableBlock.hpp"
#include "Koopa.hpp"
#include "PiranhaPlant.hpp"
#include "BackgroundProp.hpp"
#include "Flag.hpp"
#include "EnterablePipe.hpp"
#include "Coin.hpp"

void MapManager::LoadMap(const std::string& filePath,
    std::vector<std::shared_ptr<Block>>& outBlocks,
    std::vector<std::shared_ptr<Enemy>>& outEnemies,
    std::vector<std::shared_ptr<Item>>& outItems) {
    std::ifstream file(filePath);

    if (!file.is_open()) {
        LOG_ERROR("�L�k���J�a��: {}", filePath);
        return;
    }

    std::string line;
    int row = 0;

    float startX = -300.0f;
    float startY = 200.0f;

    while (std::getline(file, line)) {
        for (size_t col = 0; col < line.length(); ++col) {
            char tileType = line[col];

            // �J��ťժ������L�A���������o�^�X
            if (tileType == '0') {
                continue;
            }

            // �w���p��n�ӹ϶�������@�ɮy��
            glm::vec2 pos = { startX + col * BLOCK_SIZE, startY - row * BLOCK_SIZE };

            // �g�@�Ӥp�p�����U�禡�A�Τ@�B�z������򥻳]�w
            auto addBlock = [&](const std::shared_ptr<Block>& block) {
                block->SetPosition(pos);
                block->SetZIndex(10);
                outBlocks.push_back(block);
                };

            // ��� switch ���޿�@�ؤF�M
            switch (tileType) {
                // �@�����t�C
            case '1': addBlock(std::make_shared<Block>(RESOURCE_DIR"/Blocks/floor.png")); break;
            case '3': addBlock(std::make_shared<BreakableBlock>(RESOURCE_DIR"/Blocks/breakable_block.png")); break;

                // �ݸ�����t�C
            case 'M': addBlock(std::make_shared<QuestionBlock>(RESOURCE_DIR"/Blocks/question_block.png", QuestionBlock::ItemType::MUSHROOM)); break;
            case 'C': addBlock(std::make_shared<QuestionBlock>(RESOURCE_DIR"/Blocks/question_block.png", QuestionBlock::ItemType::COIN)); break;
            case 'F': addBlock(std::make_shared<QuestionBlock>(RESOURCE_DIR"/Blocks/question_block.png", QuestionBlock::ItemType::FIREFLOWER)); break;
            case 'c': outItems.push_back(std::make_shared<Coin>(pos, true)); break;

                // ���ިt�C
            case 'o': addBlock(std::make_shared<UnbreakableBlock>(RESOURCE_DIR"/Blocks/pipe_tl.png")); break;
            case 'p': addBlock(std::make_shared<UnbreakableBlock>(RESOURCE_DIR"/Blocks/pipe_tr.png")); break;
            case 'k': addBlock(std::make_shared<UnbreakableBlock>(RESOURCE_DIR"/Blocks/pipe_dl.png")); break;
            case 'l': addBlock(std::make_shared<UnbreakableBlock>(RESOURCE_DIR"/Blocks/pipe_dr.png")); break;

            case 'E': addBlock(std::make_shared<EnterablePipe>(RESOURCE_DIR"/Blocks/pipe_tl.png", 2)); break;
            case 'e': addBlock(std::make_shared<EnterablePipe>(RESOURCE_DIR"/Blocks/pipe_tl.png", 0)); break;

                // �ĤH�t�C (�ĤH���ݭn�]�w ZIndex�A������i�}�C)
            case '4': outEnemies.push_back(std::make_shared<Goomba>(pos)); break;
            case '5': outEnemies.push_back(std::make_shared<Koopa>(pos)); break;
            case 'P': outEnemies.push_back(std::make_shared<PiranhaPlant>(pos)); break;

                // �X�m
            case 'X': {
                // 1. ���� 9 �`�I���X��
                for (int i = 0; i < 18; ++i) {
                    auto pole = std::make_shared<BackgroundProp>(RESOURCE_DIR"/Blocks/flag_pole.png");
                    pole->SetPosition({ pos.x, pos.y + i * 16.0f }); // ���W�|
                    outBlocks.push_back(pole);
                }
                // 2. ���ͳ��ݪ���y
                auto top = std::make_shared<BackgroundProp>(RESOURCE_DIR"/Blocks/flag_top.png");
                top->SetPosition({ pos.x, pos.y + 18 * 16.0f });
                outBlocks.push_back(top);

                // 3. ���ͷ|�ʪ��X�m�P����Ĳ�o��
                auto flag = std::make_shared<Flag>(pos);
                outItems.push_back(flag);
                break;
            }

            default: break;
            }
        }
        row++;
    }
}