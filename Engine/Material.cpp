#include "pch.h"
#include "Material.h"

int Material::_count = 0;

Material::Material() : Super(ResourceType::Material)
{
	matSBIndex = _count++;

	diffuseSrvHeapIndex = 0;
	normalSrvHeapIndex = 0;
	numFramesDirty = RENDER->GetNumFrameResources();

	ambient = { 1.0f, 1.0f, 1.0f, 1.0f };
	diffuse = { 1.0f, 1.0f, 1.0f, 1.0f };
	specular = { 1.0f, 1.0f, 1.0f, 1.0f };
	emissive = { 0.0f, 0.0f, 0.0f, 0.0f };
	tilling = { 1.0f, 1.0f };
	metallic = 0.0f;
	roughness = 1.0f;
	matTransform = MathHelper::Identity4x4();
}

Material::Material(string name) : Material(name, L"Tex_Default")
{

}

Material::Material(string name, wstring textureName) : Super(ResourceType::Material)
{
	matSBIndex = _count++;
	normalSrvHeapIndex = 0;
	numFramesDirty = RENDER->GetNumFrameResources();
	SetName(name);

	ambient = { 1.0f, 1.0f, 1.0f, 1.0f };
	diffuse = { 1.0f, 1.0f, 1.0f, 1.0f };
	specular = { 1.0f, 1.0f, 1.0f, 1.0f };
	emissive = { 0.0f, 0.0f, 0.0f, 0.0f };
	tilling = { 1.0f, 1.0f };
	metallic = 0.0f;
	roughness = 1.0f;
	matTransform = MathHelper::Identity4x4();

	SetDiffuse(RESOURCE->Get<Texture>(textureName));
}

Material::~Material()
{

}

void Material::SetDiffuse(shared_ptr<Texture> texture)
{
	if (texture == nullptr)
	{
		SetDiffuse(L"Tex_Default");
		return;
	}

	diffuseSrvHeapIndex = texture->GetSRVHeapIndex();
	diffuseTextureName = texture->GetName();
}

void Material::SetDiffuse(wstring textureName)
{
	// �ؽ��� �̸��� �����ϰ�, ���� �ؽ��Ĵ� �⺻ �ؽ��ķ�
	diffuseSrvHeapIndex = 0;
	diffuseTextureName = Utils::ToString(textureName);
}

void Material::SetNormal(shared_ptr<Texture> texture)
{
	if (texture == nullptr)
	{
		SetNormal(L"Tex_Default");
		return;
	}

	normalSrvHeapIndex = texture->GetSRVHeapIndex();
	normalTextureName = texture->GetName();
}

void Material::SetNormal(wstring textureName)
{
	normalSrvHeapIndex = 0;
	normalTextureName = Utils::ToString(textureName);
}

void Material::SetMetallicMap(shared_ptr<Texture> texture)
{
	if (texture == nullptr)
	{
		SetMetallicMap(L"Tex_Default");
		return;
	}

	metallicSrvHeapIndex = texture->GetSRVHeapIndex();
	metallicTextureName = texture->GetName();
}

void Material::SetMetallicMap(wstring textureName)
{
	metallicSrvHeapIndex = 0;
	metallicTextureName = Utils::ToString(textureName);
}

void Material::SetRoughnessMap(shared_ptr<Texture> texture)
{
	if (texture == nullptr)
	{
		SetRoughnessMap(L"Tex_Default");
		return;
	}

	roughnessSrvHeapIndex = texture->GetSRVHeapIndex();
	roughnessTextureName = texture->GetName();
}

void Material::SetRoughnessMap(wstring textureName)
{
	roughnessSrvHeapIndex = 0;
	roughnessTextureName = Utils::ToString(textureName);
}

void Material::SetSpecularMap(shared_ptr<Texture> texture)
{
	if (texture == nullptr)
	{
		SetSpecularMap(L"Tex_Default");
		return;
	}

	specularSrvHeapIndex = texture->GetSRVHeapIndex();
	specularTextureName = texture->GetName();
}

void Material::SetSpecularMap(wstring textureName)
{
	specularSrvHeapIndex = 0;
	specularTextureName = Utils::ToString(textureName);
}
