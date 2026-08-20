#pragma once

#define COUNT_COMPONENTTYPE	12
enum class BULB_API ComponentType
{
	Undefined			 = 0,
	Transform			 = 1 << 0,
	MeshRenderer		 = 1 << 1,
	SkinnedMeshRenderer	 = 1 << 2,
	Camera				 = 1 << 3,
	Rigidbody			 = 1 << 4,
	Light				 = 1 << 5,
	Animator			 = 1 << 6,
	Script				 = 1 << 7,
	ParticleEmitter		 = 1 << 8,
	CharacterController	 = 1 << 9,
	Terrain				 = 1 << 10,
	AudioSource			 = 1 << 11
};

// For runtime add feature
enum class BULB_API UsableComponentType {
	MeshRenderer,
	SkinnedMeshRenderer,
	Camera,
	Rigidbody,
	DirectionalLight,
	PointLight,
	Animator,
	ParticleEmitter,
	CharacterController,
	Terrain,
	AudioSource,
	Script
};

template<>
struct magic_enum::customize::enum_range<UsableComponentType> {
	constexpr static auto prefix_length = sizeof("ComponentType_") - 1;
	constexpr static int min = (int)UsableComponentType::MeshRenderer;
	constexpr static int max = (int)UsableComponentType::Script;
};

enum class BULB_API UIType
{
	Frame = 1 << 0,
	Panel = 1 << 1,
	Button = 1 << 2,
	Slider = 1 << 3,
	Text = 1 << 4
};

namespace Bulb {
	enum ProcessResult {
		SUCCESS,
		FAILED_INSTANCE_NOT_FOUND
	};
}