#include "pch.h"
#include "UIManager.h"


void Client::UIManager::Update()
{
	if (INPUTM->IsKeyDown(ESC)) {
		PopUI();
	}
}

void Client::UIManager::AddUI(shared_ptr<UIElement> ui)
{
	_elements.push_back(ui);
	if (!ui->IsRenderActive())
		ui->SetRenderActive(true);
	if (INPUTM->IsMouseCenterFixed()) ShowCursor(TRUE);
	INPUTM->SetMouseCenterFixMode(false);
}

void Client::UIManager::PopUI()
{
	if (_elements.size() == 0) return;

	shared_ptr<UIElement> lastUI = _elements[_elements.size() - 1];

	if (lastUI->IsRenderActive())
		lastUI->SetRenderActive(false);

	_elements.erase(_elements.begin() + _elements.size() - 1);

	if (_elements.size() == 0) {
		if (!INPUTM->IsMouseCenterFixed()) ShowCursor(false);
		INPUTM->SetMouseCenterFixMode(true);
	}
}
