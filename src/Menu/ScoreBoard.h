#pragma once

#include <vector>
#include <map>
#include <sstream>

class ScoreBoard
{
    using ScoreMap = std::map<int, std::vector<std::string>, std::greater<int>> ;

public:
    ScoreBoard();
    ~ScoreBoard();
    void setScore(std::string player_name, int score);
    int getScore(std::string player_name) const;
    int getCurrentScore() const;
    void setCurrentScore(int score);
    const ScoreMap &getAllScores() const;

private:
    void readScoresFromFile(std::string score_file);
    void writeScoresToFile(std::string score_file);

private:

    int m_current_score;
    std::map<int, std::vector<std::string>, std::greater<int>> score2players;
    std::map<std::string, int> player2score;
    int m_shown_scores = 10;
    const std::string score_file = "../HighScores.txt";
};
