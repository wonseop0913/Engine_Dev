#include "pch.h"
#include "Utils.h"

mt19937 Utils::_gen;

DxException::DxException(HRESULT hr, const wstring& functionName, const wstring& filename, int lineNumber) :
	errorCode(hr), functionName(functionName), filename(filename), lineNumber(lineNumber)
{
	wchar_t errorMsg[512];
	swprintf_s(errorMsg, L"HRESULT: 0x%08X\nFile: %s\nLine: %d\nCode: %S", hr, filename.c_str(), lineNumber, functionName.c_str());
	MessageBox(nullptr, errorMsg, L"DirectX Critical Error", MB_OK | MB_ICONERROR);
}

wstring DxException::ToString() const
{
	_com_error err(errorCode);
	wstring msg = err.ErrorMessage();

	return functionName + L" failed in " + filename + L"; line " + std::to_wstring(lineNumber) + L"; error: " + msg;
}

void Utils::Init()
{
	random_device rd;
	_gen.seed(rd());
}

string Utils::ToString(wstring s)
{
	string result;
	result.assign(s.begin(), s.end());
	return result;
}

wstring Utils::ToWString(string s)
{
	if (s.empty()) return wstring();

	int bufferSize = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, nullptr, 0);
	if (bufferSize == 0) return wstring();

	wstring result(bufferSize, L'\0');
	if (MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, result.data(), bufferSize) == 0)
		return wstring();

	result.pop_back();
	return result;
}

char* Utils::ToChar(wstring s)
{
	const wchar_t* input = s.c_str();
	size_t size = (wcslen(input) + 1) * sizeof(wchar_t);
	char* buffer = new char[size];
	size_t convertedSize;
	wcstombs_s(&convertedSize, buffer, size, input, size);

	return buffer;
}

int Utils::Random(int from, int to)
{
	uniform_int_distribution<int> dis(from, to);
	return dis(_gen);
}

string Utils::GetFileName(string path)
{
	string result;

	size_t separateIdx = path.find_last_of('\\') + 1;
	result = path.erase(0, separateIdx == string::npos ? 0 : separateIdx);
	size_t extIdx = result.find_last_of('.');
	if (extIdx != string::npos) result.erase(extIdx, string::npos);

	return result;
}
