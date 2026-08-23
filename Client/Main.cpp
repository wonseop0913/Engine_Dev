#include "pch.h"
#include "Main.h"
#include "BossScript.h"
#include "PlayerScript.h"
#include "TPVCamera.h"
#include "StartMenuSceneScript.h"
#include "MainSceneScript.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR cmdLine, int nShowCmd)
{
	AppDesc desc;
	desc.mainWndCaption = L"Bulb Engine";
	desc._4xMsaaState = true;
	desc._4xMsaaQuality = 1;
	desc.clientWidth = 1600;
	desc.clientHeight = 900;

	string startSceneName = FILEIO->ReadINI("Common", "StartScene", "./ClientSetting.ini");
	//string startSceneName = "none";
	if (startSceneName != "none") {
		SCENE->LoadScene(startSceneName);

		APP->SetAppInst(hInstance, desc);

		APP->Run();
	}
	else {
		string content = "Can't read start scene information from ClientSetting.ini.\nCheck ClientSetting.ini file exists or its content.";
		MessageBoxA(
			NULL,
			content.c_str(),
			"Error",
			MB_OK | MB_ICONERROR
		);
	}

	return 0;
}