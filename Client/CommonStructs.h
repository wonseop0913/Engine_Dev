#pragma once

struct AttackInfo {
	shared_ptr<GameObject> attakee;
	int damage;
};

struct PlayerStatus {
	int level = 1;
	int currency = 0;
};
