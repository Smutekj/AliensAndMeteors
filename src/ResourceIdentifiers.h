#pragma once

#include "ResourceHolder.h"

// Forward declaration of SFML classes
namespace sf
{
	class Texture;
	class Font;
	class Shader;
}

namespace Textures
{
	enum class ID
	{
		Explosion,
		Explosion2,
		Bomb,
		PlayerShip,
		EnemyShip,
		BossShip,
		EnemyBomber,
		Heart,
		Station,
		BoosterPurple,
		BoosterYellow,
		BackGround,
	};
}

namespace Shaders
{
	enum class ID
	{
		BrightnessPass,
		DownSamplePass,
		GaussianBlurPass,
		AddPass,
	};
}

namespace Fonts
{
	enum class ID
	{
		Main,
	};
}

// Forward declaration and a few type definitions
template <typename Resource, typename Identifier>
class ResourceHolder;

typedef ResourceHolder<sf::Texture, Textures::ID>	TextureHolder;
typedef ResourceHolder<sf::Font, Fonts::ID>			FontHolder;
typedef ResourceHolder<sf::Shader, Shaders::ID>			ShaderHolder;

