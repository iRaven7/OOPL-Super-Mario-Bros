#ifndef APP_HPP
#define APP_HPP

#include "Util/Color.hpp"
#include "Util/GameObject.hpp"
#include "pch.hpp" // IWYU pragma: export
#include "Util/Renderer.hpp"
#include "Util/Text.hpp"
#include "MapManager.hpp"
#include "Mario.hpp"
#include "Item.hpp"
#include "CollisionManager.hpp"
#include "Enemy.hpp"
#include "Fireball.hpp"
#include "Background.hpp"

class App {
public:
    enum class State {
        START,
        UPDATE,
        END,
    };

    State GetCurrentState() const { return m_CurrentState; }
    void Start();
    void Update();
    void End();
    void LoadLevel(int level, float spawnX = -300.0f);

    float GetCameraZoom() const { return m_CameraZoom; }
    void SetCameraZoom(float zoom) { m_CameraZoom = std::max(0.1f, zoom); }

private:
    void UpdateCamera();

    // ��~����X�Ӫ��p�禡�ŧi
    void UpdateUI();
    void TriggerLevelTransition();       // Ĳ�o�L�����禡
    void TriggerDeath();
    bool m_IsTransitioning = false;
    bool m_IsDeadTransition = false;
    float m_LevelTransitionTimer = 0.0f;

    bool m_IsGameOver = false;
    float m_GameOverTimer = 0.0f;

    template <typename T>
    void CleanupInactiveEntities(std::vector<std::shared_ptr<T>>& entities);

    float m_CameraZoom = 1.0f;

    State m_CurrentState = State::START;

    Util::Renderer m_Root;
    MapManager m_MapManager;
    CollisionManager m_CollisionManager;
    std::vector<std::shared_ptr<Block>> m_CurrentMapBlocks;
    std::vector<std::shared_ptr<Item>> m_Items;
    std::vector<std::shared_ptr<Enemy>> m_Enemies;
    std::shared_ptr<Mario> m_Mario;
    std::vector<std::shared_ptr<Fireball>> m_Fireballs;

    std::shared_ptr<Background> m_Background;
    // UI ����ŧi
    std::shared_ptr<Util::GameObject> m_ScoreUI;
    std::shared_ptr<Util::Text> m_ScoreText;
    std::shared_ptr<Util::GameObject> m_CoinUI;
    std::shared_ptr<Util::Text> m_CoinText;
    std::shared_ptr<Util::GameObject> m_LivesUI;
    std::shared_ptr<Util::Text> m_LivesText;
    std::shared_ptr<Util::GameObject> m_TimeUI;
    std::shared_ptr<Util::Text> m_TimeText;
    std::shared_ptr<Util::GameObject> m_GameOverUI;
    std::shared_ptr<Util::Text> m_GameOverText;

    int m_CurrentLevel = 0;
    float m_CameraX = 0.0f;
};

#endif //APP_HPP