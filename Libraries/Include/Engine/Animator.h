#pragma once
#include "Event.h"
#define MAX_BONE_COUNT		250
#define MAX_KEYFRAME_COUNT	    300
#define EMPTY_ANIMATION		"empty"

class Animation;
class Skeleton;

enum BULB_API AnimationEventTypes {
	Start,
	Speed,
	Attack,
	End,
	BlockTransition,
	Step,
	Sound,
	RotateToTarget,
	Evade,
	Velocity
};

struct BULB_API AnimationEvent
{
	AnimationEventTypes type;
	float Tick;
	Bulb::Vector4 datas[3];
	string strData;
};

struct BULB_API AnimationEventScriptData {
	float startTick = 0.0f;
	bool isInPlace;
	vector<AnimationEvent> events;
};

class BULB_API Animator : public Component
{
	friend class AnimationManager;
public:
	Animator();
	virtual ~Animator();

	void Init() override;
	void Update() override;

	void OnDestroy() override;

	void LoadXML(Bulb::XMLElement compElem) override;
	void SaveXML(Bulb::XMLElement compElem) override;

	shared_ptr<Component> Duplicate() override;

	ComponentSnapshot CaptureSnapshot() override;
	void RestoreSnapshot(ComponentSnapshot snapshot) override;

#ifdef BULB_EDITOR
	bool ShowComponentEditorGUI() override;
#endif

public:
	float GetCurrentTick() { return _currentTick; }

	bool IsPlayOnInit() { return _isPlayOnInit; }
	void SetPlayOnInit(bool value) { _isPlayOnInit = value; }

	bool IsPlaying() { return _isPlaying; }
	void PlayAnimation();
	void PauseAnimation();
	bool IsCurrentAnimationEnd() { return _isCurrentAnimationEnd; }
	bool IsInTransition() { return _isInTransition; }
	bool IsTransitionBlocked() { return _isTransitionBlocked; }
	void SetTransitionBlock(bool value) { _isTransitionBlocked = value; }

	bool IsLoop() { return _isLoop; }
	void SetLoop(bool loop) { 
		if (_isInTransition)
			_isLoopNextAnim = loop;
		else
			_isLoop = loop;
	}

	void UpdateBoneTransform();

	shared_ptr<Animation> GetCurrentAnimation() { return _currentAnimation != EMPTY_ANIMATION ? _animations[_currentAnimation] : nullptr; }
	void SetCurrentAnimation(const string& animationName, float transitionTime = 0.1f);
	
	const unordered_map<string, shared_ptr<Animation>>& GetAnimations() { return _animations; }
	void AddAnimation(shared_ptr<Animation> animation);
	void RemoveAnimation(shared_ptr<Animation> animation) { 
		RemoveAnimation(animation->GetName());
	}
	void RemoveAnimation(const string& animationName) { 
		if (_animations.find(animationName) != _animations.end()) {
			_animations.erase(animationName);
			if (_currentAnimation == animationName) {
				_currentAnimation = EMPTY_ANIMATION;
				_currentTick = 0.0f;
			}
		}
	}

	void UpdateAnimationEvent();
	void LoadAnimationEvents(const string& path);

	shared_ptr<Animation> GetPreviewAnimation() { return _previewAnimation; }
	void SetPreviewAnimation(shared_ptr<Animation> previewAnimation) {
		_previewAnimation = previewAnimation;
		_previewTick = 0.0f;
	}
	bool IsPreviewMode() { return _isPreviewMode; }
	void SetPreviewMode(bool value);
	bool IsPreviewPlaying() { return _isPreviewPlaying; }
	void SetPreviewPlaying(bool value) { _isPreviewPlaying = value; }
	float GetPreviewTick() { return _previewTick; }
	void SetPreviewTick(float value) { 
		_previewTick = value; 
		UpdateBoneTransform();
	}

	bool IsInPlace() { return _isInPlace; }
	void SetInPlace(bool value) { _isInPlace = value; }

	// Recently Optimized
	void SetBone(string boneFileName);
	void UpdateBoneInstances();

private:
	void UpdateBoneTransform(int boneIdx);
	void UpdateBoneTransformPreviewMode(int boneIdx);

	void RefreshEventScript();

	void Attack(Bulb::Vector3 offset, Bulb::Vector3 scale, float damage, bool isHostile);

public:
	Event<AnimationEvent> animationEvent;

private:
	bool _isPlayOnInit;
	bool _isPlaying;
	bool _isCurrentAnimationEnd;		// 콜백 방식으로 바꾸는거 고려.
	bool _isLoop;
	bool _isLoopNextAnim;
	bool _isPreviewMode = false;
	bool _isTransitionBlocked = false;
	bool _isInPlace = false;

	float _currentTick = 0.0f;
	float _transitionTick = 0.0f;
	float _transitionTime = 0.1f;
	float _transitionElapsedTime = 0.0f;
	bool _isInTransition = false;

	// 이거도 vector로 변경 고려
	unordered_map<string, shared_ptr<Animation>> _animations;

	shared_ptr<Transform> _rootBone;
	string _boneFileName;
	shared_ptr<Skeleton> _skeleton;

	string _currentAnimation;
	string _nextAnimation;

	string _animationEventPath;
	unordered_map<string, AnimationEventScriptData> _animationEventScriptData;
	float _currentAnimationSpeed = 1.0f;
	float _nextAnimationSpeed = 1.0f;
	int _currentAnimationEventIndex = 0;
	int _nextAnimationEventIndex = 0;

	// Preview Mode Stuffs
	shared_ptr<Animation> _previewAnimation;
	bool _isPreviewPlaying = false;
	float _previewTick = 0.0f;

	vector<int> _lastKeyframeIndex;
};
