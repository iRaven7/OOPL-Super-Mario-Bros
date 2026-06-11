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
            AddLife(1);
        }
    }

    void AddLife(int count = 1) { m_Lives += count; }
    int GetLives() const { return m_Lives; }

    void UpdateTime(float deltaTime) {
        if (m_TimeRemaining > 0.0f) {
            // ��@���Q�ڪ��p�ɾ���u����Ƨ֡A�i�̻ݨD�վ㭿�v (���B�]�� 2.5 ���t)
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
        m_Lives = 3;
        m_TimeRemaining = 400.0f;
        m_LevelComplete = false;
    }


private:
    GameStateManager() = default;
    ~GameStateManager() = default;
    GameStateManager(const GameStateManager&) = delete;
    GameStateManager& operator=(const GameStateManager&) = delete;

    int m_Score = 0;
    int m_Coins = 0;
    int m_Lives = 3;
    float m_TimeRemaining = 400.0f;
    bool m_LevelComplete = false;
};

#endif // GAME_STATE_MANAGER_HPP