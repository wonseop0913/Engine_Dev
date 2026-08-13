#pragma once

class BULB_API MathHelper
{
public:
	static const float Infinity;
	static const float Pi;

public:
	static XMFLOAT4X4 Identity4x4();

	static Bulb::Vector3 ConvertQuaternionToEuler(const Bulb::Vector4& quat);
	static Bulb::Vector3 ConvertQuaternionToEuler(const XMVECTOR& quat);

	static Bulb::Vector3 RadianToDegree(const Bulb::Vector3& radian);
	static Bulb::Vector3 DegreeToRadian(const Bulb::Vector3& degree);

	static float CCW(const Bulb::Vector2& va, const Bulb::Vector2& vb);

	static Bulb::Vector3 InterpolateVector(
		Bulb::Vector3 currentForward,
		Bulb::Vector3 targetDir,
		float rotationSpeed);
};

