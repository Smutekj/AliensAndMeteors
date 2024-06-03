#include "ScoreBoard.h"

#include <fstream>

ScoreBoard::ScoreBoard()
{
  readScoresFromFile(score_file);
}

ScoreBoard::~ScoreBoard()
{
  writeScoresToFile(score_file);
}

void ScoreBoard::setScore(std::string player_name, int score)
{

  if (score2players.count(score) > 0)
  {
    score2players.at(score).push_back(player_name);
  }
  else
  {
    score2players[score] = {player_name};
  }
  if (player2score.count(player_name) == 0)
  {
    player2score[player_name] = score;
  }
  else
  {
    /// ???
  }
}

int ScoreBoard::getScore(std::string player_name) const
{
  return player2score.at(player_name);
}

void ScoreBoard::readScoresFromFile(std::string score_file)
{
  std::ifstream file(score_file);

  std::string line;
  std::string player_name;
  int score;
  int count = 0;
  while (std::getline(file, line))
  {
    std::stringstream ss(line);
    ss >> player_name >> score;
    score2players[score].push_back(player_name);
    if (count++ > m_shown_scores)
    {
      break;
    }
  }
  file.close();
}

void ScoreBoard::writeScoresToFile(std::string score_file)
{
  std::ofstream file(score_file);

  for (auto &[score, players] : score2players)
  {
    for (auto &player : players)
    {
      file << player << " " << score << "\n";
    }
  }
  file.close();
}

void ScoreBoard::setCurrentScore(int score)
{
  m_current_score = score;
}
int ScoreBoard::getCurrentScore() const
{
  return m_current_score;
}

const ScoreBoard::ScoreMap &ScoreBoard::getAllScores() const
{
  return score2players;
}
