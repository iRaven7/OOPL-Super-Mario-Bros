#include "CollisionManager.hpp"
#include <cmath>

bool CollisionManager::CheckAABB(const glm::vec2& posA, const glm::vec2& sizeA, const glm::vec2& posB, const glm::vec2& sizeB) const {
    return std::abs(posA.x - posB.x) < (sizeA.x + sizeB.x) / 2.0f &&
        std::abs(posA.y - posB.y) < (sizeA.y + sizeB.y) / 2.0f;
}

void CollisionManager::ProcessInteractions(Mario* mario, const std::vector<std::shared_ptr<Block>>& blocks, std::vector<std::shared_ptr<Item>>& items) {
    // 1. 處理道具與瑪利歐的收集判定
    glm::vec2 marioPos = mario->GetPosition();
    glm::vec2 marioSize = mario->GetSize();

    for (auto& item : items) {
        if (!item->IsActive()) continue;

        if (CheckAABB(marioPos, marioSize, item->GetPosition(), item->GetSize())) {
            item->OnCollect(mario); // 觸發收集
        }
    }

    // 2. 處理方塊受擊與上方道具的連動彈跳 (空間查詢)
    for (const auto& block : blocks) {
        if (block->PopJustHit()) { // 讀取並重置受擊標記
            glm::vec2 blockPos = block->GetCollisionPosition();
            glm::vec2 blockSize = block->GetSize();

            for (auto& item : items) {
                if (!item->IsActive()) continue;

                glm::vec2 itemPos = item->GetPosition();
                glm::vec2 itemSize = item->GetSize();

                // 檢查 X 軸重疊
                bool xOverlap = std::abs(itemPos.x - blockPos.x) < (itemSize.x + blockSize.x) / 2.0f;

                // 檢查 Y 軸：道具底部是否貼近方塊頂部 (5 像素容錯)
                float itemBottom = itemPos.y - (itemSize.y / 2.0f);
                float blockTop = blockPos.y + (blockSize.y / 2.0f);
                bool yOnTop = std::abs(itemBottom - blockTop) < 5.0f;

                if (xOverlap && yOnTop) {
                    item->OnBlockBumped(blockPos.x); // 觸發彈跳
                }
            }
        }
    }
}