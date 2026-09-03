#pragma once
#include "Resource.h"

class BULB_API Animation : public Resource
{
public:
	struct KeyFrame
	{
		// �ʱⰪ -1.0���� null üũ
		KeyFrame() : tick(-1.0) {}

		double tick;
		Bulb::Vector3 position;
		Bulb::Vector4 rotation;
		Bulb::Vector3 scale;
	};

	struct AnimationData
	{
		string boneName;
		UINT boneId;
		vector<KeyFrame> keyFrames;
	};

public:
	Animation();
	Animation(string name, float duration, float ticksPerSecond);
	~Animation() = default;

public:
	float GetDuration() const { return _duration; }

	float GetTicksPerSecond() const { return _ticksPerSecond; }

	void AddAnimationData(const AnimationData animation);
	vector<AnimationData>& GetAnimationDatas() { return _animationDatas; }
	vector<AnimationData>* GetAnimationDatasPtr() { return &_animationDatas; }

	KeyFrame Interpolate(int boneIdx, float tick, int& lastIdx);

private:
	float _duration;
	float _ticksPerSecond;
	vector<AnimationData> _animationDatas;
};

