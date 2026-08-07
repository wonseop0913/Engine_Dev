#pragma once

enum class BULB_API ResourceType
{
	Undefined,
	Mesh,
	Texture,
	Shader,
	Material,
	Animation,

	End
};

enum
{
	RESOURCE_TYPE_COUNT = static_cast<UINT8>(ResourceType::End)
};

class BULB_API Resource
{
public:
	Resource(ResourceType type);
	virtual ~Resource();

	ResourceType GetType() { return _type; }

	void SetName(const string& name) { 
		_namew = Utils::ToWString(name);
	}
	void SetName(const wstring& name) { 
		_namew = name;
	}
	string GetName() { return Utils::ToString(_namew); }
	wstring GetNameW() { return _namew; }

	void SetPath(const string& path) {
		_pathw = Utils::ToWString(path);
	}
	void SetPath(const wstring& path) { 
		_pathw = path; 
	}
	string GetPath() { return Utils::ToString(_pathw); }
	wstring GetPathW() { return _pathw; }

	UINT GetID() { return _id; }

protected:
	virtual void Load(const wstring& path) { }

protected:
	ResourceType _type;

	wstring _namew;

	wstring _pathw;

	// ����� �� ���ҽ� Ÿ�Կ� ���� ���� ���̵� �����ϵ��� �ϰ�����.
	// ���� ������ ���̵� ���� �ٸ� Ÿ���� ���ҽ��� ������ �� ����.
	// ���߿� ���� ������ �� �� ���� ������ ���� �����ϱ�.
	// ex) id 1�� ���ÿ� ���� Mesh�� Animation -> ���� ����
	UINT _id = 0;
};
