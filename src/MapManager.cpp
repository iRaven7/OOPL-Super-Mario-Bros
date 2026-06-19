#include "MapManager.hpp"
#include <fstream>
#include "Util/Logger.hpp"
#include "BreakableBlock.hpp"
#include "QuestionBlock.hpp"
#include "Goomba.hpp"
#include "UnbreakableBlock.hpp"
#include "Koopa.hpp"
#include "FlyingKoopa.hpp"
#include "PiranhaPlant.hpp"
#include "BackgroundProp.hpp"
#include "Flag.hpp"
#include "EnterablePipe.hpp"
#include "Coin.hpp"
#include "OneUp.hpp"
#include "SuperStar.hpp"
#include "MovingPlatform.hpp"
#include "SuperFlower.hpp"

void MapManager::LoadMap(const std::string& filePath,
    std::vector<std::shared_ptr<Block>>& outBlocks,
    std::vector<std::shared_ptr<Enemy>>& outEnemies,
    std::vector<std::shared_ptr<Item>>& outItems,
    const LevelPipeConfig& pipeConfig) {
    std::ifstream file(filePath);

    if (!file.is_open()) {
        LOG_ERROR("Cannot open map: {}", filePath);
        return;
    }

    std::string line;
    int row = 0;

    float startX = -300.0f;
    float startY = 200.0f;

    while (std::getline(file, line)) {
        for (size_t col = 0; col < line.length(); ++col) {
            char tileType = line[col];

            if (tileType == '0') continue;

            glm::vec2 pos = { startX + col * BLOCK_SIZE, startY - row * BLOCK_SIZE };

            auto addBlock = [&](const std::shared_ptr<Block>& block) {
                block->SetPosition(pos);
                block->SetZIndex(60);   // above Mario (50) and all items
                outBlocks.push_back(block);
            };
            auto addPipe = [&](const std::shared_ptr<Block>& block) {
                block->SetPosition(pos);
                block->SetZIndex(65);   // pipes in front of regular blocks
                outBlocks.push_back(block);
            };

            switch (tileType) {
                // --- floor / terrain ---
            case '1': addBlock(std::make_shared<Block>(RESOURCE_DIR"/Blocks/floor.png")); break;
            case '2': addBlock(std::make_shared<Block>(RESOURCE_DIR"/Blocks/building.png")); break;
            case '9': {
                // Castle sits below Mario (ZIndex 50) so he renders in front of
                // it during the end-of-level walk instead of vanishing behind it.
                auto castle = std::make_shared<Block>(RESOURCE_DIR"/Blocks/castle.png");
                castle->SetPosition(pos);
                castle->SetZIndex(45);
                outBlocks.push_back(castle);
                break;
            }
            case '3': addBlock(std::make_shared<BreakableBlock>(RESOURCE_DIR"/Blocks/breakable_block.png")); break;

                // blue variants
            case 'b': addBlock(std::make_shared<Block>(RESOURCE_DIR"/Blocks/floor_blue.png")); break;
            case 'B': addBlock(std::make_shared<BreakableBlock>(RESOURCE_DIR"/Blocks/breakable_block_blue.png")); break;
            case 'n': addBlock(std::make_shared<Block>(RESOURCE_DIR"/Blocks/building_blue.png")); break;

                // 'f' = upward-moving platform (wraps), 'h' = downward-moving (wraps),
                // 'z' = gentle float up-and-down (oscillates ±2 blocks around placed Y).
                // Place one tile at the desired position; the platform sprite is 64px wide.
            case 'f':
            case 'h':
            case 'z': {
                auto dir = (tileType == 'f') ? MovingPlatform::Direction::UP
                         : (tileType == 'h') ? MovingPlatform::Direction::DOWN
                                             : MovingPlatform::Direction::FLOAT;
                auto plat = std::make_shared<MovingPlatform>(dir);
                plat->SetPosition(pos);
                plat->SetZIndex(10);
                outBlocks.push_back(plat);
                break;
            }

                // --- question blocks ---
            case 'M': addBlock(std::make_shared<QuestionBlock>(RESOURCE_DIR"/Blocks/question_block.png", Block::ContentType::MUSHROOM)); break;
            case 'C': addBlock(std::make_shared<QuestionBlock>(RESOURCE_DIR"/Blocks/question_block.png", Block::ContentType::COIN)); break;
            case 'F': addBlock(std::make_shared<QuestionBlock>(RESOURCE_DIR"/Blocks/question_block.png", Block::ContentType::FIREFLOWER)); break;
            case 'U': addBlock(std::make_shared<QuestionBlock>(RESOURCE_DIR"/Blocks/breakable_block.png", Block::ContentType::ONEUP)); break;

                // --- breakable-looking (hidden) item blocks ---
                // Same QuestionBlock item logic (bump from below -> item emerges,
                // can't be smashed), just re-skinned to the brick sprite so they
                // blend in with ordinary breakable blocks. Each of the three item
                // types has a normal and a blue-palette version:
                //     normal brick (breakable_block.png):       '*' STAR, '%' COIN, '@' MUSHROOM
                //     blue   brick (breakable_block_blue.png):  '$' STAR, '7' COIN, '8' MUSHROOM
            case '*': addBlock(std::make_shared<QuestionBlock>(RESOURCE_DIR"/Blocks/breakable_block.png", Block::ContentType::STAR)); break;
            case '%': addBlock(std::make_shared<QuestionBlock>(RESOURCE_DIR"/Blocks/breakable_block.png", Block::ContentType::COIN)); break;
            case '@': addBlock(std::make_shared<QuestionBlock>(RESOURCE_DIR"/Blocks/breakable_block.png", Block::ContentType::MUSHROOM)); break;
            case '$': addBlock(std::make_shared<QuestionBlock>(RESOURCE_DIR"/Blocks/breakable_block_blue.png", Block::ContentType::STAR)); break;
            case '7': addBlock(std::make_shared<QuestionBlock>(RESOURCE_DIR"/Blocks/breakable_block_blue.png", Block::ContentType::COIN)); break;
            case '8': addBlock(std::make_shared<QuestionBlock>(RESOURCE_DIR"/Blocks/breakable_block_blue.png", Block::ContentType::MUSHROOM)); break;
            case 'c': outItems.push_back(std::make_shared<Coin>(pos, true)); break;

                // --- pipe tiles (visual only) ---
                // vertical pipe
            case 'o': addPipe(std::make_shared<UnbreakableBlock>(RESOURCE_DIR"/Blocks/pipe_tl.png")); break;
            case 'p': addPipe(std::make_shared<UnbreakableBlock>(RESOURCE_DIR"/Blocks/pipe_tr.png")); break;
            case 'k': addPipe(std::make_shared<UnbreakableBlock>(RESOURCE_DIR"/Blocks/pipe_dl.png")); break;
            case 'l': addPipe(std::make_shared<UnbreakableBlock>(RESOURCE_DIR"/Blocks/pipe_dr.png")); break;
                // side (horizontal) pipe — place mouth on the right, body extending left:
                //   body row:  i i i ... (pipe_l_rotate / pipe_r_rotate for top & bottom rows)
                //   mouth col: q          (pipe_dl_rotate — top of opening)
                //              v          (pipe_dr_rotate — bottom of opening)
            case 'i': addPipe(std::make_shared<UnbreakableBlock>(RESOURCE_DIR"/Blocks/pipe_l_rotate.png")); break;
            case 'j': addPipe(std::make_shared<UnbreakableBlock>(RESOURCE_DIR"/Blocks/pipe_r_rotate.png")); break;
            case 'q': addPipe(std::make_shared<UnbreakableBlock>(RESOURCE_DIR"/Blocks/pipe_dl_rotate.png")); break;
            case 'v': addPipe(std::make_shared<UnbreakableBlock>(RESOURCE_DIR"/Blocks/pipe_dr_rotate.png")); break;

                // --- warp pipes ---
                // 'W' : enter pipe — destination and spawn come from LevelPipeConfig::subMapLevel/subMapSpawnX
                // 'w' : exit  pipe — destination and spawn come from LevelPipeConfig::parentLevel/returnSpawnX
                // 'E'/'e' kept for legacy test maps; hard-coded targets 2 and 0
            case 'W':
                if (pipeConfig.subMapLevel >= 0)
                    addPipe(std::make_shared<EnterablePipe>(RESOURCE_DIR"/Blocks/pipe_tl.png",
                        pipeConfig.subMapLevel, pipeConfig.subMapSpawnX));
                break;
            case 'w':
                if (pipeConfig.parentLevel >= 0)
                    addPipe(std::make_shared<EnterablePipe>(RESOURCE_DIR"/Blocks/pipe_tl.png",
                        pipeConfig.parentLevel, pipeConfig.returnSpawnX));
                break;
            case 'E': addPipe(std::make_shared<EnterablePipe>(RESOURCE_DIR"/Blocks/pipe_tl.png", 2)); break;
            case 'e': addPipe(std::make_shared<EnterablePipe>(RESOURCE_DIR"/Blocks/pipe_tl.png", 0)); break;

                // Horizontal warp pipes — direction-named tile characters:
                //   R / r  : right-facing mouth  (Mario stands left, presses RIGHT to enter)
                //             R → sub-map (subMapLevel),  r → parent (parentLevel)
                //   L      : left-facing mouth   (Mario stands right, presses LEFT to enter)
                //             L → sub-map (subMapLevel)
                //   '>' / '<' kept as aliases for R / L (backwards-compat, same behaviour)
                // Sprites:
                //   pipe_dl_rotate.png = top tile of a right-facing opening (mouth on left edge)
                //   pipe_dr_rotate.png = bottom tile of a right-facing opening
                //   Left-facing variants use the same sprites with flipX=true
            case 'R':
            case '>':
                if (pipeConfig.subMapLevel >= 0)
                    addPipe(std::make_shared<EnterablePipe>(RESOURCE_DIR"/Blocks/pipe_dl_rotate.png",
                        pipeConfig.subMapLevel, pipeConfig.subMapSpawnX,
                        Block::PipeEntryDir::Right));
                break;
            case 'r':
                if (pipeConfig.parentLevel >= 0)
                    addPipe(std::make_shared<EnterablePipe>(RESOURCE_DIR"/Blocks/pipe_dl_rotate.png",
                        pipeConfig.parentLevel, pipeConfig.returnSpawnX,
                        Block::PipeEntryDir::Right));
                break;
            case 'L':
                if (pipeConfig.subMapLevel >= 0)
                    addPipe(std::make_shared<EnterablePipe>(RESOURCE_DIR"/Blocks/pipe_dl_rotate.png",
                        pipeConfig.subMapLevel, pipeConfig.subMapSpawnX,
                        Block::PipeEntryDir::Left, true));
                break;
            case '<':
                if (pipeConfig.parentLevel >= 0)
                    addPipe(std::make_shared<EnterablePipe>(RESOURCE_DIR"/Blocks/pipe_dl_rotate.png",
                        pipeConfig.parentLevel, pipeConfig.returnSpawnX,
                        Block::PipeEntryDir::Left, true));
                break;

                // --- enemies ---
            case '4': outEnemies.push_back(std::make_shared<Goomba>(pos)); break;
            case '5': outEnemies.push_back(std::make_shared<Koopa>(pos)); break;
            case '6': outEnemies.push_back(std::make_shared<FlyingKoopa>(pos)); break;
            case 'P': outEnemies.push_back(std::make_shared<PiranhaPlant>(pos)); break;

                // --- flag pole ---
            case 'X': {
                for (int i = 0; i < 9; ++i) {
                    auto pole = std::make_shared<BackgroundProp>(RESOURCE_DIR"/Blocks/flag_pole.png");
                    pole->SetPosition({ pos.x, pos.y + i * 16.0f });
                    outBlocks.push_back(pole);
                }
                auto top = std::make_shared<BackgroundProp>(RESOURCE_DIR"/Blocks/flag_top.png");
                top->SetPosition({ pos.x, pos.y + 9 * 16.0f });
                outBlocks.push_back(top);
                auto flag = std::make_shared<Flag>(pos, pipeConfig.flagStopX);
                outItems.push_back(flag);
                break;
            }

                // --- castle decorations (non-collidable) ---
                // castle_door: rendered in front of Mario (ZIndex 60) so Mario appears to walk inside.
            case 'd': {
                auto door = std::make_shared<BackgroundProp>(RESOURCE_DIR"/Blocks/castle_door.png");
                door->SetPosition(pos);
                door->SetZIndex(60);
                outBlocks.push_back(door);
                break;
            }
                // castle_flag: decorative flag on castle rooftop.
            case 'g': {
                auto cflag = std::make_shared<BackgroundProp>(RESOURCE_DIR"/Blocks/castle_flag.png");
                cflag->SetPosition(pos);
                cflag->SetZIndex(20);
                outBlocks.push_back(cflag);
                break;
            }

                // --- SuperFlower question block ---
            case 'T': addBlock(std::make_shared<QuestionBlock>(RESOURCE_DIR"/Blocks/question_block.png", Block::ContentType::SUPERFLOWER)); break;

                // --- background decorations (no collision) ---
                // Hills/slides
            case 's': {
                auto prop = std::make_shared<BackgroundProp>(RESOURCE_DIR"/Blocks/slide.png");
                prop->SetPosition(pos);
                outBlocks.push_back(prop);
                break;
            }
            case 'S': {
                auto prop = std::make_shared<BackgroundProp>(RESOURCE_DIR"/Blocks/big_slide.png");
                prop->SetPosition(pos);
                outBlocks.push_back(prop);
                break;
            }
                // Bush leaves (top row) — collidable so Mario can stand on them
            case '[': addBlock(std::make_shared<Block>(RESOURCE_DIR"/Blocks/leaf_left.png")); break;
            case '-': addBlock(std::make_shared<Block>(RESOURCE_DIR"/Blocks/leaf_middle.png")); break;
            case ']': addBlock(std::make_shared<Block>(RESOURCE_DIR"/Blocks/leaf_right.png")); break;
                // Bush logs (bottom row)
            case '{': {
                auto prop = std::make_shared<BackgroundProp>(RESOURCE_DIR"/Blocks/log_left.png");
                prop->SetPosition(pos);
                outBlocks.push_back(prop);
                break;
            }
            case '~': {
                auto prop = std::make_shared<BackgroundProp>(RESOURCE_DIR"/Blocks/log_middle.png");
                prop->SetPosition(pos);
                outBlocks.push_back(prop);
                break;
            }
            case '}': {
                auto prop = std::make_shared<BackgroundProp>(RESOURCE_DIR"/Blocks/log_right.png");
                prop->SetPosition(pos);
                outBlocks.push_back(prop);
                break;
            }

            default: break;
            }
        }
        row++;
    }
}
