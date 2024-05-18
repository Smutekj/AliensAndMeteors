#pragma once

#include "SFML/Graphics.hpp"

#include "StateStack.h"
#include "ResourceIdentifiers.h"
#include "ResourceHolder.h"

#include "Commands.h"

#include <sstream>

class ScoreBoard
{

  std::map<int, std::vector<std::string>, std::greater<int>> score2players;
  std::map<std::string, int> player2score;

  int n_shown_scores = 10;

  std::string score_file = "../HighScores.txt";

public:
    int m_current_score = 0;


  ScoreBoard()
  {
    readScoresFromFile(score_file);
  }

  ~ScoreBoard()
  {
    writeScoresToFile(score_file);
  }

  void setScore(std::string player_name, int score)
  {

    if (score2players.count(score) > 0)
    {
      score2players.at(score).push_back(player_name);
    }
    else
    {
      score2players[score] = {player_name};
    }
    if(player2score.count(player_name) == 0)
    {
        player2score[player_name] = score;
    }else{
        /// ???
    }
  }

  int getScore(std::string player_name)const
  {
    return player2score.at(player_name);
  }

  int getCurrentScore()const
  {
    return m_current_score;
  }

    const auto& getAllScores()const
    {
        return score2players;
    }

private:
  void readScoresFromFile(std::string score_file)
  {
    std::ifstream file(score_file);

    std::string line;
    std::string player_name;
    int score;
    while (std::getline(file, line))
    {
      std::stringstream ss(line);
      ss >> player_name >> score;
      score2players[score].push_back(player_name);
    }
    file.close();
  }

  void writeScoresToFile(std::string score_file)
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
};

class Application
{

public:
    Application(float fps = 60);

    void run();

private:
    std::unique_ptr<sf::RenderWindow> m_window;
    std::unique_ptr<StateStack> m_state_stack;

    ResourceHolder<sf::Texture, Textures::ID> m_textures;
    float m_dt;

    KeyBindings m_bindings;
    ScoreBoard m_score;
    sf::Font m_font;

    void registerStates();
};