#pragma once

#define REGISTER_COMPONENT(className)				\
	static ComponentRegisterHelper<className> _reg_##className(#className);

template<typename T>
struct ComponentRegisterHelper {
	ComponentRegisterHelper(string name) {
		ComponentFactory::Register<T>(name);
	}
};

class BULB_API ComponentFactory
{
	friend class BulbApplication;

	using Creator = function<shared_ptr<Component>()>;

private:
	static void Init() {
		REGISTER_COMPONENT(DirectionalLight);
		REGISTER_COMPONENT(PointLight);
		REGISTER_COMPONENT(MeshRenderer);
		REGISTER_COMPONENT(SkinnedMeshRenderer);
		REGISTER_COMPONENT(Rigidbody);
		REGISTER_COMPONENT(Animator);
		REGISTER_COMPONENT(Camera);
		REGISTER_COMPONENT(CharacterController);
		REGISTER_COMPONENT(ParticleEmitter);
		REGISTER_COMPONENT(Transform);
		REGISTER_COMPONENT(Terrain);
		REGISTER_COMPONENT(AudioSource);
#ifdef BULB_EDITOR
		REGISTER_COMPONENT(EditorCamera);
#endif
	}

	static unordered_map<string, Creator>& GetMap() {
		static unordered_map<string, Creator> _registry;
		return _registry;
	}

public:
	template<typename T>
	static void Register(string name) {
		GetMap()[name] = []() { return make_shared<T>(); };
	}

	static shared_ptr<Component> Create(string name) {
		if (GetMap().contains(name)) return GetMap()[name]();
		return nullptr;
	}
};

