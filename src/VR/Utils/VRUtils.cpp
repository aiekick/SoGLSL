// NoodlesPlate Copyright (C) 2017-2024 Stephane Cuillerdier aka Aiekick
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#ifdef USE_VR

#include <VR/Utils/VRUtils.h>


const char* VRUtils::OpenXRResultString(const XrResult& m_result)
{
	switch (m_result) {
#define ENTRY(NAME, VALUE) \
	case VALUE: return #NAME;
		XR_LIST_ENUM_XrResult(ENTRY)
#undef ENTRY
	default: return "<UNKNOWN>";
	}
}

void VRUtils::CreateProjectionFov(glm::mat4& result, GraphicsAPI graphicsApi, const XrFovf fov, const float nearZ, const float farZ)
{
	const float tanAngleLeft = tanf(fov.angleLeft);
	const float tanAngleRight = tanf(fov.angleRight);

	const float tanAngleDown = tanf(fov.angleDown);
	const float tanAngleUp = tanf(fov.angleUp);

	const float tanAngleWidth = tanAngleRight - tanAngleLeft;

	// Set to tanAngleDown - tanAngleUp for a clip space with positive Y
	// down (Vulkan). Set to tanAngleUp - tanAngleDown for a clip space with
	// positive Y up (OpenGL / D3D / Metal).
	const float tanAngleHeight =
		graphicsApi == GRAPHICS_VULKAN ? (tanAngleDown - tanAngleUp) : (tanAngleUp - tanAngleDown);

	// Set to nearZ for a [-1,1] Z clip space (OpenGL / OpenGL ES).
	// Set to zero for a [0,1] Z clip space (Vulkan / D3D / Metal).
	const float offsetZ =
		(graphicsApi == GRAPHICS_OPENGL || graphicsApi == GRAPHICS_OPENGL_ES) ? nearZ : 0;

	float *matrixArr = glm::value_ptr(result[0]);

	if (farZ <= nearZ) 
	{
		// place the far plane at infinity
		matrixArr[0] = 2 / tanAngleWidth;
		matrixArr[4] = 0;
		matrixArr[8] = (tanAngleRight + tanAngleLeft) / tanAngleWidth;
		matrixArr[12] = 0;

		matrixArr[1] = 0;
		matrixArr[5] = 2 / tanAngleHeight;
		matrixArr[9] = (tanAngleUp + tanAngleDown) / tanAngleHeight;
		matrixArr[13] = 0;

		matrixArr[2] = 0;
		matrixArr[6] = 0;
		matrixArr[10] = -1;
		matrixArr[14] = -(nearZ + offsetZ);

		matrixArr[3] = 0;
		matrixArr[7] = 0;
		matrixArr[11] = -1;
		matrixArr[15] = 0;
	}
	else 
	{
		// normal projection
		matrixArr[0] = 2 / tanAngleWidth;
		matrixArr[4] = 0;
		matrixArr[8] = (tanAngleRight + tanAngleLeft) / tanAngleWidth;
		matrixArr[12] = 0;

		matrixArr[1] = 0;
		matrixArr[5] = 2 / tanAngleHeight;
		matrixArr[9] = (tanAngleUp + tanAngleDown) / tanAngleHeight;
		matrixArr[13] = 0;

		matrixArr[2] = 0;
		matrixArr[6] = 0;
		matrixArr[10] = -(farZ + offsetZ) / (farZ - nearZ);
		matrixArr[14] = -(farZ * (nearZ + offsetZ)) / (farZ - nearZ);

		matrixArr[3] = 0;
		matrixArr[7] = 0;
		matrixArr[11] = -1;
		matrixArr[15] = 0;
	}
}

void VRUtils::CreateFromQuaternion(glm::mat4& result, const XrQuaternionf& quat)
{
	const float x2 = quat.x + quat.x;
	const float y2 = quat.y + quat.y;
	const float z2 = quat.z + quat.z;

	const float xx2 = quat.x * x2;
	const float yy2 = quat.y * y2;
	const float zz2 = quat.z * z2;

	const float yz2 = quat.y * z2;
	const float wx2 = quat.w * x2;
	const float xy2 = quat.x * y2;
	const float wz2 = quat.w * z2;
	const float xz2 = quat.x * z2;
	const float wy2 = quat.w * y2;

	float* matrixArr = glm::value_ptr(result[0]);

	matrixArr[0] = 1.0f - yy2 - zz2;
	matrixArr[1] = xy2 + wz2;
	matrixArr[2] = xz2 - wy2;
	matrixArr[3] = 0.0f;

	matrixArr[4] = xy2 - wz2;
	matrixArr[5] = 1.0f - xx2 - zz2;
	matrixArr[6] = yz2 + wx2;
	matrixArr[7] = 0.0f;

	matrixArr[8] = xz2 + wy2;
	matrixArr[9] = yz2 - wx2;
	matrixArr[10] = 1.0f - xx2 - yy2;
	matrixArr[11] = 0.0f;

	matrixArr[12] = 0.0f;
	matrixArr[13] = 0.0f;
	matrixArr[14] = 0.0f;
	matrixArr[15] = 1.0f;
}

void VRUtils::CreateTranslation(glm::mat4& result, const float& x, const float& y, const float& z)
{
	float* matrixArr = glm::value_ptr(result[0]);

	matrixArr[0] = 1.0f;
	matrixArr[1] = 0.0f;
	matrixArr[2] = 0.0f;
	matrixArr[3] = 0.0f;
	matrixArr[4] = 0.0f;
	matrixArr[5] = 1.0f;
	matrixArr[6] = 0.0f;
	matrixArr[7] = 0.0f;
	matrixArr[8] = 0.0f;
	matrixArr[9] = 0.0f;
	matrixArr[10] = 1.0f;
	matrixArr[11] = 0.0f;
	matrixArr[12] = x;
	matrixArr[13] = y;
	matrixArr[14] = z;
	matrixArr[15] = 1.0f;
}

void VRUtils::CreateFromQuaternion(ct::fvec3& result_dir, float* result_angle, XrQuaternionf quat)
{
	//http://www.euclideanspace.com/maths/geometry/rotations/conversions/quaternionToAngle/index.htm
	//z = a + bi + cj + dk

	if (quat.w > 1.0f)
	{
		// normalize
		float n = sqrt(quat.x * quat.x + quat.y * quat.y + quat.z * quat.z + quat.w * quat.w);
		if (n > DBL_EPSILON)
		{
			quat.x /= n;
			quat.y /= n;
			quat.z /= n;
			quat.w /= n;
		}
	}

	if (result_angle)
		*result_angle = 2.0f * acosf(quat.w);

	const float dw = sqrt(1.0f - quat.w * quat.w);
	if (dw < FLT_EPSILON)
	{
		result_dir.x = quat.x;
		result_dir.y = quat.y;
		result_dir.z = quat.z;
	}
	else
	{
		const float ddw = 1.0f / dw;
		result_dir.x = quat.x * dw;
		result_dir.y = quat.y * dw;
		result_dir.z = quat.z * dw;
	}
	
	result_dir.normalize();
}

void VRUtils::CreateFromQuaternion(float& result_x, float& result_y, float& result_z, float* result_angle, const XrQuaternionf& quat)
{
	ct::fvec3 vec;
	CreateFromQuaternion(vec, result_angle, quat);
	result_x = vec.x;
	result_y = vec.y;
	result_z = vec.z;
}

void VRUtils::Multiply(glm::mat4& result, const glm::mat4& a, const glm::mat4& b)
{
	float* matrixArr = glm::value_ptr(result[0]);
	const float* aArr = glm::value_ptr(a[0]);
	const float* bArr = glm::value_ptr(b[0]);

	matrixArr[0] = aArr[0] * bArr[0] + aArr[4] * bArr[1] + aArr[8] * bArr[2] + aArr[12] * bArr[3];
	matrixArr[1] = aArr[1] * bArr[0] + aArr[5] * bArr[1] + aArr[9] * bArr[2] + aArr[13] * bArr[3];
	matrixArr[2] = aArr[2] * bArr[0] + aArr[6] * bArr[1] + aArr[10] * bArr[2] + aArr[14] * bArr[3];
	matrixArr[3] = aArr[3] * bArr[0] + aArr[7] * bArr[1] + aArr[11] * bArr[2] + aArr[15] * bArr[3];

	matrixArr[4] = aArr[0] * bArr[4] + aArr[4] * bArr[5] + aArr[8] * bArr[6] + aArr[12] * bArr[7];
	matrixArr[5] = aArr[1] * bArr[4] + aArr[5] * bArr[5] + aArr[9] * bArr[6] + aArr[13] * bArr[7];
	matrixArr[6] = aArr[2] * bArr[4] + aArr[6] * bArr[5] + aArr[10] * bArr[6] + aArr[14] * bArr[7];
	matrixArr[7] = aArr[3] * bArr[4] + aArr[7] * bArr[5] + aArr[11] * bArr[6] + aArr[15] * bArr[7];

	matrixArr[8] = aArr[0] * bArr[8] + aArr[4] * bArr[9] + aArr[8] * bArr[10] + aArr[12] * bArr[11];
	matrixArr[9] = aArr[1] * bArr[8] + aArr[5] * bArr[9] + aArr[9] * bArr[10] + aArr[13] * bArr[11];
	matrixArr[10] = aArr[2] * bArr[8] + aArr[6] * bArr[9] + aArr[10] * bArr[10] + aArr[14] * bArr[11];
	matrixArr[11] = aArr[3] * bArr[8] + aArr[7] * bArr[9] + aArr[11] * bArr[10] + aArr[15] * bArr[11];

	matrixArr[12] =
		aArr[0] * bArr[12] + aArr[4] * bArr[13] + aArr[8] * bArr[14] + aArr[12] * bArr[15];
	matrixArr[13] =
		aArr[1] * bArr[12] + aArr[5] * bArr[13] + aArr[9] * bArr[14] + aArr[13] * bArr[15];
	matrixArr[14] =
		aArr[2] * bArr[12] + aArr[6] * bArr[13] + aArr[10] * bArr[14] + aArr[14] * bArr[15];
	matrixArr[15] =
		aArr[3] * bArr[12] + aArr[7] * bArr[13] + aArr[11] * bArr[14] + aArr[15] * bArr[15];
}

void VRUtils::Invert(glm::mat4& result, const glm::mat4& src)
{
	float* matrixArr = glm::value_ptr(result[0]);
	const float* srcArr = glm::value_ptr(src[0]);

	matrixArr[0] = srcArr[0];
	matrixArr[1] = srcArr[4];
	matrixArr[2] = srcArr[8];
	matrixArr[3] = 0.0f;
	matrixArr[4] = srcArr[1];
	matrixArr[5] = srcArr[5];
	matrixArr[6] = srcArr[9];
	matrixArr[7] = 0.0f;
	matrixArr[8] = srcArr[2];
	matrixArr[9] = srcArr[6];
	matrixArr[10] = srcArr[10];
	matrixArr[11] = 0.0f;
	matrixArr[12] = -(srcArr[0] * srcArr[12] + srcArr[1] * srcArr[13] + srcArr[2] * srcArr[14]);
	matrixArr[13] = -(srcArr[4] * srcArr[12] + srcArr[5] * srcArr[13] + srcArr[6] * srcArr[14]);
	matrixArr[14] = -(srcArr[8] * srcArr[12] + srcArr[9] * srcArr[13] + srcArr[10] * srcArr[14]);
	matrixArr[15] = 1.0f;
}

void VRUtils::CreateViewMatrix(glm::mat4& result, const XrVector3f& translation, const XrQuaternionf& rotation)
{
	glm::mat4 rotationMatrix;
	glm::mat4 translationMatrix;
	glm::mat4 viewMatrix;

	CreateFromQuaternion(rotationMatrix, rotation);
	CreateTranslation(translationMatrix, translation.x, translation.y, translation.z);
	Multiply(viewMatrix, translationMatrix, rotationMatrix);
	Invert(result, viewMatrix);
}

void VRUtils::CreateScale(glm::mat4& result, const float& x, const float& y, const float& z)
{
	float* matrixArr = glm::value_ptr(result[0]);

	matrixArr[0] = x;
	matrixArr[1] = 0.0f;
	matrixArr[2] = 0.0f;
	matrixArr[3] = 0.0f;
	matrixArr[4] = 0.0f;
	matrixArr[5] = y;
	matrixArr[6] = 0.0f;
	matrixArr[7] = 0.0f;
	matrixArr[8] = 0.0f;
	matrixArr[9] = 0.0f;
	matrixArr[10] = z;
	matrixArr[11] = 0.0f;
	matrixArr[12] = 0.0f;
	matrixArr[13] = 0.0f;
	matrixArr[14] = 0.0f;
	matrixArr[15] = 1.0f;
}

void VRUtils::CreateModelMatrix(glm::mat4& result, const XrVector3f& translation, const XrQuaternionf& rotation, const XrVector3f& scale)
{
	float* matrixArr = glm::value_ptr(result[0]);

	glm::mat4 scaleMatrix;
	glm::mat4 rotationMatrix;
	glm::mat4 translationMatrix;
	glm::mat4 combinedMatrix;

	CreateScale(scaleMatrix, scale.x, scale.y, scale.z);
	CreateFromQuaternion(rotationMatrix, rotation);
	CreateTranslation(translationMatrix, translation.x, translation.y,	translation.z);
	Multiply(combinedMatrix, rotationMatrix, scaleMatrix);
	Multiply(result, translationMatrix, combinedMatrix);
}

#endif // USE_VR