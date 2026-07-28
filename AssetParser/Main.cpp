#include "pch.h"

#define CURRENT_VERSION		"v0.2.0"
#define CONSOLESEPARATOR	"======================================================================"


int assetCount = 0;
vector<wstring> parsableAssetPaths;
vector<string> parsableAssetNames;

filesystem::path settingFileDir("./ParserSetting.ini");

void DirectoryIterator(filesystem::path path, int count);

int main()
{
	ComponentFactory::Register<MeshRenderer>("MeshRenderer");
	ComponentFactory::Register<MeshRenderer>("SkinnedMeshRenderer");
	ComponentFactory::Register<MeshRenderer>("Animator");

	cout << "Bulb Engine Asset Parser " << CURRENT_VERSION << endl << endl;

	if (!filesystem::exists(settingFileDir)) {
		cout << "ParserSetting.ini does not exists! Make sure that the setting file is in the proper path." << endl;
		return 0;
	}

	string assetDir = FILEIO->ReadINI("ParserSetting", "AssetSourceDir", settingFileDir.string());
	string parsedDir = FILEIO->ReadINI("ParserSetting", "ParsedSourceDir", settingFileDir.string());

	if (assetDir == "none" || parsedDir == "none") {
		cout << "Failed read some directories from setting file! Check setting file contents." << endl;
		return 0;
	}

	cout << "Asset Source Directory  - " + assetDir << endl;
	cout << "Parsed Source Directory - " + parsedDir << endl << endl;

	cout << CONSOLESEPARATOR << endl << endl;

	cout << "Asset list" << endl << endl;
	filesystem::path resourcePath(assetDir);
	DirectoryIterator(resourcePath, 0);
	cout << endl << "Found Parsable Assets: " << assetCount << endl << endl;

	cout << CONSOLESEPARATOR << endl << endl;

	// 여기서부터 파싱 부분
	for (int i = 0; i < parsableAssetPaths.size(); i++)
	{
		shared_ptr<AssetLoader> loader = make_shared<AssetLoader>();
		loader->SetAssetPath(Utils::ToWString(assetDir));

		wstring assetPathStr = parsableAssetPaths[i];
		cout << "Parsing: " << Utils::ToString(assetPathStr.c_str()) << endl;
		loader->ImportAssetFile(assetPathStr);

		filesystem::path assetPath(assetPathStr);
		filesystem::path parsedAssetPath(parsedDir + parsableAssetNames[i]);
		filesystem::copy(assetPath, parsedAssetPath);
		filesystem::remove(assetPath);
	}

	cout << CONSOLESEPARATOR << endl << endl;

	cout << assetCount << " assets parsed." << endl;

	system("pause");
	return 0;
}

void DirectoryIterator(filesystem::path path, int count)
{
	filesystem::directory_iterator iter(path);
	for (auto& i = iter; i != filesystem::end(iter); i++) {
		string pathStr = Utils::ToString(i->path().c_str());
		string name = pathStr.substr(pathStr.find_last_of('\\') + 1);

		for (int j = 0; j < count; j++)
			cout << "-";

		if (i->is_directory()) {
			if (name == "Parsed") continue;

			cout << name << endl;
			DirectoryIterator(i->path(), count + 1);
		}
		else {
			cout << name << endl;
			if (name.find(".fbx") == string::npos && 
				name.find(".FBX") == string::npos && 
				name.find(".gltf") == string::npos)
				continue;

			assetCount++;
			parsableAssetPaths.push_back(i->path().c_str());
			parsableAssetNames.push_back(name);
		}
	}
}
