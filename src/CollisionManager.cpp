// src/CollisionManager.cpp
#include "CollisionManager.hpp"
#include "Mario.hpp"
#include "Block.hpp"
#include "Item.hpp"
#include "Enemy.hpp"
#include "Fireball.hpp"
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

    // 把原本一大串的程式碼變成這樣，一目了然啦！
    HandleItemCollection(mario, items);
    HandleEnemyEnemyCollisions(enemies);
    HandleBlockItemInteractions(blocks, items);
    HandleMarioEnemyCollisions(mario, enemies);
    HandleEnemyRebound(enemies);
    HandleFireballEnemyCollisions(fireballs, enemies);
}

void CollisionManager::HandleItemCollection(Mario* mario, std::vector<std::shared_ptr<Item>>& items) {
    glm::vec2 marioPos = mario->GetPosition();
    glm::vec2 marioSize = mario->GetSize();

    for (auto& item : items) {
        if (!item->IsActive()) continue;

        if (CheckAABB(marioPos, marioSize, item->GetPosition(), item->GetSize())) {
            item->OnCollect(mario);
        }
    }
}

void CollisionManager::HandleEnemyEnemyCollisions(std::vector<std::shared_ptr<Enemy>>& enemies) {
    for (size_t i = 0; i < enemies.size(); ++i) {
        auto& enemyA = enemies[i];
        if (!enemyA->IsActive()) continue;

        for (size_t j = i + 1; j < enemies.size(); ++j) {
            auto& enemyB = enemies[j];
            if (!enemyB->IsActive()) continue;

            if (CheckAABB(enemyA->GetPosition(), enemyA->GetSize(),
                enemyB->GetPosition(), enemyB->GetSize())) {

                Koopa* koopaA = dynamic_cast<Koopa*>(enemyA.get());
                Koopa* koopaB = dynamic_cast<Koopa*>(enemyB.get());

                bool aIsMovingShell = (koopaA && koopaA->GetState() == Koopa::State::ShellMoving);
                bool bIsMovingShell = (koopaB && koopaB->GetState() == Koopa::State::ShellMoving);

                if (aIsMovingShell && !bIsMovingShell) {
                    enemyB->OnFireballHit();
                    GameStateManager::GetInstance().AddScore(100);
                    LOG_INFO("龜殼 (A) 擊殺了敵人！");
                }
                else if (bIsMovingShell && !aIsMovingShell) {
                    enemyA->OnFireballHit();
                    GameStateManager::GetInstance().AddScore(100);
                    LOG_INFO("龜殼 (B) 擊殺了敵人！");
                }
                else if (aIsMovingShell && bIsMovingShell) {
                    enemyA->OnFireballHit();
                    enemyB->OnFireballHit();
                    GameStateManager::GetInstance().AddScore(200);
                    LOG_INFO("兩個龜殼互相撞毀！");
                }
                else {
                    auto velA = enemyA->GetVelocity();
                    auto velB = enemyB->GetVelocity();
                    enemyA->SetVelocity({ -velA.x, velA.y });
                    enemyB->SetVelocity({ -velB.x, velB.y });
                }
            }
        }
    }
}

void CollisionManager::HandleBlockItemInteractions(const std::vector<std::shared_ptr<Block>>& blocks, std::vector<std::shared_ptr<Item>>& items) {
    for (const auto& block : blocks) {
        if (block->PopJustHit()) {
            glm::vec2 blockPos = block->GetCollisionPosition();
            glm::vec2 blockSize = block->GetSize();

            for (auto& item : items) {
                if (!item->IsActive()) continue;

                glm::vec2 itemPos = item->GetPosition();
                glm::vec2 itemSize = item->GetSize();

                bool xOverlap = std::abs(itemPos.x - blockPos.x) < (itemSize.x + blockSize.x) / 2.0f;
                float itemBottom = itemPos.y - (itemSize.y / 2.0f);
                float blockTop = blockPos.y + (blockSize.y / 2.0f);
                bool yOnTop = std::abs(itemBottom - blockTop) < 5.0f;

                if (xOverlap && yOnTop) {
                    item->OnBlockBumped(blockPos.x);
                }
            }
        }
    }
}

void CollisionManager::HandleMarioEnemyCollisions(Mario* mario, std::vector<std::shared_ptr<Enemy>>& enemies) {
    glm::vec2 marioPos = mario->GetPosition();
    glm::vec2 marioSize = mario->GetSize();

    for (auto& enemy : enemies) {
        if (!enemy->IsActive()) continue;

        glm::vec2 enemyPos = enemy->GetPosition();
        glm::vec2 enemySize = enemy->GetSize();

        if (CheckAABB(marioPos, marioSize, enemyPos, enemySize)) {
            float marioBottom = marioPos.y - (marioSize.y / 2.0f);
            float enemyTop = enemyPos.y + (enemySize.y / 2.0f);
            bool canBeStomped = enemy->IsStompable();

            if (mario->GetVelocity().y < 0.0f && marioBottom >= enemyTop - 8.0f && canBeStomped) {
                enemy->OnStomped(mario);
                mario->SetVelocity({ mario->GetVelocity().x, 600.0f });
                mario->SetPosition({ marioPos.x, enemyTop + (marioSize.y / 2.0f) + 1.0f });
            }
            else {
                enemy->OnSideCollision(mario);
            }
        }
    }
}

void CollisionManager::HandleEnemyRebound(std::vector<std::shared_ptr<Enemy>>& enemies) {
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

                if (p1.x < p2.x) {
                    e1->SetVelocity({ -std::abs(v1.x), v1.y });
                    e2->SetVelocity({ std::abs(v2.x), v2.y });
                    e1->SetPosition({ p1.x - 1.0f, p1.y });
                    e2->SetPosition({ p2.x + 1.0f, p2.y });
                }
                else {
                    e1->SetVelocity({ std::abs(v1.x), v1.y });
                    e2->SetVelocity({ -std::abs(v2.x), v2.y });
                    e1->SetPosition({ p1.x + 1.0f, p1.y });
                    e2->SetPosition({ p2.x - 1.0f, p2.y });
                }
            }
        }
    }
}

void CollisionManager::HandleFireballEnemyCollisions(std::vector<std::shared_ptr<Fireball>>& fireballs, std::vector<std::shared_ptr<Enemy>>& enemies) {
    for (auto& fireball : fireballs) {
        if (!fireball->IsActive()) continue;

        for (auto& enemy : enemies) {
            if (!enemy->IsActive()) continue;

            if (CheckAABB(fireball->GetPosition(), fireball->GetSize(),
                enemy->GetPosition(), enemy->GetSize())) {
                enemy->OnFireballHit();
                fireball->SetActive(false);
                break;
            }
        }
    }
}