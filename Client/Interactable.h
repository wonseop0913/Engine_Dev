#pragma once
#include "Script.h"
class Interactable : public Script
{
public:
	virtual ~Interactable() = default;

	virtual void Interact() = 0;
};

