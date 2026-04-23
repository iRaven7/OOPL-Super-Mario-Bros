#include "CollisionManager.hpp"
#include <cmath>

bool CollisionManager::CheckAABB(const glm::vec2& posA, const glm::vec2& sizeA, const glm::vec2& posB, const glm::vec2& sizeB) const {
    return std::abs(posA.x - posB.x) < (sizeA.x + sizeB.x) / 2.0f &&
        std::abs(posA.y - posB.y) < (sizeA.y + sizeB.y) / 2.0f;
}

void CollisionManager::ProcessInteractions(Mario* mario,
    const std::vector<std::shared_ptr<Block>>& blocks,
    std::vector<std::shared_ptr<Item>>& items,
    std::vector<std::shared_ptr<Enemy>>& enemies) {
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

    /// 3. 處理瑪利歐與敵人的戰鬥碰撞
    for (auto& enemy : enemies) {
        if (!enemy->IsActive()) continue;

        glm::vec2 enemyPos = enemy->GetPosition();
        glm::vec2 enemySize = enemy->GetSize();

        if (CheckAABB(marioPos, marioSize, enemyPos, enemySize)) {
            // 強化判定：計算瑪利歐底部與敵人頂部的相對位置
            float marioBottom = marioPos.y - (marioSize.y / 2.0f);
            float enemyTop = enemyPos.y + (enemySize.y / 2.0f);

            // 條件：瑪利歐正在下墜 (vy < 0) 且 腳底高於敵人中心點以上
            if (mario->GetVelocity().y < 0.0f && marioBottom > enemyPos.y) {
                enemy->OnStomped(mario);
                // 賦予反彈力，並將位置稍微移高防止連續碰撞
                mario->SetVelocity({ mario->GetVelocity().x, 600.0f });
                mario->SetPosition({ mario->GetPosition().x, enemyTop + (marioSize.y / 2.0f) + 1.0f });
            }
            else {
                // 側面或由下往上撞到，才觸發受傷
                enemy->OnSideCollision(mario);
            }
        }
    }

    // 4. 處理敵人與敵人之間的碰撞反彈
    for (size_t i = 0; i < enemies.size(); ++i) {
        for (size_t j = i + 1; j < enemies.size(); ++j) {
            auto& e1 = enemies[i];
            auto& e2 = enemies[j];

            if (!e1->IsActive() || !e2->IsActive()) continue;

            glm::vec2 p1 = e1->GetPosition();
            glm::vec2 p2 = e2->GetPosition();

            if (CheckAABB(p1, e1->GetSize(), p2, e2->GetSize())) {
                glm::vec2 v1 = e1->GetVelocity();
                glm::vec2 v2 = e2->GetVelocity();

                // 依據相對位置決定反彈方向，確保兩者分離
                if (p1.x < p2.x) {
                    // e1 在左側，強迫向左走；e2 在右側，強迫向右走
                    e1->SetVelocity({ -std::abs(v1.x), v1.y });
                    e2->SetVelocity({ std::abs(v2.x), v2.y });

                    // 位置微調，強制推開 1.0f 像素，防止下個 Frame 繼續重疊判定
                    e1->SetPosition({ p1.x - 1.0f, p1.y });
                    e2->SetPosition({ p2.x + 1.0f, p2.y });
                }
                else {
                    // e1 在右側，e2 在左側
                    e1->SetVelocity({ std::abs(v1.x), v1.y });
                    e2->SetVelocity({ -std::abs(v2.x), v2.y });

                    e1->SetPosition({ p1.x + 1.0f, p1.y });
                    e2->SetPosition({ p2.x - 1.0f, p2.y });
                }
            }
        }
    }
}