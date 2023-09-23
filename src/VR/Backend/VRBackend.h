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

#ifdef USE_VR

#include <ctools/cTools.h>
#include <Gui/GuiBackend.h>
#include <Renderer/RenderPack.h>
#include <ctools/ConfigAbstract.h>
#include <Systems/Interfaces/MouseInterface.h>
#include <Systems/Interfaces/CameraInterface.h>
#include <Systems/Interfaces/WidgetInterface.h>
#include <VR/Backend/VRActions.h>

#include <unordered_set>
#include <string>
#include <vector>
#include <memory>

#include <openxr/openxr.h>
#include <openxr/openxr_platform.h>
#include <openxr/openxr_reflection.h>

struct OpenXRInfoStruct
{
	std::string runtimeName;
	std::string runtimeVersion;
	std::string systemName;
	std::string vendorId;
	bool orientationTracking = false;
	bool positionTracking = false;
	uint32_t maxLayerCount = 0;
	ct::uvec2 maxSwapChainSize = 0U;
	bool openglSupported = false;
	bool depthSupported = false;
};

class RenderPack;
class UniformVariant;
class VRBackend : public CameraInterface, public WidgetInterface, public conf::ConfigAbstract
{
public:
	bool headsetCanMove = false;

private:
	bool m_Loaded = false;
	bool m_InRendering = false;

	XrResult m_result = XR_SUCCESS;

	VRActions m_Actions;

	// OpenXR objects
	XrInstance  m_xr_instance = {};
	const char* m_xr_instance_err = nullptr;
	XrSession   m_xr_session = {};
	const char* m_xr_session_err = nullptr;
	XrSystemId  m_xr_system_id = {};
	const char* m_xr_system_err = nullptr;

	PFN_xrGetOpenGLGraphicsRequirementsKHR pfnGetOpenGLGraphicsRequirementsKHR = nullptr;

	// frame
	XrFrameState m_FrameState = { XR_TYPE_FRAME_STATE, nullptr };
	XrFrameWaitInfo m_FrameWaitInfo = { XR_TYPE_FRAME_WAIT_INFO, nullptr };

	// space
	XrReferenceSpaceType m_PlaySpaceType = XR_REFERENCE_SPACE_TYPE_LOCAL;
	XrSpace m_PlaySpace = XR_NULL_HANDLE;
	XrPosef m_BasePose;

	// view
	XrViewConfigurationType m_ViewType = XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO;
	uint32_t m_ViewCount = 0U;
	uint32_t m_ViewCurrentIndex = 0U;
	std::vector<XrViewConfigurationView> m_ViewConfigViews;
	std::vector<XrCompositionLayerProjectionView> m_ProjectionViews;
	std::vector<XrView> m_Views;
	ct::ivec2 m_ViewSize;

	// poll events
	XrSessionState m_xr_session_state = XR_SESSION_STATE_UNKNOWN;
	bool m_NeedToQuitMainLoop = false;
	bool m_SessionIsRunning = false;
	bool m_RunFrameCycle = false;

	// transforms
	XrViewState m_ViewState;
		
	OpenXRInfoStruct m_OpenXRInfo = {};

	// SwapChains
	std::vector<XrSwapchain> m_SwapChains;
	std::vector<uint32_t> m_SwapChainsSizes;
	std::vector<std::vector<XrSwapchainImageOpenGLKHR>> m_SwapChainsImages;
	std::vector<XrCompositionLayerDepthInfoKHR> m_DepthInfos;
	std::vector<XrSwapchain> m_DepthSwapChains;
	std::vector<uint32_t> m_DepthSwapChainsSizes;
	std::vector<std::vector<XrSwapchainImageOpenGLKHR>> m_DepthSwapChainsImages;
	uint32_t m_AcquiredSwapChainIndex = UINT32_MAX;
	uint32_t m_AcquiredSwapChainIndexDepth = UINT32_MAX;

	std::vector<std::vector<uint32_t>> m_FrameBuffers;

public:
	bool Init(CodeTreePtr vCodeTree) override;
	void Unit() override;
	bool DrawWidget(CodeTreePtr vCodeTree, UniformVariantPtr vUniPtr,
		const float& vMaxWidth, const float& vFirstColumnWidth, RenderPackWeak vRenderPack,
		const bool& vShowUnUsed, const bool& vShowCustom, const bool& vForNodes, bool* vChange) override;
	bool SerializeUniform(UniformVariantPtr vUniform, std::string* vStr) override;
	bool DeSerializeUniform(ShaderKeyPtr vShaderKey, UniformVariantPtr vUniform, const std::vector<std::string>& vParams) override;
	void Complete_Uniform(ShaderKeyPtr vParentKey, RenderPackWeak vRenderPack, const UniformParsedStruct& vUniformParsed, UniformVariantPtr vUniform) override;
	bool UpdateUniforms(UniformVariantPtr vUniPtr) override;

	void ClearVars();

	bool IsLoaded();

	bool NewFrame();
	void EndFrame();
	bool BeginXRFrame();
	void EndXRFrame();
	bool StartXRFrameRendering(uint32_t vViewIndex, GLuint* vCurrentFBOId = nullptr);
	void EndXRFrameRendering(uint32_t vViewIndex);
	void RenderFBO(ct::ivec2 vScreenSize, ImVec4 vClearColor);

	const bool& IsInRendering() const;
	const uint32_t& GetViewCount() const;
	const XrFrameState& GetFrameState() const;
	const ct::ivec2 GetFBOSize() const;

	const OpenXRInfoStruct& GetOpenXRInfo() const;
	const std::vector<XrView>& GetOpenXRViews() const;
	const VRActions& GetXRActions() const;
	bool IsCameraMoving(const uint32_t vEye) const;

private:
	bool CreateXRInstance();
	void DestroyXRInstance();

	bool CreateXRSystem();
	void DestroyXRSystem();

	bool CreateXRSession();
	void DestroyXRSession();

	bool CreateXRViews();
	void DestroyXRViews();

	bool CreateXRRefSpace();
	void DestroyXRRefSpace();

	int64_t GetSwapchainFormat(XrInstance instance, XrSession session, int64_t preferred_format, bool fallback);
	bool CreateXRSwapChains();
	void DestroyXRSwapChains();

	bool CreateFrameBuffers();
	void DestroyFrameBuffers();

	void RetrieveOpenXRInfos();

	void PollXREvents();
	void ReactToSessionState(const XrSessionState& vState);

	void WaitXRFrame();
	bool RetrieveXRTransform();

	const char* GetSessionStateString(const XrSessionState& vState);

	void ComputeTransforms(const uint32_t vViewIndex);

public:
	static VRBackend* Instance()
	{
		static VRBackend _instance;
		return &_instance;
	}

protected:
	VRBackend(); // Prevent construction
	VRBackend(const VRBackend&) {}; // Prevent construction by copying
	VRBackend& operator =(const VRBackend&) { return *this; }; // Prevent assignment
	~VRBackend(); // Prevent unwanted destruction

public:
	std::string getXml(const std::string& vOffset, const std::string& vUserDatas) override;
	bool setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas) override;
};

#endif // USE_VR
