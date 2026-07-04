#include "pch.h"
#include "Animator.h"

// REGISTER_COMPONENT(Animator);

Animator::Animator() : Component(ComponentType::Animator)
{
	_isPlayOnInit = true;
	_isPlaying = true;
	_isLoop = true;
	_isCurrentAnimationEnd = false;

	_currentAnimation = EMPTY_ANIMATION;
	_nextAnimation = EMPTY_ANIMATION;
}

Animator::~Animator()
{
#ifdef PRINT_DEBUG_CONSOLE_LOG
	cout << "Released - Component:Animator:" << _id << "\n";
#endif
}

void Animator::Init()
{
	_isPlaying = _isPlayOnInit;
	if (!_boneFileName.empty()) {
		SetBone(_boneFileName);
	}

	ANIMATION->AddAnimator(static_pointer_cast<Animator>(shared_from_this()));
}

void Animator::Update()
{
#ifdef BULB_EDITOR
	if (!EDITOR->IsOnPlay()) return;
#endif

	if (_isPreviewMode)
	{
		if (_previewAnimation == nullptr || !_isPreviewPlaying)
			return;

		UpdateBoneTransform();
		_skeleton->UpdateUploadBuffer();

		float ticksPerSecond = (GetPreviewAnimation()->GetTicksPerSecond() != 0.0f) ? GetPreviewAnimation()->GetTicksPerSecond() : 25.0f;
		_previewTick += TIME->DeltaTime() * ticksPerSecond;

		if (_previewTick > GetPreviewAnimation()->GetDuration()) {
			_previewTick = 0.0f;
		}
	}

	else
	{
		if (_currentAnimation == EMPTY_ANIMATION || !_isPlaying)
			return;

		UpdateBoneTransform();
		_skeleton->UpdateUploadBuffer();
		UpdateAnimationEvent();

		if (_isInTransition) {
			_transitionElapsedTime += TIME->DeltaTime();
			if (_transitionElapsedTime >= _transitionTime) {
				_isInTransition = false;
				_isCurrentAnimationEnd = false;
				_currentAnimation = _nextAnimation;
				_nextAnimation = EMPTY_ANIMATION;
				_currentTick = _transitionTick;
				_transitionTick = 0.0f;
				_transitionElapsedTime = 0.0f;
				_currentAnimationSpeed = _nextAnimationSpeed;

				_nextAnimationEventIndex = 0;
			}
			else {
				float ticksPerSecond = _animations[_nextAnimation]->GetTicksPerSecond();
				_transitionTick += TIME->DeltaTime() * ticksPerSecond * _nextAnimationSpeed;
			}
		}

		float ticksPerSecond = (GetCurrentAnimation()->GetTicksPerSecond() != 0.0f) ? GetCurrentAnimation()->GetTicksPerSecond() : 25.0f;
		_currentTick += TIME->DeltaTime() * ticksPerSecond * _currentAnimationSpeed;

		if (_currentTick > GetCurrentAnimation()->GetDuration()) {
			if (_isLoop)
			{
				_currentTick = 0.0f;
				_currentAnimationEventIndex = 0;
			}
			else
			{
				_isCurrentAnimationEnd = true;
				_currentTick = GetCurrentAnimation()->GetDuration();
			}
		}
		else if (_isCurrentAnimationEnd) {
			// 여기 뭐 집어넣어야되는데 뭐 넣어야되지
		}
		else {
			_isCurrentAnimationEnd = false;
		}
	}
}

bool Animator::ShowComponentEditorGUI()
{
	if (ImGui::CollapsingHeader("Animator", ImGuiTreeNodeFlags_DefaultOpen)) {
		static string addAnimPath;

		if (ImGui::Button("PreviewMode Toggle")) {
			SetPreviewMode(!_isPreviewMode);
		}

		if (_isPreviewMode) {
			ImGui::SeparatorText("Preview Mode");
			ImGui::Text("Name:");
			ImGui::SameLine();
			ImGui::Text(_previewAnimation != nullptr ? _previewAnimation->GetName().c_str() : "None");

			ImGui::SeparatorText("AnimationTick");
			float duration = _previewAnimation != nullptr ? _previewAnimation->GetDuration() : 0.0f;
			if (ImGui::SliderFloat("##TickSlider", &_previewTick, 0.0f, duration, "%.1f")) {
				SetPreviewPlaying(false);
			}
			if (ImGui::Button(_isPreviewPlaying ? "Pause" : "Play")) {
				SetPreviewPlaying(!_isPreviewPlaying);
			}

			ImGui::InputText("##PreviewAnimationName", &addAnimPath);
			if (ImGui::Button("Set Animation")) {
				auto animation = RESOURCE->LoadAnimation(addAnimPath);
				if (animation == nullptr) {
					DEBUG->ErrorLog("Failed to load animation");
					return false;
				}
				SetPreviewAnimation(animation);
				addAnimPath = "";
			}
		}
		else
		{
			ImGui::SeparatorText("Current Animation");
			ImGui::Text(_animations[_currentAnimation] != nullptr ? _animations[_currentAnimation]->GetName().c_str() : "None");
			ImGui::SeparatorText("Animation Tick");
			if (_animations[_currentAnimation] != nullptr)
				ImGui::Text("%.1f / %.1f", _currentTick, _animations[_currentAnimation]->GetDuration());
			else
				ImGui::Text("-- / --");

			ImGui::SeparatorText("Animation List");
			ImGui::InputText("##AddAnimationName", &addAnimPath);
			if (ImGui::Button("Add Animation")) {
				auto animation = RESOURCE->LoadAnimation(addAnimPath);
				if (animation == nullptr) {
					DEBUG->ErrorLog("Failed to load animation");
					return false;
				}
				AddAnimation(animation);
				addAnimPath = "";
			}

			vector<string> removeQueue;
			for (auto& a : _animations) {
				ImGui::Text(a.first.c_str());
				ImGui::SameLine();
				string buttonLabel = "-##" + a.second->GetPath();
				if (ImGui::Button(buttonLabel.c_str())) {
					removeQueue.push_back(a.first);
				}
			}
			if (removeQueue.size() > 0) {
				for (const string& name : removeQueue)
					RemoveAnimation(name);
			}
		}
	}

	return false;
}

void Animator::OnDestroy()
{
#ifdef PRINT_DEBUG_CONSOLE_LOG
	cout << "OnDestroy - Animator:" << _id << "\n";
#endif

	for (auto anim : _animations)
		anim.second.reset();

	if (_rootBone != nullptr)
		_rootBone.reset();
	if (_skeleton != nullptr)
		_skeleton.reset();
	if (_previewAnimation != nullptr)
		_previewAnimation.reset();

	ANIMATION->DeleteAnimator(static_pointer_cast<Animator>(shared_from_this()));
}

void Animator::LoadXML(Bulb::XMLElement compElem)
{
	const char* boneFile = compElem.Attribute("Bone");
	if (boneFile != 0) _boneFileName = boneFile;

	Bulb::XMLElement animsElem = compElem.FirstChildElement("Animations");

	Bulb::XMLElement animElem = animsElem.FirstChildElement("Animation");
	while (!animElem.IsNullPtr()) {
		const char* path = animElem.Attribute("Path");
		if (path != 0) AddAnimation(RESOURCE->LoadAnimation(path));
		animElem = animElem.NextSiblingElement();
	}
}

void Animator::SaveXML(Bulb::XMLElement compElem)
{
	compElem.SetAttribute("ComponentType", "Animator");

	compElem.SetAttribute("Bone", _boneFileName.c_str());

	Bulb::XMLElement animsElem = compElem.InsertNewChildElement("Animations");
	for (auto& anim : _animations) {
		Bulb::XMLElement animElem = animsElem.InsertNewChildElement("Animation");
		animElem.SetAttribute("Path", anim.second->GetPath().c_str());
	}
}

shared_ptr<Component> Animator::Duplicate()
{
	shared_ptr<Animator> comp = static_pointer_cast<Animator>(ComponentFactory::Create("Animator"));

	comp->SetBone(_boneFileName);
	for (auto& anim : _animations) {
		comp->AddAnimation(anim.second);
	}

	return comp;
}

ComponentSnapshot Animator::CaptureSnapshot()
{
	ComponentSnapshot snapshot;

	snapshot.id = _id;
	snapshot.componentType = "Animator";

	snapshot.datas.push_back(_isPlayOnInit ? 1 : 0);
	snapshot.datas.push_back(_isLoop ? 1 : 0);
	snapshot.datas.push_back(_isPreviewMode ? 1 : 0);
	snapshot.datas.push_back(_isTransitionBlocked ? 1 : 0);

	snapshot.datas.push_back(_currentTick);
	snapshot.datas.push_back(_transitionTime);

	snapshot.strDatas.push_back(_currentAnimation);

	return snapshot;
}

void Animator::RestoreSnapshot(ComponentSnapshot snapshot)
{
	_isPlayOnInit = snapshot.datas[0] == 1;
	_isLoop = snapshot.datas[1] == 1;
	_isPreviewMode = snapshot.datas[2] == 1;
	_isTransitionBlocked = snapshot.datas[3] == 1;

	_currentTick = snapshot.datas[4];
	_transitionTime = snapshot.datas[5];

	_currentAnimationEventIndex = 0;
	_nextAnimationEventIndex = 0;

	_currentAnimation = snapshot.strDatas[0];
}

void Animator::PlayAnimation()
{
	if (_currentAnimation == EMPTY_ANIMATION)
	{
		cout << "Current Animation is null!" << endl;
		return;
	}

	_isPlaying = true;
	_isCurrentAnimationEnd = false;
}

void Animator::PauseAnimation()
{
	if (_currentAnimation == EMPTY_ANIMATION)
	{
		cout << "Current Animation is null!" << endl;
		return;
	}

	_isPlaying = false;
}

void Animator::UpdateBoneTransform()
{
	if (!_isPreviewMode) {
		for (vector<int> indices : _skeleton->depthSortedTransformIndices) {
			for (int idx : indices) {
				UpdateBoneTransform(idx);
			}
		}
	}

	else {
		for (vector<int> indices : _skeleton->depthSortedTransformIndices) {
			for (int idx : indices) {
				UpdateBoneTransformPreviewMode(idx);
			}
		}
	}
}

void Animator::UpdateBoneTransform(int boneIdx)
{
	Animation::KeyFrame currKeyFrame = GetCurrentAnimation()->Interpolate(boneIdx, _currentTick, _lastKeyframeIndex[boneIdx]);

	if (currKeyFrame.tick < 0.0) {
		_skeleton->UpdateBoneTransform(boneIdx);
		return;
	}

	if (_isInTransition && _animations.contains(_nextAnimation))
	{
		// 다음 애니메이션 키프레임
		int dummyIdx = 0;
		Animation::KeyFrame nextFrame = _animations[_nextAnimation]->Interpolate(boneIdx, _transitionTick, dummyIdx);

		// 가중치 계산
		float alpha = _transitionElapsedTime / _transitionTime;
		alpha = std::clamp(alpha, 0.0f, 1.0f);

		// Lerp / Slerp
		Bulb::Vector3 pos = currKeyFrame.position.Lerp(nextFrame.position, alpha);
		Bulb::Vector3 scale = currKeyFrame.scale.Lerp(nextFrame.scale, alpha);
		Bulb::Vector4 rot;
		XMStoreFloat4(&rot, XMQuaternionSlerp(XMLoadFloat4(&currKeyFrame.rotation), XMLoadFloat4(&nextFrame.rotation), alpha));

		XMMATRIX matScale = XMMatrixScaling(scale.x, scale.y, scale.z);
		XMMATRIX matRotation = XMMatrixRotationQuaternion(XMQuaternionSlerp(XMLoadFloat4(&currKeyFrame.rotation), XMLoadFloat4(&nextFrame.rotation), alpha));
		XMMATRIX matTranslation = XMMatrixTranslation(pos.x, pos.y, pos.z);

		_skeleton->UpdateBoneTransform(boneIdx, matScale * matRotation * matTranslation);
	}
	else
	{
		XMMATRIX matScale = XMMatrixScaling(currKeyFrame.scale.x, currKeyFrame.scale.y, currKeyFrame.scale.z);
		XMMATRIX matRotation = XMMatrixRotationQuaternion(XMLoadFloat4(&currKeyFrame.rotation));
		XMMATRIX matTranslation = XMMatrixTranslation(currKeyFrame.position.x, currKeyFrame.position.y, currKeyFrame.position.z);

		_skeleton->UpdateBoneTransform(boneIdx, matScale * matRotation * matTranslation);
	}
}

void Animator::UpdateBoneTransformPreviewMode(int boneIdx)
{
	Animation::KeyFrame currKeyFrame = GetPreviewAnimation()->Interpolate(boneIdx, _previewTick, _lastKeyframeIndex[boneIdx]);

	if (currKeyFrame.tick < 0.0) {
		_skeleton->UpdateBoneTransform(boneIdx);
		return;
	}

	XMMATRIX matScale = XMMatrixScaling(currKeyFrame.scale.x, currKeyFrame.scale.y, currKeyFrame.scale.z);
	XMMATRIX matRotation = XMMatrixRotationQuaternion(XMLoadFloat4(&currKeyFrame.rotation));
	XMMATRIX matTranslation = XMMatrixTranslation(currKeyFrame.position.x, currKeyFrame.position.y, currKeyFrame.position.z);

	_skeleton->UpdateBoneTransform(boneIdx, matScale * matRotation * matTranslation);
}

void Animator::SetCurrentAnimation(const string& animationName, float _transitionTime)
{
	if (_currentAnimation == EMPTY_ANIMATION || _transitionTime == 0.0f)
	{
		_currentAnimation = animationName;
		_isCurrentAnimationEnd = false;
		_currentTick = 0.0f;
		return;
	}
	_nextAnimation = animationName;
	_transitionElapsedTime = 0.0f;
	_transitionTick = 0.0f;
	_isCurrentAnimationEnd = false;
	_isInTransition = true;
}

void Animator::AddAnimation(shared_ptr<Animation> animation)
{
	_animations[animation->GetName()] = animation;
	if (_currentAnimation == EMPTY_ANIMATION) {
		_currentAnimation = animation->GetName();
	}
}

void Animator::UpdateAnimationEvent()
{
	// Current Animation Event
	if (_animationEvents.contains(_currentAnimation))
	{
		if (_currentAnimationEventIndex < _animationEvents[_currentAnimation].size()) {
			AnimationEvent currentEvent = _animationEvents[_currentAnimation][_currentAnimationEventIndex];
			if (currentEvent.Tick <= _currentTick) {
				if (currentEvent.type == AnimationEventTypes::Speed) {
					_currentAnimationSpeed = currentEvent.datas[0].x;
				}
				if (currentEvent.type == AnimationEventTypes::Attack) {
					animationEvent.Execute(currentEvent);
				}
				if (currentEvent.type == AnimationEventTypes::End) {
					_isCurrentAnimationEnd = true;
				}
				if (currentEvent.type == AnimationEventTypes::BlockTransition) {
					_isTransitionBlocked = currentEvent.datas[0].x == 1;
				}

				_currentAnimationEventIndex++;
			}
		}
	}
	else
	{
		_currentAnimationSpeed = 1.0f;
		_currentAnimationEventIndex = 0;
		_isTransitionBlocked = false;
	}

	// Next Animation Event (On Transition)
	if (_animationEvents.contains(_nextAnimation))
	{
		if (_nextAnimationEventIndex < _animationEvents[_nextAnimation].size()) {
			AnimationEvent nextEvent = _animationEvents[_nextAnimation][_nextAnimationEventIndex];
			if (nextEvent.Tick <= _transitionTick) {
				if (nextEvent.type == AnimationEventTypes::Speed) {
					_nextAnimationSpeed = nextEvent.datas[0].x;
				}
				if (nextEvent.type == AnimationEventTypes::Attack) {
					animationEvent.Execute(nextEvent);
				}

				_nextAnimationEventIndex++;
			}
		}
	}
	else
	{
		_nextAnimationSpeed = 1.0f;
		_nextAnimationEventIndex = 0;
	}
}

void Animator::LoadAnimationEvents(const string& path)
{
	tinyxml2::XMLDocument doc;

	doc.LoadFile(path.c_str());

	XMLNode* node = doc.FirstChild();
	XMLElement* animation = node->FirstChildElement();
	while (true)
	{
		if (animation == nullptr)
			break;
		string animationName = animation->Attribute("Name");
		XMLElement* event = animation->FirstChildElement();
		while (true)
		{
			if (event == nullptr)
				break;
			AnimationEvent animEvent;
			animEvent.Tick = event->FloatAttribute("Tick");
			string eventName(event->Name());
			if (eventName == "Speed") {
				animEvent.type = AnimationEventTypes::Speed;

				animEvent.datas[0].x = event->FloatAttribute("Speed");
			}
			if (eventName == "Attack") {
				animEvent.type = AnimationEventTypes::Attack;

				animEvent.datas[0].x = event->FloatAttribute("OffsetX");
				animEvent.datas[0].y = event->FloatAttribute("OffsetY");
				animEvent.datas[0].z = event->FloatAttribute("OffsetZ");
				animEvent.datas[0].w = event->FloatAttribute("Damage");

				animEvent.datas[1].x = event->FloatAttribute("ScaleX", 1.0f);
				animEvent.datas[1].y = event->FloatAttribute("ScaleY", 1.0f);
				animEvent.datas[1].z = event->FloatAttribute("ScaleZ", 1.0f);
				animEvent.datas[1].w = event->BoolAttribute("IsHostile") ? 1 : 0;

				animEvent.datas[2].x = event->BoolAttribute("MeleeWeaponActivation", true) ? 1 : 0;
			}
			if (eventName == "End") {
				animEvent.type = AnimationEventTypes::End;
			}
			if (eventName == "BlockTransition") {
				animEvent.type = AnimationEventTypes::BlockTransition;
				animEvent.datas[0].x = event->BoolAttribute("Flag") ? 1 : 0;
			}
			_animationEvents[animationName].push_back(animEvent);
			event = event->NextSiblingElement();
		}

		animation = animation->NextSiblingElement();
	}
}

void Animator::SetPreviewMode(bool value)
{
	if (value)
	{
		_isPreviewMode = true;
		_isPreviewPlaying = true;
	}
	else
	{
		_isPreviewMode = false;
		_isPreviewPlaying = false;
		_previewAnimation = nullptr;
		_previewTick = 0.0f;
	}
}

void Animator::SetBone(string boneFileName)
{
	_boneFileName = boneFileName;

	UpdateBoneInstances();

	_skeleton = SKELETON->GetSkeleton(_rootBone);
	if (_skeleton == nullptr)
		_skeleton = SKELETON->LoadSkeleton(_boneFileName, _rootBone);

	_lastKeyframeIndex.clear();
	_lastKeyframeIndex.resize(_skeleton->instancedTransforms.size(), 0);
}

void Animator::UpdateBoneInstances()
{
	auto bones = RESOURCE->LoadBone(_boneFileName);
	string rootBoneName = "";
	for (auto& bone : bones) {
		if (bone.second.id == 0) {
			rootBoneName = bone.second.name;
			break;
		}
	}

	auto childs = GetTransform()->GetChilds();
	for (int i = 0; i < childs.size(); i++)
	{
		childs.insert(childs.end(), childs[i]->GetChilds().begin(), childs[i]->GetChilds().end());
	}

	for (int i = 0; i < childs.size(); i++)
	{
		if (rootBoneName == childs[i]->GetGameObject()->GetName())
			_rootBone = childs[i];
	}
}


void Animator::Attack(Bulb::Vector3 offset, Bulb::Vector3 scale, float damage, bool isHostile)
{
	shared_ptr<GameObject> attackColliderObj = GameObject::Instantiate();
	attackColliderObj->GetTransform()->SetPosition(GetTransform()->GetPosition());
	attackColliderObj->GetTransform()->SetQuaternion(GetTransform()->GetQuaternion());
	attackColliderObj->SetTag("AttackAlly");

	auto attackCollider = make_shared<Rigidbody>();
	attackCollider->SetColliderExtents(scale / 2);
	attackCollider->SetColliderOffset(offset);
	attackCollider->SetColliderTrigger(true);
	attackCollider->SetGravity(false);
	attackColliderObj->AddComponent(attackCollider);

	attackColliderObj->Delete(1.0f);
}
