// NoodlesPlate Copyright (C) 2017-2023 Stephane Cuillerdier aka Aiekick
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

#include <ctools/cTools.h>
#include <ctools/ConfigAbstract.h>
#include "Interfaces/CameraInterface.h"

#pragma warning(push)
#pragma warning(disable:4201)   // suppress even more warnings about nameless structs
#include <glm/glm.hpp>
#include <glm/vec3.hpp> // glm::vec3
#include <glm/vec4.hpp> // glm::vec4
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/intersect.hpp>
#include <glm/gtx/transform.hpp>
#pragma warning(pop)

enum class CAMERA_TYPE_Enum : uint8_t
{
	CAMERA_TYPE_PERSPECTIVE = 0,
	CAMERA_TYPE_ORTHOGRAPHIC,
	CAMERA_TYPE_Count
};

enum class CAMERA_MODE_Enum : uint8_t
{
	CAMERA_MODE_TURNTABLE_Y = 0,
	CAMERA_MODE_FREE,
	CAMERA_MODE_Count
};

class ShaderKey;
class CameraSystem : public CameraInterface, public conf::ConfigAbstract
{
public: // params to be serialized
	glm::vec2 camSize;
	glm::vec2 nearFarPlanes = glm::vec2(0.01f, 10000.0f);
	glm::vec4 lrbtPlanes = glm::vec4(-10.0f, 10.0f, -10.0f, 10.0f);
	glm::quat camOrientation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f); // direction of the camera (free fly mode), w is in first

	struct CameraSettingsStruct
	{
		// Rot Y mode
		glm::vec2 m_TransXY = glm::vec2(0.0f);
		ct::bvec2 m_TransXYLock = true;
		float m_TransFactor = 5.0f;
		float m_Zoom = 5.0f;
		bool m_ZoomLock = false;
		float m_ZoomFactor = 5.0f;

		// flight mode
		glm::vec3 m_PosXYZ = glm::vec3(0.0f);
		float m_SpeedFactor = 0.01f;

		// Rot Y mode and flight mode
		glm::vec3 m_TargetXYZ = glm::vec3(0.0f);
		ct::bvec3 m_TargetXYZLock = true;
		glm::vec3 m_RotationXYZ = glm::vec3(0.785f, -0.612f, 0.0f);
		ct::bvec3 m_RotationXYZLock = ct::bvec3(false, false, true);
		float m_RotationFactor = 1.0f;
		bool m_UniformScaleLock = true;
		float m_UniformScale = 1.0f;

		float m_PerspAngle = 45.0f;
		
		CAMERA_TYPE_Enum m_CameraType = CAMERA_TYPE_Enum::CAMERA_TYPE_PERSPECTIVE;
		CAMERA_MODE_Enum m_CameraMode = CAMERA_MODE_Enum::CAMERA_MODE_TURNTABLE_Y;

#ifdef USE_VR
		bool m_UseVRControllerForControlCamera = false;
		glm::vec3 m_VRRotStepFactor = glm::vec3(0.01f);
		float m_VRZoomStepFactor = 1.0f;
#endif

	} m_CameraSettings;
	
public:
	bool m_NeedCamChange = true;

public:
	static CameraSystem* Instance()
	{
		static CameraSystem _instance;
		return &_instance;
	}
	
protected:
	CameraSystem() = default; // Prevent construction
	CameraSystem(const CameraSystem&) = default; // Prevent construction by copying
	CameraSystem& operator =(const CameraSystem&) { return *this; }; // Prevent assignment
	~CameraSystem() = default; // Prevent unwanted destruction

public:
	void SetCameraMode(const CAMERA_MODE_Enum& vCameraMode);
	void SetCameraType(const CAMERA_TYPE_Enum& vCameraType);
	
	void SetPerspectiveAngle(const float& vAngle);
	void SetPerspectiveLimitZ(
		const float& vNearPlane, 
		const float& vFarPlane);
	void SetOrthographicLimitZ(
		const float& vLeftPlane, 
		const float& vRightPlane, 
		const float& vBottomPlane, 
		const float& vTopPlane);
	
	void SetTargetXYZ(const ct::fvec3& vTarget);

	void IncTranslateXY(const ct::fvec2& vMouseOffset);
	void SetTranslateXY(const ct::fvec2& vTranlation);
	void SetTranslateFactor(const float& vTranslateFactor);

	void IncRotateXYZ(const ct::fvec3& vMouseOffset);
	void SetRotateXYZ(const ct::fvec3& vRotate);
	void SetRotateFactor(const float& vRotateFactor);
	
	void IncZoom(const float& vMouseOffset); 
	void SetZoom(const float& vZoom);
	void SetZoomFactor(const float& vZoomFactor);

	void IncFlyingPosition(const float& vMovementOffset);

	const CameraSettingsStruct& GetCameraSettings() const { return m_CameraSettings; }
	void SetViewMatrix(const glm::mat4& vMatrix);
	void SetProjMatrix(const glm::mat4& vMatrix);
	void NeedCamChange();
	bool DrawImGui();
	bool ForceUpdate(const ct::uvec2& vScreenSize);
	bool UpdateIfNeeded(const ct::uvec2& vScreenSize);

	///////////////////////////////////////////////////////
	///////////////////////////////////////////////////////
	///////////////////////////////////////////////////////
	
	std::string toStrFromGLMVec2(const glm::vec2& v, char delimiter=';');
	glm::vec2 toGLMVec2FromStr(const std::string& vStr, char delimiter = ';');
	std::string toStrFromGLMVec3(const glm::vec3& v, char delimiter = ';');
	glm::vec3 toGLMVec3FromStr(const std::string& vStr, char delimiter = ';');
	std::string toStrFromGLMVec4(const glm::vec4& v, char delimiter = ';');
	glm::vec4 toGLMVec4FromStr(const std::string& vStr, char delimiter = ';');

	///////////////////////////////////////////////////////
	//// CONFIGURATION ////////////////////////////////////
	///////////////////////////////////////////////////////

	std::string getXml(const std::string& vOffset, const std::string& vUserDatas) override;
	bool setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas) override;

private:
	bool ComputeCameras(const ct::uvec2& vScreenSize);
	glm::mat4 ComputeCameraMatrix(const ct::uvec2& vScreenSize);
	glm::mat4 ComputeCameraMatrix();
	glm::mat4 ComputeProjectionMatrix(const ct::uvec2& vScreenSize);
	glm::mat4 ComputeModelMatrix();
	glm::mat4 ComputeNormalMatrix();
	glm::mat4 ComputeInvCameraMatrix();
};

