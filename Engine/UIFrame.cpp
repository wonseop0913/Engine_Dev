#include "pch.h"
#include "UIFrame.h"

UIFrame::UIFrame() : UIElement(UIType::Frame)
{
	_passthroughMouse = true;
}

UIFrame::~UIFrame()
{
	cout << "Released - UIFrame\n";
}

void UIFrame::LoadXML(XMLElement* uiElem)
{
	
}
