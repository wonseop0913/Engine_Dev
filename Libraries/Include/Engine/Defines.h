#pragma once

#define GET_SINGLE(classname)		classname::GetInstance()

#ifndef ReleaseCom
#define ReleaseCom(x) { if (x) { x->Release(); x = 0; } }
#endif

#ifdef BULB_EDITOR
#define EDITOR		GET_SINGLE(EditorManager)
#define EDITORGUI	GET_SINGLE(EditorGUIManager)
#define DEBUG		GET_SINGLE(DebugManager)
#endif
#define APP			GET_SINGLE(BulbApplication)
#define GRAPHIC		GET_SINGLE(Graphic)
#define TIME		GET_SINGLE(GameTimer)
#define RESOURCE	GET_SINGLE(ResourceManager)
#define RENDER		GET_SINGLE(RenderManager)
#define INPUTM		GET_SINGLE(InputManager)
#define ENGINESTAT	GET_SINGLE(EngineStatusManager)
#define PHYSICS		GET_SINGLE(PhysicsManager)
#define PARTICLE	GET_SINGLE(ParticleManager)
#define SKELETON	GET_SINGLE(SkeletonManager)
#define THREAD		GET_SINGLE(ThreadManager)
#define ANIMATION	GET_SINGLE(AnimationManager)
#define UI			GET_SINGLE(UIManager)
#define SCENE		GET_SINGLE(SceneManager)
#define SOUND		GET_SINGLE(SoundManager)

#define FILEIO		GET_SINGLE(FileIOUtil)


/**************/
/* Extentions */
/**************/
#define BULB_EXT_UNKNOWN			".brsc"
#define BULB_EXT_UNKNOWNW			L".brsc"
#define BULB_EXT_MESH				".bmesh"
#define BULB_EXT_MESHW				L".bmesh"
#define BULB_EXT_ANIMATION			".banim"
#define BULB_EXT_ANIMATIONW			L".banim"
#define BULB_EXT_BONE				".bbone"
#define BULB_EXT_BONEW				L".bbone"
#define BULB_EXT_PREFAB				".bprefab"
#define BULB_EXT_PREFABW			L".bprefab"


/*********/
/* Paths */
/*********/
#define RESOURCE_PATH					"..\\Resources\\"
#define RESOURCE_PATHW					L"..\\Resources\\"
#define RESOURCE_PATH_MATERIAL			"..\\Resources\\Materials\\"
#define RESOURCE_PATH_MATERIALW			L"..\\Resources\\Materials\\"
#define RESOURCE_PATH_SHADER			"..\\Resources\\Shaders\\"
#define RESOURCE_PATH_SHADERW			L"..\\Resources\\Shaders\\"
#define RESOURCE_PATH_TEXTURE			"..\\Resources\\Textures\\"
#define RESOURCE_PATH_TEXTUREW			L"..\\Resources\\Textures\\"
#define RESOURCE_PATH_MESH				"..\\Resources\\Meshes\\"
#define RESOURCE_PATH_MESHW				L"..\\Resources\\Meshes\\"
#define RESOURCE_PATH_ANIMATION			"..\\Resources\\Animations\\"
#define RESOURCE_PATH_ANIMATIONW		L"..\\Resources\\Animations\\"
#define RESOURCE_PATH_BONE				"..\\Resources\\Bones\\"
#define RESOURCE_PATH_BONEW				L"..\\Resources\\Bones\\"
#define RESOURCE_PATH_PREFAB			"..\\Resources\\Prefabs\\"
#define RESOURCE_PATH_PREFABW			L"..\\Resources\\Prefabs\\"
#define RESOURCE_PATH_ASSET				"..\\Resources\\Assets\\"
#define RESOURCE_PATH_ASSETW			L"..\\Resources\\Assets\\"
#define RESOURCE_PATH_SCENE				"..\\Resources\\Scenes\\"
#define RESOURCE_PATH_SCENEW			L"..\\Resources\\Scenes\\"


/**********/
/* Values */
/**********/

#define GRAVITY_ACC		9.8f