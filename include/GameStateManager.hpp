#ifndef GAME_STATE_MANAGER_HPP
#define GAME_STATE_MANAGER_HPP

#include "SFXManager.hpp"

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

    void AddLife(int count = 1) {
        m_Lives += count;
        // Any life gained (1-Up item, 100-coin rollover, combo overflow) cues
        // the 1-Up jingle; losing a life on death does not.
        if (count > 0) SFXManager::GetInstance().Play(SFXManager::Sound::OneUp);
    }
    int GetLives() const { return m_Lives; }

    // --- Stomp combo: consecutive mid-air stomps escalate; the combo resets the
    // moment Mario lands. Each stomp awards the next value in the table; once the
    // table is exhausted every further stomp grants a 1-Up instead.
    int RegisterStompCombo() {
        static const int kTable[] = { 100, 200, 400, 500, 800, 1000, 2000, 4000, 5000, 8000 };
        const int kCount = static_cast<int>(sizeof(kTable) / sizeof(kTable[0]));
        int idx = m_StompCombo++;
        if (idx < kCount) {
            AddScore(kTable[idx]);
            return kTable[idx];
        }
        AddLife(1);
        return 0;   // 0 => a 1-Up was granted
    }
    void ResetStompCombo() { m_StompCombo = 0; }

    // Time bonus at level clear: 50 pts per whole second left on the clock.
    void ApplyTimeBonus() { AddScore(GetTimeRemaining() * 50); }

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

    // Restart the clock for a fresh attempt at a level. Unlike Reset() this leaves
    // score/coins/lives intact — used on level transitions and on death-respawn,
    // both of which start the player over on a full timer.
    void ResetTime() { m_TimeRemaining = 400.0f; }


    void Reset() {
        m_Score = 0;
        m_Coins = 0;
        m_Lives = 3;
        m_TimeRemaining = 400.0f;
        m_LevelComplete = false;
        m_StompCombo = 0;
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
    int m_StompCombo = 0;
};

#endif // GAME_STATE_MANAGER_HPP