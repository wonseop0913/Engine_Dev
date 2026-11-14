#include "pch.h"
#include "GameManager.h"

int Client::GameManager::GetLevelUpCost() const
{
	return _playerStatus.level * 100;
}

void Client::GameManager::AddCurrency(int amount)
{
	if (amount <= 0)
		return;

	_playerStatus.currency += amount;

	// 자동 레벨업 기능, 사용 금지
	//while (_playerStatus.currency >= GetLevelUpCost())
	//	LevelUp();
}

bool Client::GameManager::LevelUp()
{
	if (_playerStatus.currency < GetLevelUpCost()) return false;

	_playerStatus.currency -= GetLevelUpCost();
	++_playerStatus.level;
	return true;
}
