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

#include <ctools/cTools.h>
#include <openxr/openxr.h>
#include <openxr/openxr_platform.h>
#include <openxr/openxr_reflection.h>
#include <vector>
#include <map>

#define HAND_LEFT_INDEX 0
#define HAND_RIGHT_INDEX 1
#define HAND_COUNT 2

class VRAction {
public:
};

class ControllerData {
public:
    XrActionStateFloat grabValue;
    XrActionStateFloat triggerValue;
    XrActionStateFloat squeezeValue;
    XrActionStateFloat thumbstickXValue;
    XrActionStateFloat thumbstickYValue;
    XrSpaceLocation controllerLocation;
};

class ControllerInput {
private:
    XrActionType actionType = XR_ACTION_TYPE_FLOAT_INPUT;

public:
    XrAction action;
    std::vector<XrPath> paths;

public:
    bool CreateAction(const char* vActionName,
                      const char* vLocalizedActionName,
                      const std::vector<XrPath>& vControllerBasePaths,
                      const XrActionSet& vActionSet,
                      const XrActionType& vActionType);
    template <typename T>
    bool CaptureDatas(const XrSession& vXRSession, const uint32_t& vEye, const std::vector<XrPath>& vControllerBasePaths, T& vOutputAction);

    void clear();
};

class VRActions {
private:
    XrResult m_result = XR_SUCCESS;
    XrActionSet m_GamePlayActionSet;
    std::vector<XrPath> m_ControllerBasePaths;

    std::vector<XrPath> m_SelectClickPath;

    ControllerInput m_TriggerInput;
    ControllerInput m_SqueezeInput;
    ControllerInput m_ThumbstickXInput;
    ControllerInput m_ThumbstickYInput;

    std::vector<XrPath> m_GripPosePath;
    std::vector<XrPath> m_HapticPath;
    XrPath m_InteractionProfilePath;
    XrAction m_GrabActionFloat;
    XrAction m_HapticAction;
    XrAction m_ControllerPoseAction;
    std::vector<XrSpace> m_ControllerPoseSpaces;
    std::vector<ControllerData> m_ControllerDatas;
    // std::vector<ControllerData> m_LastControllerDatas;

public:
    VRActions();
    ~VRActions();

    void clear();

    bool CreateXRActions(const XrInstance& vXRInstance, const XrSession& vXRSession, const XrPosef& vBasePose);
    void DestroyXRActions();

    bool SyncActions(const XrSession& vXRSession);

    void GetActionRealTimeInfos(const XrSession& vXRSession, const XrSpace& vPlaySpace, const XrFrameState& vFrameState);

    const std::vector<ControllerData>& GetOpenXRControllerDatas() const;

    void EventInteractionProfileChnaged(const XrInstance& vXRInstance, const XrSession& vXRSession, const XrEventDataBuffer& vRunTimeEvent);

private:
    bool FillActionPath(const XrInstance& vXRInstance, const char* vStringPath, XrPath* vActionPath);
    bool SuggestController_Simple(const XrInstance& vXRInstance);
    bool SuggestController_OculusTouch(const XrInstance& vXRInstance);
    bool SuggestController_ValveIndex(const XrInstance& vXRInstance);
};

#endif  // USE_VR