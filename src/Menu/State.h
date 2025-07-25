#pragma once

#include <IncludesGl.h>

#include <memory>
#include <fstream>
#include <string>

namespace States
{
	enum class ID
	{
		None,
		Exit,
		Menu,
		Game,
		Loading,
		Pause,
		Score,
		Player_Died,
		Shop,
		Settings,
		KeyBindings,
		Graphics,
	};
}

class Renderer;
class Window;

class StateStack;
class Player;
class KeyBindings;
class ScoreBoard;
class TextureHolder;
class Font;
struct PlayerEntity;

class State
{
public:
	struct Context
	{
		Context(Renderer &window, Window& window_handle, TextureHolder& textures, KeyBindings& bindings, Font& font, ScoreBoard& score) 
		: textures(&textures), window(&window),window_handle(&window_handle), bindings(&bindings), font(&font), score(&score) {}

		TextureHolder* textures;
		Renderer *window;
		Window *window_handle;
		KeyBindings* bindings;
		Font* font;
		ScoreBoard* score;
		PlayerEntity* p_player = nullptr;
	};

public:
	State(StateStack &stack, Context context);
	virtual ~State() {}

	virtual void draw() = 0;
	virtual void update(float dt) = 0;
	virtual void handleEvent(const SDL_Event &event) = 0;

protected:
	void requestStackPush(States::ID stateID);
	void requestStackPop();
	void requestStateClear();

protected:
	StateStack *m_stack;
	Context m_context;
};



