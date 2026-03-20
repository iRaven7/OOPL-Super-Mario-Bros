#ifndef APP_HPP
#define APP_HPP

#include "pch.hpp" // IWYU pragma: export
#include "Util/Renderer.hpp"
#include "MapManager.hpp"
#include "Mario.hpp"

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

    void UpdateCamera();

private:
    void ValidTask();

private:
    State m_CurrentState = State::START;



    Util::Renderer m_Root; // 負責管理所有畫面上的物件
    MapManager m_MapManager;
    std::vector<std::shared_ptr<Block>> m_CurrentMapBlocks;

    std::shared_ptr<Mario> m_Mario; // 宣告 m_Mario
    float m_CameraX = 0.0f;         // 宣告 m_CameraX

    // --- 解決 'SCREEN_WIDTH' 未定義的問題 ---
    static constexpr float SCREEN_WIDTH = 800.0f;
    static constexpr float CAMERA_DEADZONE_RATIO = 0.5f;

    int m_CurrentLevel = 0;
};

#endif