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

#pragma once

#ifdef USE_VR

#include <openxr/openxr.h>
#include <openxr/openxr_reflection.h>
#include <ctools/cTools.h>

#pragma warning(push)
#pragma warning(disable:4201)   // suppress even more warnings about nameless structs
#include <glm/glm.hpp>
#include <glm/vec3.hpp> // glm::vec3
#include <glm/vec4.hpp> // glm::vec4
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/quaternion.hpp>
#pragma warning(pop)

class VRUtils
{
public:
	typedef enum
	{
		GRAPHICS_VULKAN,
		GRAPHICS_OPENGL,
		GRAPHICS_OPENGL_ES
	} GraphicsAPI;

public:
	static const char* OpenXRResultString(const XrResult& m_result);
	static void	CreateProjectionFov(glm::mat4& result, GraphicsAPI graphicsApi, const XrFovf fov, const float nearZ, const float farZ);
	static void	CreateFromQuaternion(glm::mat4& result, const XrQuaternionf& quat);
	static void	CreateTranslation(glm::mat4& result, const float& x, const float& y, const float& z);
	static void CreateFromQuaternion(float& result_x, float& result_y, float& result_z, float* result_angle, const XrQuaternionf& quat);
	static void CreateFromQuaternion(ct::fvec3& result_dir, float* result_angle, XrQuaternionf quat);
	static void	Multiply(glm::mat4& result, const glm::mat4& a, const glm::mat4& b);
	static void	Invert(glm::mat4& result, const glm::mat4& src);
	static void	CreateViewMatrix(glm::mat4& result, const XrVector3f& translation, const XrQuaternionf& rotation);
	static void CreateScale(glm::mat4& result, const float& x, const float& y, const float& z);
	static void CreateModelMatrix(glm::mat4& result, const XrVector3f& translation, const XrQuaternionf& rotation, const XrVector3f& scale);
};

#endif // USE_VR