#pragma once

namespace Client
{
	class UIManager
	{
	public:
		UIManager() = default;
		~UIManager() = default;

		UIManager(const UIManager& rhs) = delete;
		UIManager& operator=(const UIManager& rhs) = delete;


		void Update();

	public:
		void AddUI(shared_ptr<UIElement> ui);
		void PopUI();

		bool IsAnyUIActive() { return _elements.size() > 0; }

	private:
		vector<shared_ptr<UIElement>> _elements;
	};
}
