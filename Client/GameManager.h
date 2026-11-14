#pragma once
#include "CommonStructs.h"

namespace Client
{
	class GameManager
	{
	public:
		GameManager() = default;
		~GameManager() = default;

		GameManager(const GameManager& rhs) = delete;
		GameManager& operator=(const GameManager& rhs) = delete;

	public:
		int GetCurrency() const { return _playerStatus.currency; }
		int GetLevel() const { return _playerStatus.level; }
		int GetLevelUpCost() const;

		void AddCurrency(int amount);

		bool LevelUp();

		PlayerStatus _playerStatus;
	};
}
