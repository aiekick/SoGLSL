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

// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include <VR/Backend/VRBackend.h>

#ifdef USE_VR

#include <ctools/Logger.h>
#include <VR/Utils/VRUtils.h>
#include <Gui/GuiBackend.h>
#include <Systems/CameraSystem.h>
#include <Profiler/TracyProfiler.h>
#include <ImGuiPack.h>
#include <functional>

/////////////////////////////////////////////////////////////////////
///// CTOR/DTOR /////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////

VRBackend::VRBackend()
{

}

VRBackend::~VRBackend()
{

}

/////////////////////////////////////////////////////////////////////
///// OVERRIDES /////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////

bool VRBackend::Init(CodeTreePtr vCodeTree)
{
	UNUSED(vCodeTree);

	m_Loaded = true;

	Unit();

	if (CreateXRInstance()) {
		if (CreateXRSystem()) {
			if (CreateXRSession()) {
				if (CreateXRViews()) {
					if (CreateXRSwapChains()) {
						if (CreateXRRefSpace()) {
							if (m_Actions.CreateXRActions(m_xr_instance, m_xr_session, m_BasePose)) {
								if (CreateFrameBuffers()) {
									m_Loaded = true;
								}
							}
						}
					}
				}
			}
		}
	}
	
	if (!m_Loaded)
	{
		Unit();
	}

	return m_Loaded;
}

void VRBackend::Unit()
{
	if (m_xr_instance && m_xr_session)
	{
		m_result = xrRequestExitSession(m_xr_session);
		if (XR_FAILED(m_result))
		{
            LogVarDebugError("%s", VRUtils::OpenXRResultString(m_result));
		}
	}

	DestroyFrameBuffers();
	DestroyXRRefSpace();
	m_Actions.DestroyXRActions();
	DestroyXRSwapChains();
	DestroyXRViews();
	DestroyXRSession();
	DestroyXRSystem();
	DestroyXRInstance();

	ClearVars();

	m_Loaded = false;
}

void VRBackend::ClearVars()
{
	m_xr_instance = {};
	m_xr_instance_err = nullptr;
	m_xr_session = {};
	m_xr_session_err = nullptr;
	m_xr_system_id = {};
	m_xr_system_err = nullptr;

	pfnGetOpenGLGraphicsRequirementsKHR = nullptr;

	// frame
	m_FrameState = { XR_TYPE_FRAME_STATE, nullptr };
	m_FrameWaitInfo = { XR_TYPE_FRAME_WAIT_INFO, nullptr };

	// space
	m_PlaySpaceType = XR_REFERENCE_SPACE_TYPE_LOCAL;
	m_PlaySpace = XR_NULL_HANDLE;
	m_BasePose = {};

	// view
	m_ViewType = XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO;
	m_ViewCount = 0U;
	m_ViewCurrentIndex = 0U;
	m_ViewConfigViews.clear();
	m_ProjectionViews.clear();
	m_Views.clear();

	// poll events
	m_xr_session_state = XR_SESSION_STATE_UNKNOWN;
	m_NeedToQuitMainLoop = false;
	m_SessionIsRunning = false;
	m_RunFrameCycle = false;

	// transforms
	m_ViewState = {};

	m_OpenXRInfo = {};

	// SwapChains
	m_SwapChains.clear();
	m_SwapChainsSizes.clear();
	m_SwapChainsImages.clear();
	m_DepthInfos.clear();
	m_DepthSwapChains.clear();
	m_DepthSwapChainsSizes.clear();
	m_DepthSwapChainsImages.clear();
	m_AcquiredSwapChainIndex = UINT32_MAX;
	m_AcquiredSwapChainIndexDepth = UINT32_MAX;
	m_FrameBuffers.clear();
	m_InRendering = false;

	m_Actions.clear();
}

bool VRBackend::DrawWidget(CodeTreePtr vCodeTree, UniformVariantPtr vUniPtr,
	const float& vMaxWidth, const float& vFirstColumnWidth, RenderPackWeak vRenderPack,
	const bool& vShowUnUsed, const bool& vShowCustom, const bool& vForNodes, bool* vChange)
{
	bool catched = false;

	if (vUniPtr && vCodeTree && vChange)
	{
		ShaderKeyPtr key = nullptr;
		auto rpPtr = vRenderPack.lock();
		if (rpPtr)
			key = rpPtr->GetShaderKey();

		if (vUniPtr->widget == "vr:use")
		{
			vCodeTree->DrawUniformName(key, vUniPtr, 0);
			ImGui::SameLine(vFirstColumnWidth);
			ImGui::Text("use : %s", (vUniPtr->x > 0.5f) ? "true" : "false");

			catched = true;
		}
		else if (
			vUniPtr->widget == "vr:pos"||
			vUniPtr->widget == "vr:dir")
		{
			vCodeTree->DrawUniformName(key, vUniPtr, 0);
			ImGui::SameLine(vFirstColumnWidth);
			ImGui::Text("x:%.3f, y:%.3f, z:%.3f", vUniPtr->x, vUniPtr->y, vUniPtr->z);

			catched = true;
		}
		else if (
			vUniPtr->widget == "vr:thumb:left"||
			vUniPtr->widget == "vr:thumb:right"||
			vUniPtr->widget == "vr:thumb:accum:left"||
			vUniPtr->widget == "vr:thumb:accum:right")
		{
			vCodeTree->DrawUniformName(key, vUniPtr, 0);
			ImGui::SameLine(vFirstColumnWidth);
			ImGui::Text("x:%.3f, y:%.3f", vUniPtr->x, vUniPtr->y);

			catched = true;
		}
		else if (
			vUniPtr->widget == "vr:trigger:left" ||
			vUniPtr->widget == "vr:trigger:right" ||
			vUniPtr->widget == "vr:squeeze:left" ||
			vUniPtr->widget == "vr:squeeze:right")
		{
			vCodeTree->DrawUniformName(key, vUniPtr, 0);
			ImGui::SameLine(vFirstColumnWidth);
			ImGui::Text("x:%.3f", vUniPtr->x);

			catched = true;
		}
	}

	return catched;
}

bool VRBackend::SerializeUniform(UniformVariantPtr vUniform, std::string* vStr)
{
	UNUSED(vUniform);
	UNUSED(vStr);

	return false;
}

bool VRBackend::DeSerializeUniform(
	ShaderKeyPtr vShaderKey, 
	UniformVariantPtr vUniform, 
	const std::vector<std::string>& vParams)
{
	UNUSED(vShaderKey);
	UNUSED(vUniform);
	UNUSED(vParams);

	return false;
}

void VRBackend::Complete_Uniform(
	ShaderKeyPtr vParentKey, 
	RenderPackWeak vRenderPack, 
	const UniformParsedStruct& vUniformParsed, 
	UniformVariantPtr vUniform)
{
    if (!vRenderPack.expired())
		return;

	bool catched = false;

	vUniform->widget = vUniformParsed.params;
	vUniform->canWeSave = false;
	vUniform->timeLineSupported = false;

#define checkifexist(a) (vUniformParsed.paramsDico.find(a) != vUniformParsed.paramsDico.end())

	if (vUniform->glslType == uType::uTypeEnum::U_FLOAT)
	{
		//vr:use
		//vr:trigger:left
		//vr:trigger:right
		//vr:squeeze:left
		//vr:squeeze:right

		if (checkifexist("vr"))
		{
			if (checkifexist("use"))
			{
				vUniform->widget = "vr:use";
				catched = true;
			}
			else if (checkifexist("trigger"))
			{
				if (checkifexist("left"))
				{
					vUniform->widget = "vr:trigger:left";
					catched = true;
				}
				else if (checkifexist("right"))
				{
					vUniform->widget = "vr:trigger:right";
					catched = true;
				}
			}
			else if (checkifexist("squeeze"))
			{
				if (checkifexist("left"))
				{
					vUniform->widget = "vr:squeeze:left";
					catched = true;
				}
				else if (checkifexist("right"))
				{
					vUniform->widget = "vr:squeeze:right";
					catched = true;
				}
			}
		}
	}
	else if (vUniform->glslType == uType::uTypeEnum::U_INT)
	{
		//vr:use

		if (checkifexist("vr"))
		{
			if (checkifexist("use"))
			{
				vUniform->widget = "vr:use";
				catched = true;
			}
		}
	}
	else if (vUniform->glslType == uType::uTypeEnum::U_UINT)
	{
		//vr:use

		if (checkifexist("vr"))
		{
			if (checkifexist("use"))
			{
				vUniform->widget = "vr:use";
				catched = true;
			}
		}
	}
	else if (vUniform->glslType == uType::uTypeEnum::U_BOOL)
	{
		//vr:use

		if (checkifexist("vr"))
		{
			if (checkifexist("use"))
			{
				vUniform->widget = "vr:use";
				catched = true;
			}
		}
	}
	else if (vUniform->glslType == uType::uTypeEnum::U_VEC3)
	{
		//vr:pos
		//vr:dir

		if (checkifexist("vr"))
		{
			if (checkifexist("pos"))
			{
				vUniform->widget = "vr:pos";
				catched = true;
			}
			else if (checkifexist("dir"))
			{
				vUniform->widget = "vr:dir";
				catched = true;
			}
		}
	}
	else if (vUniform->glslType == uType::uTypeEnum::U_VEC2)
	{
		//vr:thumb:left
		//vr:thumb:right
		//vr:thumb:accum:left
		//vr:thumb:accum:right
		//vr:thumb:left:factor=0.1
		//vr:thumb:right:factor=0.1
		//vr:thumb:accum:left:factor=0.1
		//vr:thumb:accum:right:factor=0.1

		if (checkifexist("vr") && checkifexist("thumb"))
		{
			vUniform->step.x = 1.0f;
			vUniform->step.y = 1.0f;
			
			if (checkifexist("accum"))
			{
				if(checkifexist("factor"))
				{
					if (vUniformParsed.paramsDico.at("factor").size() > 0U)
					{
						vUniform->step.x = ct::fvariant(vUniformParsed.paramsDico.at("factor")[0]).GetF();
						vUniform->step.y = vUniform->step.x;
					}
					if (vUniformParsed.paramsDico.at("factor").size() > 1U)
					{
						vUniform->step.y = ct::fvariant(vUniformParsed.paramsDico.at("factor")[1]).GetF();
					}
				}
				
				if (checkifexist("left"))
				{
					vUniform->widget = "vr:thumb:accum:left";
					catched = true;
				}
				else if (checkifexist("right"))
				{
					vUniform->widget = "vr:thumb:accum:right";
					catched = true;
				}
			}
			else
			{
				if (checkifexist("left"))
				{
					vUniform->widget = "vr:thumb:left";
					catched = true;
				}
				else if (checkifexist("right"))
				{
					vUniform->widget = "vr:thumb:right";
					catched = true;
				}
			}
		}
	}

#undef checkifexist

	if (!catched)
	{
		vParentKey->GetSyntaxErrors()->SetSyntaxError(vParentKey, "Parsing Error :", "Bad Uniform Type", false,
			LineFileErrors(vUniformParsed.sourceCodeLine, "", ct::toStr("uniform of type %s is not supported for the moment", vUniform->widget.c_str())));
	}
}

bool VRBackend::IsCameraMoving(const uint32_t vEye) const
{
	if (CameraSystem::Instance()->m_CameraSettings.m_UseVRControllerForControlCamera)
	{
		const auto& datas = m_Actions.GetOpenXRControllerDatas();
		if (datas.size() > vEye)
		{
			if (datas[vEye].squeezeValue.currentState > 0.5f)
			{
				return true;
			}
		}
	}
	
	return false;
}

bool VRBackend::UpdateUniforms(UniformVariantPtr vUniPtr)
{
	if (vUniPtr)
	{
        auto& vrActionsRef = VRBackend::Instance()->GetXRActionsRef();
		if (vUniPtr->widget == "vr:use")
		{
			if (m_InRendering)
			{
				switch (vUniPtr->glslType)
				{
				case uType::uTypeEnum::U_FLOAT:
					vUniPtr->x = 1.0f;
					return true;
				case uType::uTypeEnum::U_INT:
					vUniPtr->ix = 1;
					return true;
				case uType::uTypeEnum::U_UINT:
					vUniPtr->ux = 1U;
					return true;
				case uType::uTypeEnum::U_BOOL:
					vUniPtr->bx = true;
					return true;
				default:
					break;
				}
			}
			else
			{
				switch (vUniPtr->glslType)
				{
				case uType::uTypeEnum::U_FLOAT:
					vUniPtr->x = 0.0f;
					return true;
				case uType::uTypeEnum::U_INT:
					vUniPtr->ix = 0;
					return true;
				case uType::uTypeEnum::U_UINT:
					vUniPtr->ux = 0U;
					return true;
				case uType::uTypeEnum::U_BOOL:
					vUniPtr->bx = false;
					return true;
				default:
					break;
				}
			}		
		}
		else if (vUniPtr->glslType == uType::uTypeEnum::U_FLOAT)
		{
			//"vr:trigger:left" 
			//"vr:trigger:right" 
			//"vr:squeeze:left" 
			//"vr:squeeze:right"

			if (vUniPtr->widget == "vr:trigger:left")
			{
				if (m_InRendering)
				{
                const auto& xr_controllerDatas = vrActionsRef.GetOpenXRControllerDatas();
					if (!xr_controllerDatas.empty())
					{
						vUniPtr->x = xr_controllerDatas[0].triggerValue.currentState;
					}

					return true;
				}
			}
			else if (vUniPtr->widget == "vr:trigger:right")
			{
				if (m_InRendering)
				{
					const auto& xr_controllerDatas = vrActionsRef.GetOpenXRControllerDatas();
					if (xr_controllerDatas.size() > 1U)
					{
						vUniPtr->x = xr_controllerDatas[1].triggerValue.currentState;
					}

					return true;
				}
			}
			else if (vUniPtr->widget == "vr:squeeze:left")
			{
				if (m_InRendering)
				{
					const auto& xr_controllerDatas = vrActionsRef.GetOpenXRControllerDatas();
					if (!xr_controllerDatas.empty())
					{
						vUniPtr->x = xr_controllerDatas[0].squeezeValue.currentState;
					}

					return true;
				}
			}
			else if (vUniPtr->widget == "vr:squeeze:right")
			{
				if (m_InRendering)
				{
					const auto& xr_controllerDatas = vrActionsRef.GetOpenXRControllerDatas();
					if (xr_controllerDatas.size() > 1U)
					{
						vUniPtr->x = xr_controllerDatas[0].squeezeValue.currentState;
					}

					return true;
				}
			}

		}
		else if (vUniPtr->glslType == uType::uTypeEnum::U_VEC3)
		{
			if (vUniPtr->widget == "vr:pos")
			{
				if (m_InRendering && (uint32_t)m_Views.size() > m_ViewCurrentIndex)
				{
					auto pos = m_Views[m_ViewCurrentIndex].pose.position;
					vUniPtr->x = pos.x;
					vUniPtr->y = pos.y;
					vUniPtr->z = pos.z;

					return true;
				}
			}
			else if (vUniPtr->widget == "vr:dir")
			{
				if (m_InRendering && (uint32_t)m_Views.size() > m_ViewCurrentIndex)
				{
					const auto& quat = m_Views[m_ViewCurrentIndex].pose.orientation;
					vUniPtr->x = quat.x;
					vUniPtr->y = quat.y;
					vUniPtr->z = quat.z;
					return true;
				}
			}
		}
		else if (vUniPtr->glslType == uType::uTypeEnum::U_VEC2)
		{
			if (vUniPtr->widget == "vr:thumb:left")
			{
				if (m_InRendering)
				{
					if (!IsCameraMoving(0))
					{
						const auto& xr_controllerDatas = vrActionsRef.GetOpenXRControllerDatas();
						if (!xr_controllerDatas.empty())
						{
							vUniPtr->x = xr_controllerDatas[0].thumbstickXValue.currentState * vUniPtr->step.x;
							vUniPtr->y = xr_controllerDatas[0].thumbstickYValue.currentState * vUniPtr->step.y;
						}

						return true;
					}
				}
			}
			else if (vUniPtr->widget == "vr:thumb:right")
			{
				if (m_InRendering)
				{
					if (!IsCameraMoving(1))
					{
						const auto& xr_controllerDatas = vrActionsRef.GetOpenXRControllerDatas();
						if (xr_controllerDatas.size() > 1U)
						{
							vUniPtr->x = xr_controllerDatas[1].thumbstickXValue.currentState * vUniPtr->step.x;
							vUniPtr->y = xr_controllerDatas[1].thumbstickYValue.currentState * vUniPtr->step.y;
						}

						return true;
					}
				}
			}
			else if (vUniPtr->widget == "vr:thumb:accum:left")
			{
				if (m_InRendering)
				{
					if (!IsCameraMoving(0))
					{
						const auto& xr_controllerDatas = vrActionsRef.GetOpenXRControllerDatas();
						if (!xr_controllerDatas.empty())
						{
							vUniPtr->x += xr_controllerDatas[0].thumbstickXValue.currentState * vUniPtr->step.x;
							vUniPtr->y += xr_controllerDatas[0].thumbstickYValue.currentState * vUniPtr->step.y;
						}

						return true;
					}
				}
			}
			else if (vUniPtr->widget == "vr:thumb:accum:right")
			{
				if (m_InRendering)
				{
					if (!IsCameraMoving(1))
					{
                        const auto& xr_controllerDatas = vrActionsRef.GetOpenXRControllerDatas();
						if (xr_controllerDatas.size() > 1U)
						{
							vUniPtr->x += xr_controllerDatas[1].thumbstickXValue.currentState * vUniPtr->step.x;
							vUniPtr->y += xr_controllerDatas[1].thumbstickYValue.currentState * vUniPtr->step.y;
						}

						return true;
					}
				}
			}
		}
		else if (vUniPtr->glslType == uType::uTypeEnum::U_MAT4)
		{
			if (m_InRendering)
			{
				if (vUniPtr->widget == "camera:mvp")
				{
					vUniPtr->uFloatArr = glm::value_ptr(uCam[0]);
					vUniPtr->ownFloatArr = false;

					return true;
				}
				else if (vUniPtr->widget == "camera:imvp")
				{
					vUniPtr->uFloatArr = glm::value_ptr(uInvCam[0]);
					vUniPtr->ownFloatArr = false;

					return true;
				}
				else if (vUniPtr->widget == "camera:m")
				{
					vUniPtr->uFloatArr = glm::value_ptr(uModel[0]);
					vUniPtr->ownFloatArr = false;

					return true;
				}
				else if (vUniPtr->widget == "camera:v")
				{
					vUniPtr->uFloatArr = glm::value_ptr(uView[0]);
					vUniPtr->ownFloatArr = false;

					return true;
				}
				else if (vUniPtr->widget == "camera:p")
				{
					vUniPtr->uFloatArr = glm::value_ptr(uProj[0]);
					vUniPtr->ownFloatArr = false;

					return true;
				}
				else if (vUniPtr->widget == "camera:nm")
				{
					vUniPtr->uFloatArr = glm::value_ptr(uNormalMatrix[0]);
					vUniPtr->ownFloatArr = false;

					return true;
				}
			}
		}
	}

	return false;
}

/////////////////////////////////////////////////////////////////////
///// PUBLIC ////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////

bool VRBackend::IsLoaded()
{
	return m_Loaded;
}


bool VRBackend::NewFrame()
{
	m_InRendering = false;

	if (m_Loaded)
	{
		PollXREvents();

		if (m_RunFrameCycle)
		{
			return true;
		}
	}

	return false;
}

void VRBackend::EndFrame()
{

}

const OpenXRInfoStruct& VRBackend::GetOpenXRInfo() const
{
	return m_OpenXRInfo;
}

const std::vector<XrView>& VRBackend::GetOpenXRViews() const
{
	return m_Views;
}

VRActions& VRBackend::GetXRActionsRef() 
{
	return m_Actions;
}

/////////////////////////////////////////////////////////////////////
///// PRIVATE ///////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////

bool VRBackend::CreateXRInstance()
{
	if (m_xr_instance != XR_NULL_HANDLE || m_xr_instance_err != nullptr)
	{
		CTOOL_DEBUG_BREAK;
		return false;
	}

	std::vector<const char*> exts = {};
	exts.push_back(XR_KHR_OPENGL_ENABLE_EXTENSION_NAME);

	XrInstanceCreateInfo create_info = { XR_TYPE_INSTANCE_CREATE_INFO };
	create_info.enabledExtensionCount = (uint32_t)exts.size();
	create_info.enabledExtensionNames = exts.data();
	create_info.enabledApiLayerCount = 0;
	create_info.enabledApiLayerNames = nullptr;
	create_info.applicationInfo.applicationVersion = 1;
	create_info.applicationInfo.engineVersion = 1;
	create_info.applicationInfo.apiVersion = XR_CURRENT_API_VERSION;
	snprintf(create_info.applicationInfo.applicationName, sizeof(create_info.applicationInfo.applicationName), "%s", "NDP");
	snprintf(create_info.applicationInfo.engineName, sizeof(create_info.applicationInfo.engineName), "NDP");

	m_result = xrCreateInstance(&create_info, &m_xr_instance);
	if (XR_FAILED(m_result))
	{
		m_xr_instance_err = VRUtils::OpenXRResultString(m_result);
		LogVarError("%s", m_xr_instance_err);
		//CTOOL_DEBUG_BREAK;
		return false;
	}

	m_result = xrGetInstanceProcAddr(m_xr_instance, "xrGetOpenGLGraphicsRequirementsKHR",
			(PFN_xrVoidFunction*)&pfnGetOpenGLGraphicsRequirementsKHR);
	if (XR_FAILED(m_result))
	{
		LogVarError("%s", VRUtils::OpenXRResultString(m_result));
		CTOOL_DEBUG_BREAK;
	}

	return true;
}

void VRBackend::DestroyXRInstance()
{
	if (m_xr_instance)
		xrDestroyInstance(m_xr_instance);

	m_xr_instance = XR_NULL_HANDLE;
	m_xr_instance_err = nullptr;
}

bool VRBackend::CreateXRSystem()
{
	if (m_xr_instance_err != nullptr)
	{
		m_xr_system_err = "XrInstance not available!";
		LogVarError("%s", m_xr_system_err);
		CTOOL_DEBUG_BREAK;
		return false;
	}
	if (m_xr_system_id != XR_NULL_SYSTEM_ID || m_xr_system_err != nullptr)
	{
		CTOOL_DEBUG_BREAK;
		return false;
	}

	XrSystemGetInfo system_info = { XR_TYPE_SYSTEM_GET_INFO };
	system_info.formFactor = XR_FORM_FACTOR_HEAD_MOUNTED_DISPLAY;
	m_result = xrGetSystem(m_xr_instance, &system_info, &m_xr_system_id);
	if (XR_FAILED(m_result))
	{
		m_xr_system_err = VRUtils::OpenXRResultString(m_result);
		LogVarError("%s", m_xr_system_err);

		system_info.formFactor = XR_FORM_FACTOR_HANDHELD_DISPLAY;
		m_result = xrGetSystem(m_xr_instance, &system_info, &m_xr_system_id);
		if (XR_FAILED(m_result))
		{
			m_xr_system_err = VRUtils::OpenXRResultString(m_result);
			LogVarError("%s", m_xr_system_err);
			return false;
		}
	}

	return true;
}

void VRBackend::DestroyXRSystem()
{
	m_xr_system_id = XR_NULL_SYSTEM_ID;
	m_xr_system_err = nullptr;
}

bool VRBackend::CreateXRSession()
{
	if (m_xr_instance_err != nullptr)
	{
		m_xr_session_err = "XrInstance not available!";
		LogVarError("%s", m_xr_session_err);
		CTOOL_DEBUG_BREAK;
		return false;
	}

	if (m_xr_system_err != nullptr)
	{
		LogVarError("XrSystem not available!");
		CTOOL_DEBUG_BREAK;
		return false;
	}

	XrGraphicsRequirementsOpenGLKHR opengl_reqs = { XR_TYPE_GRAPHICS_REQUIREMENTS_OPENGL_KHR, nullptr };
	m_result = pfnGetOpenGLGraphicsRequirementsKHR(m_xr_instance, m_xr_system_id, &opengl_reqs);
	if (XR_FAILED(m_result))
	{
		m_xr_session_err = VRUtils::OpenXRResultString(m_result);
		LogVarError("%s", m_xr_session_err);
		CTOOL_DEBUG_BREAK;
		return false;
	}

	XrGraphicsBindingOpenGLWin32KHR gfx_binding = { XR_TYPE_GRAPHICS_BINDING_OPENGL_WIN32_KHR };
	gfx_binding.hDC = (HDC)wglGetCurrentDC();
	gfx_binding.hGLRC = (HGLRC)wglGetCurrentContext();

	XrSessionCreateInfo session_info = { XR_TYPE_SESSION_CREATE_INFO };
	session_info.next = &gfx_binding;
	session_info.systemId = m_xr_system_id;
	m_result = xrCreateSession(m_xr_instance, &session_info, &m_xr_session);
	if (XR_FAILED(m_result))
	{
		m_xr_session_err = VRUtils::OpenXRResultString(m_result);
		LogVarError("%s", m_xr_session_err);
		CTOOL_DEBUG_BREAK;
		return false;
	}

	RetrieveOpenXRInfos();

	return true;
}

void VRBackend::DestroyXRSession()
{
	if (m_xr_session)
		xrDestroySession(m_xr_session);

	m_xr_session = XR_NULL_HANDLE;
	m_xr_session_err = nullptr;
}

bool VRBackend::CreateXRViews()
{
	m_result = xrEnumerateViewConfigurationViews(m_xr_instance, m_xr_system_id, m_ViewType, 0, &m_ViewCount, nullptr);
	if (XR_FAILED(m_result))
	{
		auto err = VRUtils::OpenXRResultString(m_result);
		LogVarError("Failed to get view configuration view count : %s", err);
		CTOOL_DEBUG_BREAK;
		return false;
	}

	m_ViewConfigViews.resize(m_ViewCount);
	for (auto& configView : m_ViewConfigViews)
	{
		configView.type = XR_TYPE_VIEW_CONFIGURATION_VIEW;
		configView.next = nullptr;
	}

	m_result = xrEnumerateViewConfigurationViews(m_xr_instance, m_xr_system_id, m_ViewType, m_ViewCount,
		&m_ViewCount, m_ViewConfigViews.data());
	if (XR_FAILED(m_result))
	{
		auto err = VRUtils::OpenXRResultString(m_result);
		LogVarError("Failed to enumerate view configuration views : %s", err);
		CTOOL_DEBUG_BREAK;
		return false;
	}

	m_ViewSize.x = m_ViewConfigViews[0].recommendedImageRectWidth;
	m_ViewSize.y = m_ViewConfigViews[0].recommendedImageRectHeight;

	return true;
}

void VRBackend::DestroyXRViews()
{

}

bool VRBackend::CreateXRRefSpace()
{
	m_BasePose.orientation.x = 0.0f;
	m_BasePose.orientation.y = 0.0f;
	m_BasePose.orientation.z = 0.0f;
	m_BasePose.orientation.w = 1.0f;

	m_BasePose.position.x = 0.0f;
	m_BasePose.position.y = 0.0f;
	m_BasePose.position.z = 0.0f;
	//m_BasePose.position.z = -10.0f; // poyr le zoom cam

	m_PlaySpaceType = XR_REFERENCE_SPACE_TYPE_LOCAL; //seat scale
	//m_PlaySpaceType = XR_REFERENCE_SPACE_TYPE_STAGE; //room scale
	XrReferenceSpaceCreateInfo play_space_create_info;
	play_space_create_info.type = XR_TYPE_REFERENCE_SPACE_CREATE_INFO;
	play_space_create_info.next = nullptr;
	play_space_create_info.referenceSpaceType = m_PlaySpaceType;
	play_space_create_info.poseInReferenceSpace = m_BasePose;

	const auto& m_result = xrCreateReferenceSpace(m_xr_session, &play_space_create_info, &m_PlaySpace);
	if (XR_FAILED(m_result))
	{
		auto err = VRUtils::OpenXRResultString(m_result);
		LogVarError("xrCreateReferenceSpace() fail to create play space : %s", err);
		CTOOL_DEBUG_BREAK;
		return false;
	}

	return true;
}

void VRBackend::DestroyXRRefSpace()
{
	xrDestroySpace(m_PlaySpace);
}


// returns the preferred swapchain format if it is supported
// else:
// - if fallback is true, return the first supported format
// - if fallback is false, return -1
int64_t VRBackend::GetSwapchainFormat(XrInstance m_xr_instance, XrSession session, int64_t preferred_format, bool fallback)
{
	m_result;

	uint32_t swapchain_format_count;
	m_result = xrEnumerateSwapchainFormats(session, 0, &swapchain_format_count, nullptr);
	if (XR_FAILED(m_result))
	{
		auto err = VRUtils::OpenXRResultString(m_result);
		LogVarError("xrEnumerateSwapchainFormats() fail to get number of supported swapchain formats : %s", err);
		CTOOL_DEBUG_BREAK;
		return -1;
	}

	//printf("Runtime supports %d swapchain formats\n", swapchain_format_count);
	std::vector<int64_t> swapchain_formats;
	swapchain_formats.resize(swapchain_format_count);
	m_result = xrEnumerateSwapchainFormats(session, swapchain_format_count, &swapchain_format_count, swapchain_formats.data());
	if (XR_FAILED(m_result))
	{
		auto err = VRUtils::OpenXRResultString(m_result);
		LogVarError("xrEnumerateSwapchainFormats() fail to enumerate swapchain formats : %s", err);
		CTOOL_DEBUG_BREAK;
		return -1;
	}

	int64_t chosen_format = fallback ? swapchain_formats[0] : -1;

	for (uint32_t i = 0; i < swapchain_format_count; i++)
	{
		//printf("Supported GL format: %#lx\n", swapchain_formats[i]);
		if (swapchain_formats[i] == preferred_format)
		{
			chosen_format = swapchain_formats[i];
			//printf("Using preferred swapchain format %#lx\n", chosen_format);
			break;
		}
	}
	if (fallback && chosen_format != preferred_format)
	{
		//printf("Falling back to non preferred swapchain format %#lx\n", chosen_format);
	}

	swapchain_formats.clear();

	return chosen_format;
}

bool VRBackend::CreateXRSwapChains()
{
	// SRGB is usually a better choice than linear
	// a more sophisticated approach would iterate supported swapchain formats and choose from them
	int64_t color_format = GetSwapchainFormat(m_xr_instance, m_xr_session, GL_SRGB8_ALPHA8_EXT, true);

	// GL_DEPTH_COMPONENT16 is a good bet
	// SteamVR 1.16.4 supports GL_DEPTH_COMPONENT16, GL_DEPTH_COMPONENT24, GL_DEPTH_COMPONENT32
	// but NOT GL_DEPTH_COMPONENT32F
	int64_t depth_format = GetSwapchainFormat(m_xr_instance, m_xr_session, GL_DEPTH_COMPONENT16, false);
	if (depth_format < 0)
	{
		printf("Preferred depth format GL_DEPTH_COMPONENT16 not supported, disabling depth\n");
		m_OpenXRInfo.depthSupported = false;
	}

	// In the frame loop we render into OpenGL textures we receive from the runtime here.
	m_SwapChains.clear();
	m_SwapChains.resize(m_ViewCount);
	m_SwapChainsSizes.clear();
	m_SwapChainsSizes.resize(m_ViewCount);
	m_SwapChainsImages.clear();
	m_SwapChainsImages.resize(m_ViewCount);
	for (uint32_t i = 0; i < m_ViewCount; i++)
	{
		XrSwapchainCreateInfo swapchain_create_info;
		swapchain_create_info.type = XR_TYPE_SWAPCHAIN_CREATE_INFO;
		swapchain_create_info.usageFlags = XR_SWAPCHAIN_USAGE_SAMPLED_BIT | XR_SWAPCHAIN_USAGE_COLOR_ATTACHMENT_BIT;
		swapchain_create_info.createFlags = 0;
		swapchain_create_info.format = color_format;
		swapchain_create_info.sampleCount = m_ViewConfigViews[i].recommendedSwapchainSampleCount;
		swapchain_create_info.width = m_ViewSize.x;
		swapchain_create_info.height = m_ViewSize.y;
		swapchain_create_info.faceCount = 1;
		swapchain_create_info.arraySize = 1;
		swapchain_create_info.mipCount = 1;
		swapchain_create_info.next = nullptr;

		m_result = xrCreateSwapchain(m_xr_session, &swapchain_create_info, &m_SwapChains[i]);
		if (XR_FAILED(m_result))
		{
			auto err = VRUtils::OpenXRResultString(m_result);
			LogVarError("xrCreateSwapchain() failed to create swapchain %d : %s", i, err);
			CTOOL_DEBUG_BREAK;
			return false;
		}

		// The runtime controls how many textures we have to be able to render to
		// (e.g. "triple buffering")
		m_result = xrEnumerateSwapchainImages(m_SwapChains[i], 0, &m_SwapChainsSizes[i], nullptr);
		if (XR_FAILED(m_result))
		{
			auto err = VRUtils::OpenXRResultString(m_result);
			LogVarError("xrEnumerateSwapchainImages() failed to enumerate swapchains : %s", err);
			CTOOL_DEBUG_BREAK;
			return false;
		}

		m_SwapChainsImages[i].resize(m_SwapChainsSizes[i]);
		for (uint32_t j = 0; j < m_SwapChainsSizes[i]; ++j)
		{
			m_SwapChainsImages[i][j].type = XR_TYPE_SWAPCHAIN_IMAGE_OPENGL_KHR;
			m_SwapChainsImages[i][j].next = nullptr;
		}
		m_result = xrEnumerateSwapchainImages(m_SwapChains[i], m_SwapChainsSizes[i], &m_SwapChainsSizes[i],
			(XrSwapchainImageBaseHeader*)m_SwapChainsImages[i].data());
		if (XR_FAILED(m_result))
		{
			auto err = VRUtils::OpenXRResultString(m_result);
			LogVarError("xrEnumerateSwapchainImages() failed to enumerate swapchains images : %s", err);
			CTOOL_DEBUG_BREAK;
			return false;
		}
	}

	// --- Create swapchain for depth buffers if supported
	if (m_OpenXRInfo.depthSupported)
	{
		m_DepthSwapChains.clear();
		m_DepthSwapChains.resize(m_ViewCount);
		m_DepthSwapChainsSizes.clear();
		m_DepthSwapChainsSizes.resize(m_ViewCount);
		m_DepthSwapChainsImages.clear();
		m_DepthSwapChainsImages.resize(m_ViewCount);
		for (uint32_t i = 0; i < m_ViewCount; i++)
		{
			XrSwapchainCreateInfo swapchain_create_info;
			swapchain_create_info.type = XR_TYPE_SWAPCHAIN_CREATE_INFO;
			swapchain_create_info.usageFlags = XR_SWAPCHAIN_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
			swapchain_create_info.createFlags = 0;
			swapchain_create_info.format = depth_format;
			swapchain_create_info.sampleCount = m_ViewConfigViews[i].recommendedSwapchainSampleCount;
			swapchain_create_info.width = m_ViewConfigViews[i].recommendedImageRectWidth;
			swapchain_create_info.height = m_ViewConfigViews[i].recommendedImageRectHeight;
			swapchain_create_info.faceCount = 1;
			swapchain_create_info.arraySize = 1;
			swapchain_create_info.mipCount = 1;
			swapchain_create_info.next = nullptr;

			m_result = xrCreateSwapchain(m_xr_session, &swapchain_create_info, &m_DepthSwapChains[i]);
			if (XR_FAILED(m_result))
			{
				auto err = VRUtils::OpenXRResultString(m_result);
				LogVarError("xrCreateSwapchain() failed to create Depth swapchains image %i : %s", i, err);
				CTOOL_DEBUG_BREAK;
				return false;
			}

			m_result = xrEnumerateSwapchainImages(m_DepthSwapChains[i], 0, &m_DepthSwapChainsSizes[i], nullptr);
			if (XR_FAILED(m_result))
			{
				auto err = VRUtils::OpenXRResultString(m_result);
				LogVarError("xrCreateSwapchain() failed to enumerate Depth swapchains : %s", err);
				CTOOL_DEBUG_BREAK;
				return false;
			}

			// these are wrappers for the actual OpenGL texture id
			m_DepthSwapChainsImages[i].resize(m_DepthSwapChainsSizes[i]);
			for (uint32_t j = 0; j < m_DepthSwapChainsSizes[i]; j++)
			{
				m_DepthSwapChainsImages[i][j].type = XR_TYPE_SWAPCHAIN_IMAGE_OPENGL_KHR;
				m_DepthSwapChainsImages[i][j].next = nullptr;
			}
			m_result = xrEnumerateSwapchainImages(m_DepthSwapChains[i], m_DepthSwapChainsSizes[i], &m_DepthSwapChainsSizes[i],
				(XrSwapchainImageBaseHeader*)m_DepthSwapChainsImages[i].data());
			if (XR_FAILED(m_result))
			{
				auto err = VRUtils::OpenXRResultString(m_result);
				LogVarError("xrCreateSwapchain() failed to enumerate Depth swapchains images : %s", err);
				CTOOL_DEBUG_BREAK;
				return false;
			}
		}
	}

	m_Views.clear();
	m_Views.resize(m_ViewCount);
	for (auto& view : m_Views)
	{
		view.type = XR_TYPE_VIEW;
		view.next = nullptr;
	}

	uint32_t idx = 0U;
	m_ProjectionViews.clear();
	m_ProjectionViews.resize(m_ViewCount);
	for (auto& projectionView : m_ProjectionViews)
	{
		projectionView.type = XR_TYPE_COMPOSITION_LAYER_PROJECTION_VIEW;
		projectionView.next = nullptr;
		projectionView.subImage.swapchain = m_SwapChains[idx];
		projectionView.subImage.imageArrayIndex = 0;
		projectionView.subImage.imageRect.offset.x = 0;
		projectionView.subImage.imageRect.offset.y = 0;
		projectionView.subImage.imageRect.extent.width = m_ViewSize.x;
		projectionView.subImage.imageRect.extent.height = m_ViewSize.y;

		++idx;
	};

	if (m_OpenXRInfo.depthSupported)
	{
		idx = 0U;
		m_DepthInfos.clear();
		m_DepthInfos.resize(m_ViewCount);
		for (auto& depthInfos : m_DepthInfos)
		{
			depthInfos.type = XR_TYPE_COMPOSITION_LAYER_DEPTH_INFO_KHR;
			depthInfos.next = nullptr;
			depthInfos.minDepth = 0.f;
			depthInfos.maxDepth = 1.f;
			depthInfos.nearZ = 0.01f;
			depthInfos.farZ = 100.0f;
			depthInfos.subImage.swapchain = m_DepthSwapChains[idx];
			depthInfos.subImage.imageArrayIndex = 0;
			depthInfos.subImage.imageRect.offset.x = 0;
			depthInfos.subImage.imageRect.offset.y = 0;
			depthInfos.subImage.imageRect.extent.width = m_ViewSize.x;
			depthInfos.subImage.imageRect.extent.height = m_ViewSize.y;

			m_ProjectionViews[idx].next = &depthInfos;

			++idx;
		};
	}

	return true;
}

void VRBackend::DestroyXRSwapChains()
{
	for (auto& swap : m_SwapChains)
	{
		xrDestroySwapchain(swap);
	}

	if (m_OpenXRInfo.depthSupported)
	{
		for (auto& swap : m_DepthSwapChains)
		{
			xrDestroySwapchain(swap);
		}
	}
}

bool VRBackend::CreateFrameBuffers()
{
	m_FrameBuffers.resize(m_ViewCount);
	for (uint32_t i = 0U; i < m_ViewCount; ++i)
	{
		m_FrameBuffers[i].resize(m_SwapChainsSizes[i]);
		glGenFramebuffers(m_SwapChainsSizes[i], m_FrameBuffers[i].data());
	}
	
	return true;
}

void VRBackend::DestroyFrameBuffers()
{
	if (!m_FrameBuffers.empty())
	{
		for (size_t i = 0U; i < m_ViewCount; ++i)
		{
			if (m_SwapChainsSizes.size() > i && m_FrameBuffers.size() > i)
				glDeleteFramebuffers(m_SwapChainsSizes[i], m_FrameBuffers[i].data());
		}

		m_FrameBuffers.clear();
	}
}

void VRBackend::RetrieveOpenXRInfos()
{
	struct xr_properties_t {
		XrInstanceProperties							m_xr_instance;
		XrSystemProperties								system;
		XrSystemHandTrackingPropertiesEXT				hand_tracking;
		XrSystemHandTrackingMeshPropertiesMSFT			hand_mesh;
		XrSystemEyeGazeInteractionPropertiesEXT			gaze;
		XrSystemFoveatedRenderingPropertiesVARJO		foveated_varjo;
		XrSystemColorSpacePropertiesFB					color_space_fb;
	} result = {};

	if (!m_xr_instance_err) 
	{
		result.m_xr_instance = { XR_TYPE_INSTANCE_PROPERTIES };
		m_result = xrGetInstanceProperties(m_xr_instance, &result.m_xr_instance);
		if (XR_FAILED(m_result))
		{
			auto err = VRUtils::OpenXRResultString(m_result);
			LogVarError("%s", err);
			CTOOL_DEBUG_BREAK;
			return;
		}
		else 
		{
			m_OpenXRInfo.runtimeName = ct::toStr("%s", result.m_xr_instance.runtimeName);
			m_OpenXRInfo.runtimeVersion = ct::toStr("%d.%d.%d",
				(int32_t)XR_VERSION_MAJOR(result.m_xr_instance.runtimeVersion),
				(int32_t)XR_VERSION_MINOR(result.m_xr_instance.runtimeVersion),
				(int32_t)XR_VERSION_PATCH(result.m_xr_instance.runtimeVersion));
		}
	}
	else 
	{
		LogVarError("No XrInstance available");
	}

	if (!m_xr_instance_err && !m_xr_system_err) 
	{
		result.system = { XR_TYPE_SYSTEM_PROPERTIES };
		result.hand_tracking = { XR_TYPE_SYSTEM_HAND_TRACKING_PROPERTIES_EXT };
		result.hand_mesh = { XR_TYPE_SYSTEM_HAND_TRACKING_MESH_PROPERTIES_MSFT };
		result.gaze = { XR_TYPE_SYSTEM_EYE_GAZE_INTERACTION_PROPERTIES_EXT };
		result.foveated_varjo = { XR_TYPE_SYSTEM_FOVEATED_RENDERING_PROPERTIES_VARJO };
		result.color_space_fb = { XR_TYPE_SYSTEM_COLOR_SPACE_PROPERTIES_FB };

		result.system.next = &result.hand_tracking;
		result.hand_tracking.next = &result.hand_mesh;
		result.hand_mesh.next = &result.gaze;
		result.gaze.next = &result.foveated_varjo;
		result.foveated_varjo.next = &result.color_space_fb;

		XrResult error = xrGetSystemProperties(m_xr_instance, m_xr_system_id, &result.system);
		if (XR_FAILED(error)) 
		{
			auto err = VRUtils::OpenXRResultString(error);
			LogVarError("%s", err);
			CTOOL_DEBUG_BREAK;
			return;
		}
	}
	else 
	{
		if (m_xr_system_err)   LogVarError("No XrSystemId available");
		if (m_xr_instance_err) LogVarError("No XrInstance available");
	}

	m_OpenXRInfo.systemName = ct::toStr("%s", result.system.systemName);
	m_OpenXRInfo.vendorId = ct::toStr("%u", result.system.vendorId);
	m_OpenXRInfo.orientationTracking = result.system.trackingProperties.orientationTracking;
	m_OpenXRInfo.positionTracking = result.system.trackingProperties.positionTracking;
	m_OpenXRInfo.maxLayerCount = result.system.graphicsProperties.maxLayerCount;
	m_OpenXRInfo.maxSwapChainSize.x = result.system.graphicsProperties.maxSwapchainImageWidth;
	m_OpenXRInfo.maxSwapChainSize.y = result.system.graphicsProperties.maxSwapchainImageHeight;

	if (!m_xr_instance_err && !m_xr_system_err)
	{
		uint32_t ext_count = 0;
		m_result = xrEnumerateInstanceExtensionProperties(nullptr, 0, &ext_count, nullptr);
		if (XR_FAILED(m_result))
		{
			auto err = VRUtils::OpenXRResultString(m_result);
			LogVarError("Faile to enumerate number of extension properties %s", err);
			CTOOL_DEBUG_BREAK;
			return;
		}

		std::vector<XrExtensionProperties> ext_props;
		ext_props.resize(ext_count);
		for (auto& ext_prop : ext_props)
		{
			ext_prop.type = XR_TYPE_EXTENSION_PROPERTIES;
			ext_prop.next = nullptr;
		}

		m_result = xrEnumerateInstanceExtensionProperties(nullptr, ext_count, &ext_count, ext_props.data());
		if (XR_FAILED(m_result))
		{
			auto err = VRUtils::OpenXRResultString(m_result);
			LogVarError("Fail to enumerate extension properties %s", err);
			CTOOL_DEBUG_BREAK;
			return;
		}

		for (uint32_t i = 0; i < ext_count; i++) 
		{
			//printf("\t%s v%d\n", ext_props[i].extensionName, ext_props[i].extensionVersion);
			if (strcmp(XR_KHR_OPENGL_ENABLE_EXTENSION_NAME, ext_props[i].extensionName) == 0)
			{
				m_OpenXRInfo.openglSupported = true;
			}

			if (strcmp(XR_KHR_COMPOSITION_LAYER_DEPTH_EXTENSION_NAME, ext_props[i].extensionName) == 0)
			{
				m_OpenXRInfo.depthSupported = true;
			}
		}
	}
	else
	{
		if (m_xr_system_err)   LogVarError("No XrSystemId available");
		if (m_xr_instance_err) LogVarError("No XrInstance available");
	}
}

void VRBackend::PollXREvents()
{
	m_xr_session_state = XR_SESSION_STATE_UNKNOWN;
	
	// --- Controllerle runtime Events
	// we do this before xrWaitFrame() so we can go idle or
	// break out of the main render loop as early as possible and don't have to
	// uselessly render or submit one. Calling xrWaitFrame commits you to
	// calling xrBeginFrame eventually.
	XrEventDataBuffer runtime_event = { XR_TYPE_EVENT_DATA_BUFFER, nullptr };
	XrResult poll_m_result = xrPollEvent(m_xr_instance, &runtime_event);
	while (poll_m_result == XR_SUCCESS)
	{
		switch (runtime_event.type)
		{
		case XR_TYPE_EVENT_DATA_INSTANCE_LOSS_PENDING:
		{
			XrEventDataInstanceLossPending* event = (XrEventDataInstanceLossPending*)&runtime_event;
			LogVarError("XR Event : m_xr_instance loss pending at %lu! Destroying m_xr_instance.\n", event->lossTime);
			m_NeedToQuitMainLoop = true;
			continue;
		}
		case XR_TYPE_EVENT_DATA_SESSION_STATE_CHANGED:
		{
			XrEventDataSessionStateChanged* event = (XrEventDataSessionStateChanged*)&runtime_event;
			LogVarInfo("XR Event : session state changed from %s(%d) to %s(%d)\n", 
				GetSessionStateString(m_xr_session_state), m_xr_session_state,
				GetSessionStateString(event->state), event->state);
			m_xr_session_state = event->state;
			ReactToSessionState(m_xr_session_state);
			break; // session event handling switch
		}
		case XR_TYPE_EVENT_DATA_INTERACTION_PROFILE_CHANGED:
		{
			m_Actions.EventInteractionProfileChnaged(m_xr_instance, m_xr_session, runtime_event);
			break;
		}
		default: 
			LogVarError("Unhandled event (type %d)\n", runtime_event.type);
			break;
		}

		runtime_event.type = XR_TYPE_EVENT_DATA_BUFFER;
		poll_m_result = xrPollEvent(m_xr_instance, &runtime_event);
	}
	if (poll_m_result == XR_EVENT_UNAVAILABLE)
	{
		// processed all events in the queue
	}
	else
	{
		LogVarError("Failed to poll events!\n");
		CTOOL_DEBUG_BREAK;
	}
}

void VRBackend::ReactToSessionState(const XrSessionState& vState)
{
	/*
	* react to session state changes, see OpenXR spec 9.3 diagram. What we need to react to:
	*
	* * READY -> xrBeginSession STOPPING -> xrEndSession (note that the same session can be restarted)
	* * EXITING -> xrDestroySession (EXITING only happens after we went through STOPPING and called xrEndSession)
	*
	* After exiting it is still possible to create a new session but we don't do that here.
	*
	* * IDLE -> don't run render loop, but keep polling for events
	* * SYNCHRONIZED, VISIBLE, FOCUSED -> run render loop
	*/

	switch (vState)
	{
	// skip render loop, keep polling
	case XR_SESSION_STATE_MAX_ENUM: // must be a bug
	case XR_SESSION_STATE_IDLE:
	case XR_SESSION_STATE_UNKNOWN:
		m_RunFrameCycle = false;
		break; // state handling switch

	// do nothing, run render loop normally
	case XR_SESSION_STATE_FOCUSED:
	case XR_SESSION_STATE_SYNCHRONIZED:
	case XR_SESSION_STATE_VISIBLE:
		m_RunFrameCycle = true;
		break; // state handling switch

	// begin session and then run render loop
	case XR_SESSION_STATE_READY:
		// start session only if it is not running, i.e. not when we already called xrBeginSession
		// but the runtime did not switch to the next state yet
		if (!m_SessionIsRunning)
		{
			XrViewConfigurationType view_type = XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO;
			XrSessionBeginInfo session_begin_info = { XR_TYPE_SESSION_BEGIN_INFO, nullptr, view_type };
			const auto& m_result = xrBeginSession(m_xr_session, &session_begin_info);
			if (XR_FAILED(m_result))
			{
				auto err = VRUtils::OpenXRResultString(m_result);
				LogVarError("Fail to begin session : %s", err);
				CTOOL_DEBUG_BREAK;
			}
			else
			{
				m_SessionIsRunning = true;
			}
		}
		m_RunFrameCycle = true; // after beginning the session, run render loop
		break; // state handling switch
	
	// end session, skip render loop, keep polling for next state change
	case XR_SESSION_STATE_STOPPING:
		// end session only if it is running, i.e. not when we already called xrEndSession but the
		// runtime did not switch to the next state yet
		if (m_SessionIsRunning)
		{
			const auto& m_result = xrEndSession(m_xr_session);
			if (XR_FAILED(m_result))
			{
				auto err = VRUtils::OpenXRResultString(m_result);
				LogVarError("Fail to end session : %s", err);
				CTOOL_DEBUG_BREAK;
			}
			else
			{
				m_SessionIsRunning = false;
			}
		}
		m_RunFrameCycle = false; // after ending the session, don't run render loop
		break; // state handling switch
	
	// destroy session, skip render loop, exit render loop and quit
	case XR_SESSION_STATE_LOSS_PENDING:
	case XR_SESSION_STATE_EXITING:
		const auto& m_result = xrDestroySession(m_xr_session);
		if (XR_FAILED(m_result))
		{
			auto err = VRUtils::OpenXRResultString(m_result);
			LogVarError("Failed to destroy session : %s", err);
			CTOOL_DEBUG_BREAK;
		}
		else
		{
			m_NeedToQuitMainLoop = true;
			m_RunFrameCycle = false;
		}
		break; // state handling switch
	}
}

void VRBackend::WaitXRFrame()
{
	// --- Wait for our turn to do head-pose dependent computation and render a frame
	m_FrameState = { XR_TYPE_FRAME_STATE, nullptr };
	m_FrameWaitInfo = { XR_TYPE_FRAME_WAIT_INFO, nullptr };
	const auto& m_result = xrWaitFrame(m_xr_session, &m_FrameWaitInfo, &m_FrameState);
	if (XR_FAILED(m_result))
	{
		auto err = VRUtils::OpenXRResultString(m_result);
		LogVarError("xrWaitFrame() was not successful, exiting ... : %s", err);
		CTOOL_DEBUG_BREAK;
	}
}

bool VRBackend::BeginXRFrame()
{
	WaitXRFrame();
	RetrieveXRTransform();
	m_Actions.SyncActions(m_xr_session);
	m_Actions.GetActionRealTimeInfos(m_xr_session, m_PlaySpace, m_FrameState);

	// --- Begin frame
	XrFrameBeginInfo frame_begin_info;
	frame_begin_info.type = XR_TYPE_FRAME_BEGIN_INFO;
	frame_begin_info.next = nullptr;

	const auto& m_result = xrBeginFrame(m_xr_session, &frame_begin_info);
	if (XR_FAILED(m_result))
	{
		auto err = VRUtils::OpenXRResultString(m_result);
		LogVarError("Fail to Start Frame : %s", err);
		CTOOL_DEBUG_BREAK;
		return false;
	}

	return true;
}

bool VRBackend::StartXRFrameRendering(uint32_t vViewIndex, GLuint* vCurrentFBOId)
{
	// render each eye and fill projection_views with the m_result
	
	if (!m_FrameState.shouldRender)
	{
		//LogVarError("shouldRender = false, Skipping rendering work\n");
		return false;
	}

	m_InRendering = true;
	m_ViewCurrentIndex = vViewIndex;

	ComputeTransforms(vViewIndex);

	XrSwapchainImageAcquireInfo acquire_info;
	acquire_info.type = XR_TYPE_SWAPCHAIN_IMAGE_ACQUIRE_INFO;
	acquire_info.next = nullptr;

	m_AcquiredSwapChainIndex = UINT32_MAX;
	m_result = xrAcquireSwapchainImage(m_SwapChains[vViewIndex], &acquire_info, &m_AcquiredSwapChainIndex);
	if (XR_FAILED(m_result))
	{
		auto err = VRUtils::OpenXRResultString(m_result);
		LogVarError("xrAcquireSwapchainImage() Fail to Acquire Swapchain Image : %s", err);
		CTOOL_DEBUG_BREAK;
		return false;
	}

	XrSwapchainImageWaitInfo wait_info;
	wait_info.type = XR_TYPE_SWAPCHAIN_IMAGE_WAIT_INFO;
	wait_info.next = nullptr;
	wait_info.timeout = 1000;

	m_result = xrWaitSwapchainImage(m_SwapChains[vViewIndex], &wait_info);
	if (XR_FAILED(m_result))
	{
		auto err = VRUtils::OpenXRResultString(m_result);
		LogVarError("Failed to wait for swapchain image : %s", err);
		CTOOL_DEBUG_BREAK;
		return false;
	}

	m_AcquiredSwapChainIndexDepth = UINT32_MAX;
	if (m_OpenXRInfo.depthSupported)
	{
		XrSwapchainImageAcquireInfo depth_acquire_info;
		depth_acquire_info.type = XR_TYPE_SWAPCHAIN_IMAGE_ACQUIRE_INFO;
		depth_acquire_info.next = nullptr;
		m_result = xrAcquireSwapchainImage(m_DepthSwapChains[vViewIndex], &depth_acquire_info, &m_AcquiredSwapChainIndexDepth);
		if (XR_FAILED(m_result))
		{
			auto err = VRUtils::OpenXRResultString(m_result);
			LogVarError("xrAcquireSwapchainImage() Fail to Acquire Swapchain Depth Image : %s", err);
			CTOOL_DEBUG_BREAK;
			return false;
		}

		XrSwapchainImageWaitInfo depth_wait_info;
		depth_wait_info.type = XR_TYPE_SWAPCHAIN_IMAGE_WAIT_INFO;
		depth_wait_info.next = nullptr;
		depth_wait_info.timeout = 1000;
		m_result = xrWaitSwapchainImage(m_DepthSwapChains[vViewIndex], &depth_wait_info);
		if (XR_FAILED(m_result))
		{
			auto err = VRUtils::OpenXRResultString(m_result);
			LogVarError("xrWaitSwapchainImage() Fail to Wiat Swapchain Depth Image : %s", err);
			CTOOL_DEBUG_BREAK;
			return false;
		}
	}

	m_ProjectionViews[vViewIndex].pose = m_Views[vViewIndex].pose;
	m_ProjectionViews[vViewIndex].fov = m_Views[vViewIndex].fov;

	GLuint depth_image = m_OpenXRInfo.depthSupported ? m_DepthSwapChainsImages[vViewIndex][m_AcquiredSwapChainIndexDepth].image : 0;

	/////////////////////////////////////////////////////
	//// OPENGL RENDER //////////////////////////////////
	/////////////////////////////////////////////////////

	if (vCurrentFBOId)
		*vCurrentFBOId = m_FrameBuffers[vViewIndex][m_AcquiredSwapChainIndex];

	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_FrameBuffers[vViewIndex][m_AcquiredSwapChainIndex]);
	glDrawBuffer(GL_COLOR_ATTACHMENT0);
	
	glViewport(0, 0, m_ViewSize.x, m_ViewSize.y);
	glScissor(0, 0, m_ViewSize.x, m_ViewSize.y);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, 
		m_SwapChainsImages[vViewIndex][m_AcquiredSwapChainIndex].image, 0);

	if (m_OpenXRInfo.depthSupported)
	{
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, 
			m_DepthSwapChainsImages[vViewIndex][m_AcquiredSwapChainIndexDepth].image, 0);
	}

	return true;
}

void VRBackend::EndXRFrameRendering(uint32_t vViewIndex)
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	m_InRendering = false;

	/////////////////////////////////////////////////////
	/////////////////////////////////////////////////////
	/////////////////////////////////////////////////////

	XrSwapchainImageReleaseInfo release_info;
	release_info.type = XR_TYPE_SWAPCHAIN_IMAGE_RELEASE_INFO;
	release_info.next = nullptr;

	m_result = xrReleaseSwapchainImage(m_SwapChains[vViewIndex], &release_info);
	if (XR_FAILED(m_result))
	{
		auto err = VRUtils::OpenXRResultString(m_result);
		LogVarError("xrReleaseSwapchainImage() Fail to Wait release swapchain image : %s", err);
		CTOOL_DEBUG_BREAK;
		return;
	}

	if (m_OpenXRInfo.depthSupported)
	{
		XrSwapchainImageReleaseInfo depth_release_info;
		depth_release_info.type = XR_TYPE_SWAPCHAIN_IMAGE_RELEASE_INFO;
		depth_release_info.next = nullptr;
		m_result = xrReleaseSwapchainImage(m_DepthSwapChains[vViewIndex], &depth_release_info);
		if (XR_FAILED(m_result))
		{
			auto err = VRUtils::OpenXRResultString(m_result);
			LogVarError("xrReleaseSwapchainImage() Fail to Wait release swapchain Depth image : %s", err);
			CTOOL_DEBUG_BREAK;
			return;
		}
	}
}

static bool s_last_RunFrameCycle = false;
void VRBackend::RenderFBO(ct::ivec2 vScreenSize, ImVec4 vClearColor)
{
	TracyGpuZone("VRBackend::RenderFBO");
	AIGPScoped("VRBackend", "RenderFBO");

	if (m_RunFrameCycle && m_RunFrameCycle != s_last_RunFrameCycle)
	{
		/*
		glClearColor(
			vClearColor.x,
			vClearColor.y,
			vClearColor.z,
			vClearColor.w);
		glClear(GL_COLOR_BUFFER_BIT);
		*/

		s_last_RunFrameCycle = m_RunFrameCycle;
	}

	if (!m_FrameBuffers.empty() &&
		!m_FrameBuffers[0].empty())
	{
		if (m_ViewSize.x > 0.0f && 
			m_ViewSize.y > 0.0f)
		{
			auto rc = ct::GetScreenRectWithSize<float>(
				ct::fvec2((float)m_ViewSize.x, (float)m_ViewSize.y),
				ct::fvec2((float)vScreenSize.x, (float)vScreenSize.y),
				false);

			glBindFramebuffer(GL_READ_FRAMEBUFFER, m_FrameBuffers[0][0]);
			glReadBuffer(GL_COLOR_ATTACHMENT0 + 0);

			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
			glDrawBuffer(GL_BACK);

			glBlitFramebuffer(0, 0, (GLint)m_ViewSize.x, (GLint)m_ViewSize.y,
				(GLint)rc.left, (GLint)rc.bottom, (GLint)rc.right, (GLint)rc.top,
				GL_COLOR_BUFFER_BIT, GL_LINEAR);
		}
	}
}

const bool& VRBackend::IsInRendering() const
{
	return m_InRendering;
}

const uint32_t& VRBackend::GetViewCount() const
{
	return m_ViewCount;
}

const XrFrameState& VRBackend::GetFrameState() const
{
	return m_FrameState;
}

const ct::ivec2 VRBackend::GetFBOSize() const 
{
	return m_ViewSize;
}

void VRBackend::EndXRFrame()
{
	XrCompositionLayerProjection projection_layer;
	projection_layer.type = XR_TYPE_COMPOSITION_LAYER_PROJECTION;
	projection_layer.next = nullptr;
	projection_layer.layerFlags = 0;
	projection_layer.space = m_PlaySpace;
	projection_layer.viewCount = m_ViewCount;
	projection_layer.views = m_ProjectionViews.data();

	uint32_t submitted_layer_count = 1U;
	const XrCompositionLayerBaseHeader* submitted_layers[1] = {
		(const XrCompositionLayerBaseHeader* const)&projection_layer };

	if ((m_ViewState.viewStateFlags & XR_VIEW_STATE_ORIENTATION_VALID_BIT) == 0)
	{
		//LogVarError("submitting 0 layers because orientation is invalid\n");
		submitted_layer_count = 0;
	}

	if (!m_FrameState.shouldRender)
	{
		//LogVarError("submitting 0 layers because shouldRender = false\n");
		submitted_layer_count = 0;
	}

	XrFrameEndInfo frameEndInfo;
	frameEndInfo.type = XR_TYPE_FRAME_END_INFO;
	frameEndInfo.displayTime = m_FrameState.predictedDisplayTime;
	frameEndInfo.layerCount = submitted_layer_count;
	frameEndInfo.layers = submitted_layers;
	frameEndInfo.environmentBlendMode = XR_ENVIRONMENT_BLEND_MODE_OPAQUE;
	frameEndInfo.next = nullptr;

	const auto& m_result = xrEndFrame(m_xr_session, &frameEndInfo);
	if (XR_FAILED(m_result))
	{
		auto err = VRUtils::OpenXRResultString(m_result);
		LogVarError("Fail to end frame : %s", err);
		CTOOL_DEBUG_BREAK;
	}
}

bool VRBackend::RetrieveXRTransform()
{
	// --- Create projection matrices and view matrices for each eye
	XrViewLocateInfo view_locate_info;
	view_locate_info.type = XR_TYPE_VIEW_LOCATE_INFO;
	view_locate_info.next = nullptr;
	view_locate_info.viewConfigurationType = XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO;
	view_locate_info.displayTime = m_FrameState.predictedDisplayTime;
	view_locate_info.space = m_PlaySpace;

	m_ViewState.type = XR_TYPE_VIEW_STATE;
	m_ViewState.next = nullptr;

	for (auto& view : m_Views)
	{
		view.next = nullptr;
	}

	const auto& m_result = xrLocateViews(m_xr_session, &view_locate_info, &m_ViewState, m_ViewCount, &m_ViewCount, m_Views.data());
	if (XR_FAILED(m_result))
	{
		auto err = VRUtils::OpenXRResultString(m_result);
		LogVarError("xrLocateViews() fail : %s", err);
		CTOOL_DEBUG_BREAK;
		return false;
	}
	
	if (!headsetCanMove)
	{
		for (auto& view : m_Views)
		{
			//view.pose = XrPosef();
		}
	}

	return true;
}

const char* VRBackend::GetSessionStateString(const XrSessionState& vState)
{
	switch (vState)
	{
	case XR_SESSION_STATE_UNKNOWN:
		return "XR_SESSION_STATE_UNKNOWN";
	case XR_SESSION_STATE_IDLE:
		return "XR_SESSION_STATE_IDLE";
	case XR_SESSION_STATE_READY:
		return "XR_SESSION_STATE_READY";
	case XR_SESSION_STATE_SYNCHRONIZED:
		return "XR_SESSION_STATE_SYNCHRONIZED";
	case XR_SESSION_STATE_VISIBLE:
		return "XR_SESSION_STATE_VISIBLE";
	case XR_SESSION_STATE_FOCUSED:
		return "XR_SESSION_STATE_FOCUSED";
	case XR_SESSION_STATE_STOPPING:
		return "XR_SESSION_STATE_STOPPING";
	case XR_SESSION_STATE_LOSS_PENDING :
		return "XR_SESSION_STATE_LOSS_PENDING";
	case XR_SESSION_STATE_EXITING:
		return "XR_SESSION_STATE_EXITING";
	default:
		break;
	}

	return "Unknow";
}

void VRBackend::ComputeTransforms(const uint32_t vViewIndex)
{
	auto const camPtr = CameraSystem::Instance();

	uView = glm::mat4(1.0f);
	uProj = camPtr->uProj;
	uModel = glm::mat4(1.0f);

	VRUtils::CreateProjectionFov(uProj,
		VRUtils::GraphicsAPI::GRAPHICS_OPENGL, 
		m_Views[vViewIndex].fov,
		camPtr->nearFarPlanes.x, 
		camPtr->nearFarPlanes.y);
	VRUtils::CreateViewMatrix(uModel,
		m_Views[vViewIndex].pose.position, 
		m_Views[vViewIndex].pose.orientation);

	uModel *= camPtr->uModel;
	uNormalMatrix = glm::transpose(glm::inverse(uModel));
	uCam = uProj * uModel;
	uInvCam = glm::inverse(uCam);
}

/////////////////////////////////////////////////////////////////////
///// CONFIGURATION XML /////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////

std::string VRBackend::getXml(const std::string& vOffset, const std::string& vUserDatas)
{
	UNUSED(vUserDatas);

	std::string str;

	str += vOffset + "<VRBackend>\n";

	str += vOffset + "</VRBackend>\n";

	return str;
}

bool VRBackend::setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas)
{
	UNUSED(vUserDatas);

	// The value of this child identifies the name of this element
	std::string strName = "";
	std::string strValue = "";
	std::string strParentName = "";

	strName = vElem->Value();
	if (vElem->GetText())
		strValue = vElem->GetText();
	if (vParent != nullptr)
		strParentName = vParent->Value();

	if (strParentName == "VRBackend")
	{
		
	}

	return true;
}

#endif // USE_VR