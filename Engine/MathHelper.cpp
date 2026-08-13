#include "pch.h"
#include "MathHelper.h"

const float MathHelper::Infinity = FLT_MAX;
const float MathHelper::Pi = 3.141592654f;

XMFLOAT4X4 MathHelper::Identity4x4()
{
	static XMFLOAT4X4 I(
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f);

	return I;
}

Bulb::Vector3 MathHelper::ConvertQuaternionToEuler(const Bulb::Vector4& quat)
{
	Bulb::Vector3 angles;

	// roll (x-axis rotation)
	double sinr_cosp = 2 * (quat.w * quat.x + quat.y * quat.z);
	double cosr_cosp = 1 - 2 * (quat.x * quat.x + quat.y * quat.y);
	angles.x = std::atan2(sinr_cosp, cosr_cosp);
	
	// 2. Pitch (y-axis rotation)
	double sinp = 2 * (quat.w * quat.y - quat.z * quat.x);
	if (std::abs(sinp) >= 1)
		angles.y = std::copysign(Pi / 2.0, sinp);
	else
		angles.y = std::asin(sinp);

	//double sinp = std::sqrt(1 + 2 * (quat.w * quat.y - quat.x * quat.z));
	//double cosp = std::sqrt(1 - 2 * (quat.w * quat.y - quat.x * quat.z));
	//angles.y = 2 * std::atan2(sinp, cosp) - Pi / 2;

	// yaw (z-axis rotation)
	double siny_cosp = 2 * (quat.w * quat.z + quat.x * quat.y);
	double cosy_cosp = 1 - 2 * (quat.y * quat.y + quat.z * quat.z);
	angles.z = std::atan2(siny_cosp, cosy_cosp);

	return angles;
}

Bulb::Vector3 MathHelper::ConvertQuaternionToEuler(const XMVECTOR& quat)
{
	Bulb::Vector4 quatConvert;
	XMStoreFloat4(&quatConvert, quat);
	return ConvertQuaternionToEuler(quatConvert);
}

Bulb::Vector3 MathHelper::RadianToDegree(const Bulb::Vector3& radian)
{
	return radian * (180.0f / Pi);
}

Bulb::Vector3 MathHelper::DegreeToRadian(const Bulb::Vector3& degree)
{
	return degree * (Pi / 180.0f);
}

float MathHelper::CCW(const Bulb::Vector2& va, const Bulb::Vector2& vb)
{
	return va.x * vb.y - va.y * vb.x;
}

Bulb::Vector3 MathHelper::InterpolateVector(Bulb::Vector3 currentForward, Bulb::Vector3 targetDir, float rotationSpeed)
{
	XMVECTOR current = XMVector3Normalize(XMLoadFloat3(&currentForward));
	XMVECTOR target = XMVector3Normalize(XMLoadFloat3(&targetDir));

	XMVECTOR dotVec = XMVector3Dot(current, target);
	float dot = XMMin(1.0f, XMMax(-1.0f, XMVectorGetX(dotVec)));

	if (dot > 0.9999f) return target;

	float angle = acos(dot);
	XMVECTOR axis = XMVector3Cross(current, target);

	if (XMVectorGetX(XMVector3LengthSq(axis)) < 0.0001f)
	{
		axis = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

		XMVECTOR crossUp = XMVector3Cross(current, axis);
		if (XMVectorGetX(XMVector3LengthSq(crossUp)) < 0.0001f)
		{
			axis = XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f);
		}
	}
	else
	{
		axis = XMVector3Normalize(axis);
	}

	float maxRotationAngle = rotationSpeed * TIME->DeltaTime();

	if (angle <= maxRotationAngle)
	{
		return target;
	}

	XMMATRIX rotationMatrix = XMMatrixRotationAxis(axis, maxRotationAngle);
	XMVECTOR rotatedVector = XMVector3TransformNormal(current, rotationMatrix);

	return Bulb::Vector3(XMVector3Normalize(rotatedVector));
}
