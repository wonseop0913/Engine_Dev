#include "pch.h"
#include "DirectionalLight.h"

// REGISTER_COMPONENT(DirectionalLight);

DirectionalLight::DirectionalLight() : Super()
{

}

DirectionalLight::DirectionalLight(XMFLOAT4 diffuse) : Super(diffuse)
{

}

DirectionalLight::~DirectionalLight()
{
#ifdef PRINT_DEBUG_CONSOLE_LOG
	cout << "Released - DirectionalLight:" << _id << "\n";
#endif

	_transform.reset();
}

void DirectionalLight::Init()
{
	_lightType = LightType::Directional;

	_matProj = MathHelper::Identity4x4();

	_transform = GetTransform();
	direction = _transform->GetLook();

	RENDER->AddLight(static_pointer_cast<Light>(shared_from_this()));
}

void DirectionalLight::Update()
{
	if (_gameObject.lock()->GetFramesDirty() > 0 || Camera::GetCurrentCamera()->GetFramesDirty() > 0) {
		direction = _transform->GetLook();

		XMVECTOR eyePos = XMLoadFloat3(&Camera::GetCurrentCamera()->GetEyePos());
		XMVECTOR targetPos = eyePos + XMLoadFloat3(&direction);
		XMVECTOR upVec = XMLoadFloat3(&_transform->GetUp());

		XMMATRIX matView = XMMatrixLookAtLH(eyePos, targetPos, upVec);
		XMStoreFloat4x4(&_matView, XMMatrixTranspose(matView));

		// projMat 갱신 부분 추가해야함
		// ㄴ굳이 갱신 해야하나? 잘 모르겠음
		XMMATRIX matProj = XMMatrixOrthographicLH(
			100.0f,
			100.0f,
			-100.0f,
			100.0f
		);
		XMStoreFloat4x4(&_matProj, XMMatrixTranspose(matProj));

		SetFramesDirty();
	}
}

bool DirectionalLight::ShowComponentEditorGUI()
{
	if (ImGui::CollapsingHeader("Light", ImGuiTreeNodeFlags_DefaultOpen))
	{
		static float inputs[4];
		inputs[0] = diffuse.r;
		inputs[1] = diffuse.g;
		inputs[2] = diffuse.b;
		inputs[3] = diffuse.a;

		ImGui::Text("Light Color");
		if (ImGui::InputFloat4("##DiffuseLight", inputs)) {
			diffuse.r = clamp(inputs[0], 0.0f, 1.0f);
			diffuse.g = clamp(inputs[1], 0.0f, 1.0f);
			diffuse.b = clamp(inputs[2], 0.0f, 1.0f);
			diffuse.a = clamp(inputs[3], 0.0f, 1.0f);
		}

		ImGui::InputFloat("##LightIntensity", &intensity);

		ImGui::SeparatorText("Directional Light");
	}

	return false;
}

void DirectionalLight::OnDestroy()
{
#ifdef PRINT_DEBUG_CONSOLE_LOG
	cout << "OnDestroy - DirectionalLight:" << _id << "\n";
#endif

	RENDER->DeleteLight(static_pointer_cast<Light>(shared_from_this()));

	_transform.reset();
}

void DirectionalLight::LoadXML(Bulb::XMLElement compElem)
{
	intensity = compElem.FloatAttribute("Intensity", 1.0f);

	Bulb::XMLElement diffuseElem = compElem.FirstChildElement("Diffuse");
	if (!diffuseElem.IsNullPtr()) {
		Bulb::Color color;
		color.r = diffuseElem.FloatAttribute("r");
		color.g = diffuseElem.FloatAttribute("g");
		color.b = diffuseElem.FloatAttribute("b");
		color.a = diffuseElem.FloatAttribute("a");
		diffuse = color;
	}
}

void DirectionalLight::SaveXML(Bulb::XMLElement compElem)
{
	compElem.SetAttribute("ComponentType", "DirectionalLight");

	compElem.SetAttribute("Intensity", intensity);

	Bulb::XMLElement diffuseElem = compElem.InsertNewChildElement("Diffuse");
	diffuseElem.SetAttribute("r", diffuse.r);
	diffuseElem.SetAttribute("g", diffuse.g);
	diffuseElem.SetAttribute("b", diffuse.b);
	diffuseElem.SetAttribute("a", diffuse.a);
}

shared_ptr<Component> DirectionalLight::Duplicate()
{
	shared_ptr<DirectionalLight> comp = static_pointer_cast<DirectionalLight>(ComponentFactory::Create("DirectionalLight"));

	comp->intensity = intensity;
	comp->diffuse = diffuse;

	return comp;
}

ComponentSnapshot DirectionalLight::CaptureSnapshot()
{
	ComponentSnapshot snapshot;

	snapshot.id = _id;
	snapshot.componentType = "DirectionalLight";

	return snapshot;
}

void DirectionalLight::RestoreSnapshot(ComponentSnapshot snapshot)
{

}

LightConstants DirectionalLight::GetLightConstants()
{
	LightConstants constants;
	constants.LightType = DIRECT_LIGHT;
	constants.View = _matView;
	constants.Proj = _matProj;
	constants.Diffuse = diffuse;
	constants.Direction = direction;
	constants.intensity = intensity;

	return constants;
}
