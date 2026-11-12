#pragma once

namespace Client
{
	class GameManager
	{
	public:
		GameManager() = default;
		~GameManager() = default;

		GameManager(const GameManager& rhs) = delete;
		GameManager& operator=(const GameManager& rhs) = delete;
	};
}
