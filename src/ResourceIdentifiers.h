#pragma once

#include "ResourceHolder.h"

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
		Star,
		Bomb,
		PlayerShip,
		EnemyShip,
		BossShip,
		EnemyBomber,
		Heart,
		Station,
		BoosterPurple,
		BoosterYellow,
		Emp,
		Arrow,
		BackGround,
	};
}

namespace Shaders
{
	enum class ID
	{
		BrightnessPass,
		DownSamplePass,
		GaussianVertPass,
		GaussianHorizPass,
		AddPass,
		FullPass,
	};
}

namespace Fonts
{
	enum class ID
	{
		Main,
	};
}

// // Forward declaration and a few type definitions
// template <typename Resource, typename Identifier>
// class ResourceHolder;

// typedef ResourceHolder<Texture, Textures::ID>	TextureHolder;
// typedef ResourceHolder<Font, Fonts::ID>			FontHolder;

