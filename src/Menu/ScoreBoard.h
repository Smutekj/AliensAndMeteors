#pragma once

#include <vector>
#include <map>
#include <sstream>

class ScoreBoard
{

    std::map<int, std::vector<std::string>, std::greater<int>> score2players;
    std::map<std::string, int> player2score;

    int n_shown_scores = 10;

    const std::string score_file = "../HighScores.txt";

public:
    int m_current_score = 0;

    ScoreBoard();
    ~ScoreBoard();

    void setScore(std::string player_name, int score);

    int getScore(std::string player_name) const;

    int getCurrentScore() const;

    const auto &getAllScores() const
    {
        return score2players;
    }

private:
    void readScoresFromFile(std::string score_file);

    void writeScoresToFile(std::string score_file);
};
