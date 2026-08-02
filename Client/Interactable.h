#pragma once
#include "Script.h"
class Interactable : public Script
{
public:
	virtual ~Interactable() = default;

	virtual void Interact(shared_ptr<GameObject> opponent) = 0;
};

