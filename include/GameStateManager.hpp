#ifndef GAME_STATE_MANAGER_HPP
#define GAME_STATE_MANAGER_HPP

class GameStateManager {
public:
    static GameStateManager& GetInstance() {
        static GameStateManager instance;
        return instance;
    }

    void AddScore(int score) { m_Score += score; }

    void AddCoin(int coin = 1) {
        m_Coins += coin;
        if (m_Coins >= 100) {
            m_Coins -= 100;
            // 未來可在此處擴充 1-UP (加命) 邏輯
        }
    }

    void UpdateTime(float deltaTime) {
        if (m_TimeRemaining > 0.0f) {
            // 原作瑪利歐的計時器比真實秒數快，可依需求調整倍率 (此處設為 2.5 倍速)
            m_TimeRemaining -= deltaTime * 2.5f;
            if (m_TimeRemaining < 0.0f) m_TimeRemaining = 0.0f;
        }
    }

    int GetScore() const { return m_Score; }
    int GetCoins() const { return m_Coins; }
    int GetTimeRemaining() const { return static_cast<int>(m_TimeRemaining); }

    void SetLevelComplete(bool complete) { m_LevelComplete = complete; }
    bool IsLevelComplete() const { return m_LevelComplete; }


    void Reset() {
        m_Score = 0;
        m_Coins = 0;
        m_TimeRemaining = 400.0f; // 預設關卡時間
        m_LevelComplete = false;
    }


private:
    GameStateManager() = default;
    ~GameStateManager() = default;
    GameStateManager(const GameStateManager&) = delete;
    GameStateManager& operator=(const GameStateManager&) = delete;

    int m_Score = 0;
    int m_Coins = 0;
    float m_TimeRemaining = 400.0f;
    bool m_LevelComplete = false;
};

#endif // GAME_STATE_MANAGER_HPP