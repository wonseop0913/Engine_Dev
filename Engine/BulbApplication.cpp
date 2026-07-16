#include "pch.h"
#include "BulbApplication.h"

BulbApplication* BulbApplication::s_instance = nullptr;

BulbApplication::~BulbApplication()
{
#ifdef PRINT_DEBUG_CONSOLE_LOG
	cout << "Released - ResourceManager\n";
#endif
}

BulbApplication* BulbApplication::GetInstance()
{
	if (s_instance == nullptr)
		s_instance = new BulbApplication();
	return s_instance;
}

Bulb::ProcessResult BulbApplication::Delete()
{
	if (s_instance != nullptr) {
		delete s_instance;
		s_instance = nullptr;
		return Bulb::ProcessResult::SUCCESS;
	}
	return Bulb::ProcessResult::FAILED_INSTANCE_NOT_FOUND;
}

HINSTANCE BulbApplication::GetAppInst() const
{
	return _hAppInst;
}

void BulbApplication::SetAppInst(HINSTANCE hInstance, AppDesc appDesc)
{
	_hAppInst = hInstance;
	GRAPHIC->SetAppDesc(appDesc);
}

AppStatus BulbApplication::GetAppStatus() const
{
	return _appStatus;
}

void BulbApplication::SetAppStatus(AppStatus appStatus)
{
	_appStatus = appStatus;
}

int BulbApplication::Run()
{
	if (!Init())
		return 0;

	DEBUG->Log("Application Initialized");

	MSG msg = { 0 };

	TIME->Reset();

	DEBUG->Log("Timer Reset");

	while (msg.message != WM_QUIT)
	{
		// If there are Window messages then process them.
		if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		// Otherwise, do animation/game stuff.
		else
		{
			if (_isExitReserved) {
				SendMessage(GRAPHIC->GetMainWnd(), WM_CLOSE, 0, 0);
				continue;
			}

			TIME->Tick();

			PreUpdate();
			Update();
		}
	}

	ExitApplication();

	return (int)msg.wParam;
}

void BulbApplication::QuitApplication()
{
	_isExitReserved = true;
}

// 각 코어시스템별 초기화 진행
bool BulbApplication::Init()
{
	if (!TIME->Init())
		return false;
	if (!GRAPHIC->Init())
		return false;

	ComponentFactory::Init();
	THREAD->Init();
	RENDER->Init();
	FILEIO->Init();
	UI->Init();
	PHYSICS->Init();
	SCENE->Init();
	INPUTM->Init();
	SOUND->Init();
	Utils::Init();

	DEBUG->Init();
#ifdef BULB_EDITOR
	EDITOR->Init();
#endif

	return true;
}

void BulbApplication::PreUpdate()
{
	if (!_appStatus.appPaused) {
		SCENE->PreUpdate();
		DEBUG->PreUpdate();
		PHYSICS->PreUpdate();
		RENDER->PreUpdate();
	}
}

void BulbApplication::Update()
{
	if (!_appStatus.appPaused) {
		INPUTM->Update();
		GRAPHIC->Update();
		PHYSICS->Update();
		RENDER->Update();
		ANIMATION->Update();
		ENGINESTAT->Update();
		SOUND->Update();
		UI->Update();
#ifdef BULB_EDITOR
		EDITORGUI->Update();
#endif
		DEBUG->Update();

		GRAPHIC->LateUpdate();
		PHYSICS->LateUpdate();

		RENDER->Render();
	}
	else
	{
		Sleep(100);
	}
}

void BulbApplication::ExitApplication()
{
	INPUTM->Delete();
#ifdef BULB_EDITOR
	EDITOR->Delete();
#endif
	DEBUG->Delete();
	SCENE->Delete();
	FILEIO->Delete();
	RESOURCE->Delete();
#ifdef BULB_EDITOR
	EDITORGUI->Delete();
#endif
	RENDER->Delete();
	UI->Delete();
	PHYSICS->Delete();
	PARTICLE->Delete();
	SKELETON->Delete();
	ANIMATION->Delete();
	ENGINESTAT->Delete();
	SOUND->Delete();
	THREAD->Delete();

	GRAPHIC->Delete();
	TIME->Delete();

	PostQuitMessage(0);
}
