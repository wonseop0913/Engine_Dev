#include "pch.h"
#include "UIPanel.h"

UIPanel::UIPanel() : UIElement(UIType::Panel)
{
	_name = "UIPanel";
	SetTexture(L"Tex_Default");
}

UIPanel::UIPanel(UIType type) : UIElement((UINT)UIType::Panel | (UINT)type)
{
	_name = "UIPanel";
	SetTexture(L"Tex_Default");
}

UIPanel::~UIPanel()
{
	cout << "Released - UIPanel\n";
}

void UIPanel::LoadXML(XMLElement* uiElem)
{
	const char* texture = uiElem->Attribute("Texture", "");
	SetTexture(RESOURCE->Get<Texture>(Utils::ToWString(_textureName)));

	shared_ptr<UITransform> transform = GetTransform();

	transform->SetDepth(uiElem->FloatAttribute("Depth"));

	XMLElement* pivotElem = uiElem->FirstChildElement("Pivot");
	transform->SetPivot({ pivotElem->FloatAttribute("x", 0.5f), pivotElem->FloatAttribute("y", 0.5f) });

	XMLElement* posElem = uiElem->FirstChildElement("Position");
	transform->SetLocalPosition({ posElem->FloatAttribute("x"), posElem->FloatAttribute("y"), posElem->FloatAttribute("z") });

	XMLElement* sizeElem = uiElem->FirstChildElement("Size");
	transform->SetStretchSize(sizeElem->BoolAttribute("StretchByParent", false));
	transform->SetSize({ sizeElem->FloatAttribute("x"), sizeElem->FloatAttribute("y") });

	XMLElement* colorElem = uiElem->FirstChildElement("Color");
	_color = { colorElem->FloatAttribute("r", 1.0f), colorElem->FloatAttribute("g", 1.0f), colorElem->FloatAttribute("b", 1.0f), colorElem->FloatAttribute("a", 1.0f) };
}

void UIPanel::Render(ID3D12GraphicsCommandList* cmdList)
{

}

void UIPanel::OnMouseEnter()
{

}

void UIPanel::OnMouseExit()
{

}

void UIPanel::SetTexture(shared_ptr<Texture> texture)
{
	if (texture == nullptr) {
		SetTexture(L"Tex_Default");
		return;
	}

	_textureSrvHeapIndex = texture->GetSRVHeapIndex();
	_textureName = texture->GetName();
}

void UIPanel::SetTexture(wstring textureName)
{
	auto texture = RESOURCE->Get<Texture>(textureName);
	if (texture == nullptr) {
		if (filesystem::exists(textureName)) {
			texture = make_shared<Texture>(textureName);
			RESOURCE->Add(textureName, texture);
		}
		else
			texture = RESOURCE->Get<Texture>(L"Tex_Default");
	}

	_textureSrvHeapIndex = texture->GetSRVHeapIndex();
	_textureName = Utils::ToString(textureName);
}
