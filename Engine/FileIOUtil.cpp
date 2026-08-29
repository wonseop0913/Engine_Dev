#include "pch.h"
#include "FileIOUtil.h"

FileIOUtil* FileIOUtil::s_instance = nullptr;

FileIOUtil* FileIOUtil::GetInstance()
{
	if (s_instance == nullptr)
		s_instance = new FileIOUtil();
	return s_instance;
}

Bulb::ProcessResult FileIOUtil::Delete()
{
	if (s_instance != nullptr) {
		delete s_instance;
		s_instance = nullptr;
		return Bulb::ProcessResult::SUCCESS;
	}
	return Bulb::ProcessResult::FAILED_INSTANCE_NOT_FOUND;
}

void FileIOUtil::Init()
{
	//std::future<void> tex, mat;
	//tex = THREAD->EnqueueJob([this] { LoadTextures(); });
	//mat = THREAD->EnqueueJob([this] { LoadMaterials(); });
#ifdef BULB_EDITOR
	LoadTextures();
#endif
	LoadMaterials();
}

void FileIOUtil::XMLFromMaterial(shared_ptr<Material> material, const wstring& name)
{
	if (!filesystem::exists(RESOURCE_PATH_MATERIALW + name))
		filesystem::create_directory(RESOURCE_PATH_MATERIALW + name);
	if (filesystem::exists(RESOURCE_PATH_MATERIALW + name + L"\\" + material->GetNameW() + L".xml"))
		return;

	char* nameChar = Utils::ToChar(material->GetNameW());
	tinyxml2::XMLDocument doc;

	XMLNode* node = doc.NewElement("Material");
	doc.InsertFirstChild(node);

	XMLElement* element = doc.NewElement("Name");
	element->SetText(nameChar);
	node->InsertEndChild(element);

	element = doc.NewElement("Ambient");
	element->SetAttribute("r", material->ambient.r);
	element->SetAttribute("g", material->ambient.g);
	element->SetAttribute("b", material->ambient.b);
	element->SetAttribute("a", material->ambient.a);
	node->InsertEndChild(element);

	element = doc.NewElement("Diffuse");
	element->SetAttribute("r", material->diffuse.r);
	element->SetAttribute("g", material->diffuse.g);
	element->SetAttribute("b", material->diffuse.b);
	element->SetAttribute("a", material->diffuse.a);
	node->InsertEndChild(element);

	element = doc.NewElement("Specular");
	element->SetAttribute("r", material->specular.r);
	element->SetAttribute("g", material->specular.g);
	element->SetAttribute("b", material->specular.b);
	element->SetAttribute("a", material->specular.a);
	node->InsertEndChild(element);

	element = doc.NewElement("Emissive");
	element->SetAttribute("r", material->emissive.r);
	element->SetAttribute("g", material->emissive.g);
	element->SetAttribute("b", material->emissive.b);
	element->SetAttribute("a", material->emissive.a);
	node->InsertEndChild(element);

	element = doc.NewElement("Metallic");
	element->SetAttribute("value", material->metallic);
	node->InsertEndChild(element);

	element = doc.NewElement("Roughness");
	element->SetAttribute("value", material->roughness);
	node->InsertEndChild(element);

	element = doc.NewElement("DiffuseTexture");
	element->SetText(material->diffuseTextureName.c_str());
	node->InsertEndChild(element);

	element = doc.NewElement("NormalTexture");
	element->SetText(material->normalTextureName.c_str());
	node->InsertEndChild(element);

	element = doc.NewElement("MetallicTexture");
	element->SetText(material->metallicTextureName.c_str());
	node->InsertEndChild(element);

	element = doc.NewElement("RoughnessTexture");
	element->SetText(material->roughnessTextureName.c_str());
	node->InsertEndChild(element);

	element = doc.NewElement("SpecularTexture");
	element->SetText(material->specularTextureName.c_str());
	node->InsertEndChild(element);

	char fullpath[100] = RESOURCE_PATH_MATERIAL;
	strcat(fullpath, Utils::ToChar(name));
	strcat(fullpath, "\\");
	strcat(fullpath, nameChar);
	strcat(fullpath, ".xml");
	doc.SaveFile(fullpath);
}

void FileIOUtil::LoadTextures()
{
	filesystem::path p = RESOURCE_PATH_TEXTURE;
	filesystem::recursive_directory_iterator iter(p);

	for (auto& i = iter; i != filesystem::end(iter); i++)
	{
		istringstream ss(Utils::ToString(i->path().c_str()));
		string fileName;
		while (getline(ss, fileName, '\\'));
		ss = istringstream(fileName);
		string format;
		while (getline(ss, format, '.'));
		if (format != "dds" && format != "png" && format != "jpg")
			continue;
		
		//auto defaultTex = make_shared<Texture>(UniversalUtils::ToWString(fileName));
		auto defaultTex = make_shared<Texture>(i->path().c_str());
		defaultTex->SetName(fileName);
		RESOURCE->Add<Texture>(Utils::ToWString(defaultTex->GetPath()), defaultTex);
	}
}

void FileIOUtil::LoadMaterials()
{
	tinyxml2::XMLDocument doc;
	filesystem::path p = RESOURCE_PATH_MATERIAL;
	filesystem::recursive_directory_iterator iter(p);

	for (auto& i = iter; i != filesystem::end(iter); i++)
	{
		XMLError e = doc.LoadFile(Utils::ToChar(i->path().c_str()));
		if (e != XML_SUCCESS)
			continue;

		XMLNode* node = doc.FirstChild();

		XMLElement* element = node->FirstChildElement("Name");
		string name = "";
		if (element->GetText())
			name = string(element->GetText());

		shared_ptr<Material> mat = make_shared<Material>(name);
		mat->SetPath(i->path());

		element = node->FirstChildElement("Ambient");
		if (element) {
			Bulb::Color ambient;
			ambient.r = element->FloatAttribute("r");
			ambient.g = element->FloatAttribute("g");
			ambient.b = element->FloatAttribute("b");
			ambient.a = element->FloatAttribute("a");

			mat->ambient = ambient;
		}

		element = node->FirstChildElement("Diffuse");
		if (element) {
			Bulb::Color diffuse;
			diffuse.r = element->FloatAttribute("r");
			diffuse.g = element->FloatAttribute("g");
			diffuse.b = element->FloatAttribute("b");
			diffuse.a = element->FloatAttribute("a");

			mat->diffuse = diffuse;
		}

		element = node->FirstChildElement("Specular");
		if (element) {
			Bulb::Color specular;
			specular.r = element->FloatAttribute("r");
			specular.g = element->FloatAttribute("g");
			specular.b = element->FloatAttribute("b");
			specular.a = element->FloatAttribute("a");

			mat->specular = specular;
		}

		element = node->FirstChildElement("Emissive");
		if (element) {
			Bulb::Color emissive;
			emissive.r = element->FloatAttribute("r");
			emissive.g = element->FloatAttribute("g");
			emissive.b = element->FloatAttribute("b");
			emissive.a = element->FloatAttribute("a");

			mat->emissive = emissive;
		}

		element = node->FirstChildElement("Metallic");
		if (element) {
			float metallic = element->FloatAttribute("value", 0.5f);

			mat->metallic = metallic;
		}

		element = node->FirstChildElement("Roughness");
		if (element) {
			float roughness = element->FloatAttribute("value", 0.5f);

			mat->roughness = roughness;
		}

		element = node->FirstChildElement("Tilling");
		if (element) {
			float x = element->FloatAttribute("x", 1.0f);
			float y = element->FloatAttribute("y", 1.0f);

			mat->tilling = { x, y };
		}

		string textureName = "";
		element = node->FirstChildElement("DiffuseTexture");
		const char* text = element->GetText();
		if (text) {
			textureName = string(text);

			shared_ptr<Texture> tex = RESOURCE->Get<Texture>(Utils::ToWString(textureName));
			if (tex == nullptr) {
				tex = make_shared<Texture>(Utils::ToWString(textureName));
				tex->SetName(textureName);
				RESOURCE->Add<Texture>(Utils::ToWString(tex->GetPath()), tex);
			}
			mat->SetDiffuse(tex);
		}

		element = node->FirstChildElement("NormalTexture");
		text = element->GetText();
		if (text) {
			textureName = string(text);

			shared_ptr<Texture> tex = RESOURCE->Get<Texture>(Utils::ToWString(textureName));
			if (tex == nullptr) {
				tex = make_shared<Texture>(Utils::ToWString(textureName));
				tex->SetName(textureName);
				RESOURCE->Add<Texture>(Utils::ToWString(tex->GetPath()), tex);
			}
			mat->SetNormal(tex);
		}

		element = node->FirstChildElement("MetallicTexture");
		if (element != nullptr) {
			text = element->GetText();
			if (text) {
				textureName = string(text);

				shared_ptr<Texture> tex = RESOURCE->Get<Texture>(Utils::ToWString(textureName));
				if (tex == nullptr) {
					tex = make_shared<Texture>(Utils::ToWString(textureName));
					tex->SetName(textureName);
					RESOURCE->Add<Texture>(Utils::ToWString(tex->GetPath()), tex);
				}
				mat->SetMetallicMap(tex);
			}
		}

		element = node->FirstChildElement("RoughnessTexture");
		if (element != nullptr) {
			text = element->GetText();
			if (text) {
				textureName = string(text);

				shared_ptr<Texture> tex = RESOURCE->Get<Texture>(Utils::ToWString(textureName));
				if (tex == nullptr) {
					tex = make_shared<Texture>(Utils::ToWString(textureName));
					tex->SetName(textureName);
					RESOURCE->Add<Texture>(Utils::ToWString(tex->GetPath()), tex);
				}
				mat->SetRoughnessMap(tex);
			}
		}

		element = node->FirstChildElement("SpecularTexture");
		if (element != nullptr) {
			text = element->GetText();
			if (text) {
				textureName = string(text);

				shared_ptr<Texture> tex = RESOURCE->Get<Texture>(Utils::ToWString(textureName));
				if (tex == nullptr) {
					tex = make_shared<Texture>(Utils::ToWString(textureName));
					tex->SetName(textureName);
					RESOURCE->Add<Texture>(Utils::ToWString(tex->GetPath()), tex);
				}
				mat->SetSpecularMap(tex);
			}
		}

		RESOURCE->Add<Material>(i->path(), mat);
	}

	auto mats = RESOURCE->GetByType<Material>();
}

HANDLE FileIOUtil::CreateFileHandle(string fileName, string ext)
{
	string fullPath;

	if (PathFileExistsA(fileName.c_str()))
		fullPath = fileName;
	else
	{
		if (ext == "")
			fullPath = RESOURCE_PATH + fileName + BULB_EXT_UNKNOWN;
		else
			fullPath = RESOURCE_PATH + fileName + "." + ext;

		string subPath = fullPath.substr(0, fullPath.find_last_of('\\') + 1);
		if (!filesystem::exists(subPath))
			filesystem::create_directories(subPath);
	}

	if (PathFileExistsA(fullPath.c_str())) {
		return CreateFileA(
			fullPath.c_str(),
			GENERIC_READ | GENERIC_WRITE,
			0,
			NULL,
			OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL,
			NULL
		);
	}
	else {
		return CreateFileA(
			fullPath.c_str(),
			GENERIC_READ | GENERIC_WRITE,
			0,
			NULL,
			CREATE_NEW,
			FILE_ATTRIBUTE_NORMAL,
			NULL
		);
	}
}

void FileIOUtil::WriteToFile(HANDLE fileHandle, void* data, UINT32 size)
{
	WriteFile(
		fileHandle,
		&data,
		size,
		NULL,
		NULL
	);
}

void FileIOUtil::WriteToFile(HANDLE fileHandle, const string& data)
{
	UINT32 size = (UINT32)data.size();
	WriteToFile(fileHandle, size);

	if (size == 0)
		return;

	WriteFile(
		fileHandle,
		(data.data()),
		size,
		NULL,
		NULL
	);
}

void FileIOUtil::ReadFileData(HANDLE fileHandle, string& out)
{
	UINT32 size;
	ReadFile(
		fileHandle,
		&size,
		sizeof(UINT32),
		NULL,
		NULL
	);

	char* data = new char[size + 1];
	data[size] = 0;
	ReadFile(
		fileHandle,
		data,
		size,
		NULL,
		NULL
	);

	out = data;
	delete[] data;
}

void FileIOUtil::ReadFileData(HANDLE fileHandle, void* out, UINT32 dataSize)
{
	ReadFile(
		fileHandle,
		out,
		dataSize,
		NULL,
		NULL
	);
}

string FileIOUtil::ReadINI(string section, string key, string dir, string default)
{
	wstring sectionW = Utils::ToWString(section);
	wstring keyW = Utils::ToWString(key);
	wstring dirW = Utils::ToWString(dir);
	wstring defaultW = Utils::ToWString(default);

	wchar_t out[255];
	GetPrivateProfileString(sectionW.c_str(), keyW.c_str(), defaultW.c_str(), out, 255, dirW.c_str());
	return Utils::ToString(out);
}
