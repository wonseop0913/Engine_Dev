#include "pch.h"
#include "PointLight.h"

PointLight::PointLight() : Super()
{

}

PointLight::PointLight(XMFLOAT4 diffuse) : Super(diffuse)
{

}

PointLight::~PointLight()
{
#ifdef PRINT_DEBUG_CONSOLE_LOG
	cout << "Released - PointLight:" << _id << "\n";
#endif

	_transform.reset();
}

void PointLight::Init()
{
	_lightType = LightType::Point;

	_matProj = MathHelper::Identity4x4();

	_transform = GetTransform();

	RENDER->AddLight(static_pointer_cast<Light>(shared_from_this()));
}

void PointLight::Update()
{
	if (_gameObject.lock()->GetFramesDirty() > 0) {
		_position = _transform->GetPosition();
	}
}

bool PointLight::ShowComponentEditorGUI()
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

		static float fallOffValues[2];
		fallOffValues[0] = _fallOffStart;
		fallOffValues[1] = _fallOffEnd;

		ImGui::Text("FallOff Value");
		if (ImGui::InputFloat2("##PointLightFallOffValue", fallOffValues)) {
			_fallOffStart = fallOffValues[0];
			_fallOffEnd = fallOffValues[1];
		}
	}

	return false;
}

void PointLight::OnDestroy()
{
#ifdef PRINT_DEBUG_CONSOLE_LOG
	cout << "OnDestroy - DirectionalLight:" << _id << "\n";
#endif

	RENDER->DeleteLight(static_pointer_cast<Light>(shared_from_this()));

	_transform.reset();
}

void PointLight::LoadXML(Bulb::XMLElement compElem)
{
	intensity = compElem.FloatAttribute("Intensity", 1.0f);

	_fallOffStart = compElem.FloatAttribute("FallOffStart");
	_fallOffEnd = compElem.FloatAttribute("FallOffEnd");

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

void PointLight::SaveXML(Bulb::XMLElement compElem)
{
	compElem.SetAttribute("ComponentType", "PointLight");

	compElem.SetAttribute("Intensity", intensity);

	compElem.SetAttribute("FallOffStart", _fallOffStart);
	compElem.SetAttribute("FallOffEnd", _fallOffEnd);

	Bulb::XMLElement diffuseElem = compElem.InsertNewChildElement("Diffuse");
	diffuseElem.SetAttribute("r", diffuse.r);
	diffuseElem.SetAttribute("g", diffuse.g);
	diffuseElem.SetAttribute("b", diffuse.b);
	diffuseElem.SetAttribute("a", diffuse.a);
}

shared_ptr<Component> PointLight::Duplicate()
{
	shared_ptr<PointLight> comp = static_pointer_cast<PointLight>(ComponentFactory::Create("PointLight"));

	comp->intensity = intensity;
	comp->diffuse = diffuse;
	comp->SetFallOffValues(_fallOffStart, _fallOffEnd);

	return comp;
}

ComponentSnapshot PointLight::CaptureSnapshot()
{
	ComponentSnapshot snapshot;

	snapshot.id = _id;
	snapshot.componentType = "PointLight";

	return snapshot;
}

void PointLight::RestoreSnapshot(ComponentSnapshot snapshot)
{

}

LightConstants PointLight::GetLightConstants()
{
	LightConstants constants;
	constants.LightType = POINT_LIGHT;
	constants.Position = _position;
	constants.FallOffStart = _fallOffStart;
	constants.FallOffEnd = _fallOffEnd;
	constants.Diffuse = diffuse;
	constants.intensity = intensity;

	return constants;
}
