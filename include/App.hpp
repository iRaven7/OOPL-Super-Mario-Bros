#ifndef APP_HPP
#define APP_HPP

#include "pch.hpp" // IWYU pragma: export
#include "Util/Renderer.hpp"
#include "MapManager.hpp"
#include "Mario.hpp"
#include "Item.hpp"
#include "CollisionManager.hpp"
#include "Enemy.hpp"

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
    void LoadLevel(int level);

private:
    void UpdateCamera();

    State m_CurrentState = State::START;

    Util::Renderer m_Root;
    MapManager m_MapManager;
    CollisionManager m_CollisionManager; // 新增管理器
    std::vector<std::shared_ptr<Block>> m_CurrentMapBlocks;
    std::vector<std::shared_ptr<Item>> m_Items;
    std::vector<std::shared_ptr<Enemy>> m_Enemies;
    std::shared_ptr<Mario> m_Mario;

    int m_CurrentLevel = 0;
    float m_CameraX = 0.0f;
};

#endif //APP_HPP