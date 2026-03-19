#ifndef APP_HPP
#define APP_HPP

#include "pch.hpp" // IWYU pragma: export
#include "Util/Renderer.hpp"
#include "MapManager.hpp"

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

    void End(); // NOLINT(readability-convert-member-functions-to-static)

    void LoadLevel(int level);

private:
    void ValidTask();

private:
    State m_CurrentState = State::START;

    Util::Renderer m_Root; // 負責管理所有畫面上的物件
    MapManager m_MapManager;
    std::vector<std::shared_ptr<Block>> m_CurrentMapBlocks;

    int m_CurrentLevel = 0;
};

#endif