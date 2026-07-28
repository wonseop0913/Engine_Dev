#pragma once

#define JPH_DEBUG_RENDERER

#include "Engine/pch.h"

#ifdef _DEBUG
#pragma comment(lib, "Engine/Debug/BulbEditorCore.lib")
#else
#pragma comment(lib, "Engine/Release/BulbEditorCore.lib")
#endif

#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"
#pragma comment(lib, "assimp/assimp-vc143-mt.lib")

#include "AssetLoader.h"