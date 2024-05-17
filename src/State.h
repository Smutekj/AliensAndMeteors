#pragma once

#include "ResourceHolder.h"
#include "ResourceIdentifiers.h"

#include <SFML/System/Time.hpp>
#include <SFML/Window/Event.hpp>

#include <memory>


namespace States
{
	enum ID
	{
		None,
		Title,
		Menu,
		Game,
		Loading,
		Pause,
		Settings
	};
}

namespace sf
{
	class RenderWindow;
}


class StateStack;
class Player;

class State
{
public:
	struct Context
	{
		Context(sf::RenderWindow &window, TextureHolder& textures) 
		: textures(&textures), window(&window) {}

		TextureHolder* textures;
		sf::RenderWindow *window;
	};

public:
	State(StateStack &stack, Context context);
	virtual ~State() {}

	virtual void draw() = 0;
	virtual void update(float dt) = 0;
	virtual void handleEvent(const sf::Event &event) = 0;
	bool isFinalState() const
	{
		return m_is_final_state;
	}

protected:
	void requestStackPush(States::ID stateID);
	void requestStackPop();
	void requestStateClear();

	Context getContext() const
	{
		return m_context;
	}

protected:
	StateStack *m_stack;
	Context m_context;
	bool m_is_final_state = false; //! if true no state below this one gets updated
};
