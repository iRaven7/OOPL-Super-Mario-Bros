#include "CollisionManager.hpp"
#include "Koopa.hpp"
#include "GameStateManager.hpp"
#include "Util/Logger.hpp"
#include <cmath>

bool CollisionManager::CheckAABB(const glm::vec2& posA, const glm::vec2& sizeA, const glm::vec2& posB, const glm::vec2& sizeB) const {
    return std::abs(posA.x - posB.x) < (sizeA.x + sizeB.x) / 2.0f &&
        std::abs(posA.y - posB.y) < (sizeA.y + sizeB.y) / 2.0f;
}

void CollisionManager::ProcessInteractions(Mario* mario,
    const std::vector<std::shared_ptr<Block>>& blocks,
    std::vector<std::shared_ptr<Item>>& items,
    std::vector<std::shared_ptr<Enemy>>& enemies,
    std::vector<std::shared_ptr<Fireball>>& fireballs) {
    // 1. 處理道具與瑪利歐的收集判定
    glm::vec2 marioPos = mario->GetPosition();
    glm::vec2 marioSize = mario->GetSize();

    for (auto& item : items) {
        if (!item->IsActive()) continue;

        if (CheckAABB(marioPos, marioSize, item->GetPosition(), item->GetSize())) {
            item->OnCollect(mario); // 觸發收集
        }
    }

    // --- 敵人與敵人之間的碰撞解析 ---
    for (size_t i = 0; i < enemies.size(); ++i) {
        auto& enemyA = enemies[i];
        if (!enemyA->IsActive()) continue;

        // 從 i + 1 開始遍歷，避免重複檢查或自我碰撞
        for (size_t j = i + 1; j < enemies.size(); ++j) {
            auto& enemyB = enemies[j];
            if (!enemyB->IsActive()) continue;

            // 檢查 AABB 邊界是否重疊
            if (CheckAABB(enemyA->GetPosition(), enemyA->GetSize(),
                enemyB->GetPosition(), enemyB->GetSize())) {

                // 嘗試將兩者轉型為慢慢龜 (若轉型失敗會回傳 nullptr)
                Koopa* koopaA = dynamic_cast<Koopa*>(enemyA.get());
                Koopa* koopaB = dynamic_cast<Koopa*>(enemyB.get());

                // 判斷是否為「滑行中」的龜殼
                bool aIsMovingShell = (koopaA && koopaA->GetState() == Koopa::State::ShellMoving);
                bool bIsMovingShell = (koopaB && koopaB->GetState() == Koopa::State::ShellMoving);

                if (aIsMovingShell && !bIsMovingShell) {
                    // A 是滑行龜殼，消滅 B
                    enemyB->OnFireballHit(); // 沿用通用的一擊必殺介面
                    GameStateManager::GetInstance().AddScore(100);
                    LOG_INFO("龜殼 (A) 擊殺了敵人！");
                }
                else if (bIsMovingShell && !aIsMovingShell) {
                    // B 是滑行龜殼，消滅 A
                    enemyA->OnFireballHit();
                    GameStateManager::GetInstance().AddScore(100);
                    LOG_INFO("龜殼 (B) 擊殺了敵人！");
                }
                else if (aIsMovingShell && bIsMovingShell) {
                    // 兩個滑行龜殼相撞，雙雙毀滅
                    enemyA->OnFireballHit();
                    enemyB->OnFireballHit();
                    GameStateManager::GetInstance().AddScore(200);
                    LOG_INFO("兩個龜殼互相撞毀！");
                }
                else {
                    // (可選) 兩個普通敵人相撞時的物理防呆：互相反轉 X 軸速度以彈開，防止重疊
                    auto velA = enemyA->GetVelocity();
                    auto velB = enemyB->GetVelocity();
                    enemyA->SetVelocity({ -velA.x, velA.y });
                    enemyB->SetVelocity({ -velB.x, velB.y });
                }
            }
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
    /// 3. 處理瑪利歐與敵人的戰鬥碰撞
    for (auto& enemy : enemies) {
        if (!enemy->IsActive()) continue;

        glm::vec2 enemyPos = enemy->GetPosition();
        glm::vec2 enemySize = enemy->GetSize();

        if (CheckAABB(marioPos, marioSize, enemyPos, enemySize)) {
            float marioBottom = marioPos.y - (marioSize.y / 2.0f);
            float enemyTop = enemyPos.y + (enemySize.y / 2.0f);

            // 移除 dynamic_cast，改用多型介面判斷
            bool canBeStomped = enemy->IsStompable();

            // 嚴格判定：瑪利歐正在下墜、腳底高於緩衝區，且「該敵人允許被踩踏」
            if (mario->GetVelocity().y < 0.0f && marioBottom >= enemyTop - 8.0f && canBeStomped) {
                enemy->OnStomped(mario);
                mario->SetVelocity({ mario->GetVelocity().x, 600.0f });
                mario->SetPosition({ marioPos.x, enemyTop + (marioSize.y / 2.0f) + 1.0f });
            }
            else {
                // 不符合踩踏條件 (包含側面碰撞、由下往上、或遇到不可踩踏的生物)
                // 由於不可踩踏，一律視為瑪利歐受到側面/接觸傷害
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
    // 假設你已在 ProcessInteractions 迴圈內檢查火球與敵人的碰撞：
    for (auto& fireball : fireballs) {
        if (!fireball->IsActive()) continue;

        for (auto& enemy : enemies) {
            if (!enemy->IsActive()) continue;

            // 檢查 AABB 碰撞
            if (CheckAABB(fireball->GetPosition(), fireball->GetSize(),
                enemy->GetPosition(), enemy->GetSize())) {

                // 觸發火球擊殺邏輯
                enemy->OnFireballHit();

                // 火球消失
                fireball->SetActive(false);
                break; // 這顆火球已消耗，跳出內層迴圈
            }
        }
    }
}