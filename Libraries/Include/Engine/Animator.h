#pragma once
#include "Event.h"
#define MAX_BONE_COUNT		250
#define MAX_KEYFRAME_COUNT	    300
#define EMPTY_ANIMATION		"empty"

class Animation;
class Skeleton;

enum BULB_API AnimationEventTypes {
	Speed,
	Attack,
	End,
	BlockTransition
};

// 일단은 애니메이션 속도 조절만
// 현재는 애니메이션을 잘라서 사용하는 경우가 없기 때문에 tick을 0부터 시작한다고 가정
// 하지만 추후에 애니메이션 앞부분을 잘라서 사용하면 이벤트 처리에 프레임당 딜레이가 생기는 잠재적인 문제가 있음
// 크런치 이후에 수정이 반드시 필요함
struct BULB_API AnimationEvent
{
	AnimationEventTypes type;
	float Tick;
	Bulb::Vector4 datas[3];
};

class BULB_API Animator : public Component
{
	friend class AnimationManager;
public:
	Animator();
	virtual ~Animator();

	void Init() override;
	void Update() override;

#ifdef BULB_EDITOR
	virtual bool ShowComponentEditorGUI() override;
#endif

	void OnDestroy() override;

	void LoadXML(Bulb::XMLElement compElem) override;
	void SaveXML(Bulb::XMLElement compElem) override;

	shared_ptr<Component> Duplicate() override;

	ComponentSnapshot CaptureSnapshot() override;
	void RestoreSnapshot(ComponentSnapshot snapshot) override;

public:
	float GetCurrentTick() { return _currentTick; }

	bool IsPlayOnInit() { return _isPlayOnInit; }
	void SetPlayOnInit(bool value) { _isPlayOnInit = value; }

	bool IsPlaying() { return _isPlaying; }
	void PlayAnimation();
	void PauseAnimation();
	bool IsCurrentAnimationEnd() { return _isCurrentAnimationEnd; }
	bool IsTransitionBlocked() { return _isTransitionBlocked; }

	bool IsLoop() { return _isLoop; }
	void SetLoop(bool loop) { _isLoop = loop; }

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

	// Recently Optimized
	void SetBone(string boneFileName);
	void UpdateBoneInstances();

private:
	void UpdateBoneTransform(int boneIdx);
	void UpdateBoneTransformPreviewMode(int boneIdx);

	void Attack(Bulb::Vector3 offset, Bulb::Vector3 scale, float damage, bool isHostile);

public:
	Event<AnimationEvent> animationEvent;

private:
	bool _isPlayOnInit;
	bool _isPlaying;
	bool _isCurrentAnimationEnd;		// 콜백 방식으로 바꾸는거 고려.
	bool _isLoop;
	bool _isPreviewMode = false;
	bool _isTransitionBlocked = false;

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

	unordered_map<string, vector<AnimationEvent>> _animationEvents;
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
