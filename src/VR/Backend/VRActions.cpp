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

#ifdef USE_VR

#include <VR/Backend/VRActions.h>
#include <VR/Utils/VRUtils.h>
#include <Systems/CameraSystem.h>
#include <ctools/Logger.h>

//////////////////////////////////////////////////////////////////////////////////////////
//// CONTROLLER INPUT CLASS //////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////

bool ControllerInput::CreateAction(const char* vActionName, const char* vLocalizedActionName, const std::vector<XrPath>& vControllerBasePaths,
                                   const XrActionSet& vActionSet, const XrActionType& vActionType) {
    assert(vActionName);
    assert(vLocalizedActionName);

    if (vControllerBasePaths.empty())
        return false;

    actionType = vActionType;

    XrActionCreateInfo action_info;
    action_info.type = XR_TYPE_ACTION_CREATE_INFO;
    action_info.next = NULL;
    action_info.actionType = actionType;
    action_info.countSubactionPaths = (uint32_t)vControllerBasePaths.size();
    action_info.subactionPaths = vControllerBasePaths.data();
    strcpy(action_info.actionName, vActionName);
    strcpy(action_info.localizedActionName, vLocalizedActionName);

    auto result = xrCreateAction(vActionSet, &action_info, &action);
    if (XR_FAILED(result)) {
        auto err = VRUtils::OpenXRResultString(result);
        LogVarError("xrCreateAction() failed to create action %s : %s", vActionName, err);
        CTOOL_DEBUG_BREAK;
        return false;
    }

    return true;
}

template <typename T>
bool ControllerInput::CaptureDatas(const XrSession& vXRSession, const uint32_t& vEye, const std::vector<XrPath>& vControllerBasePaths,
                                   T& vOutputAction) {
    switch (actionType) {
        case XR_ACTION_TYPE_BOOLEAN_INPUT: vOutputAction.type = XR_TYPE_ACTION_STATE_BOOLEAN; break;
        case XR_ACTION_TYPE_FLOAT_INPUT: vOutputAction.type = XR_TYPE_ACTION_STATE_FLOAT; break;
        case XR_ACTION_TYPE_VECTOR2F_INPUT: vOutputAction.type = XR_TYPE_ACTION_STATE_VECTOR2F; break;
        case XR_ACTION_TYPE_POSE_INPUT: vOutputAction.type = XR_TYPE_ACTION_STATE_POSE; break;
        default: break;
    }

    vOutputAction.next = NULL;

    XrActionStateGetInfo get_info = {};
    get_info.type = XR_TYPE_ACTION_STATE_GET_INFO;
    get_info.next = NULL;
    get_info.action = action;
    get_info.subactionPath = vControllerBasePaths[vEye];

    XrResult result = XR_SUCCESS;

    switch (actionType) {
        case XR_ACTION_TYPE_BOOLEAN_INPUT: result = xrGetActionStateBoolean(vXRSession, &get_info, (XrActionStateBoolean*)&vOutputAction); break;
        case XR_ACTION_TYPE_FLOAT_INPUT: result = xrGetActionStateFloat(vXRSession, &get_info, (XrActionStateFloat*)&vOutputAction); break;
        case XR_ACTION_TYPE_VECTOR2F_INPUT: result = xrGetActionStateVector2f(vXRSession, &get_info, (XrActionStateVector2f*)&vOutputAction); break;
        case XR_ACTION_TYPE_POSE_INPUT: result = xrGetActionStatePose(vXRSession, &get_info, (XrActionStatePose*)&vOutputAction); break;
        case XR_ACTION_TYPE_VIBRATION_OUTPUT: break;
        default: break;
    }

    if (XR_FAILED(result)) {
        auto err = VRUtils::OpenXRResultString(result);
        LogVarError("fail to get input value : %s", err);
        CTOOL_DEBUG_BREAK;
        return false;
    }

    return true;
}
void ControllerInput::clear() {
    action = {};
    paths.clear();
}

//////////////////////////////////////////////////////////////////////////////////////////
//// PRIVATE : SUGGESTIONS FOR DEVICE CONTROLLERS ////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////

VRActions::VRActions() {
}

VRActions::~VRActions() {
}

void VRActions::clear() {
    // action
    m_GamePlayActionSet;

    // controllers
    m_ControllerBasePaths.clear();
    m_SelectClickPath.clear();
    m_GripPosePath.clear();
    m_HapticPath.clear();
    m_InteractionProfilePath = {};
    m_GrabActionFloat = {};
    m_HapticAction = {};
    m_ControllerPoseSpaces.clear();
    m_ControllerPoseAction = {};
    m_ControllerDatas.clear();

    m_TriggerInput.clear();
    m_SqueezeInput.clear();
    m_ThumbstickXInput.clear();
    m_ThumbstickYInput.clear();
}

const std::vector<ControllerData>& VRActions::GetOpenXRControllerDatas() const {
    return m_ControllerDatas;
}

bool VRActions::CreateXRActions(const XrInstance& vXRInstance, const XrSession& vXRSession, const XrPosef& vBasePose) {
    m_ControllerDatas.clear();
    m_ControllerDatas.resize(HAND_COUNT);

    // compatible profiles : alls
    m_ControllerBasePaths.clear();
    m_ControllerBasePaths.resize(HAND_COUNT);
    FillActionPath(vXRInstance, "/user/hand/left", &m_ControllerBasePaths[HAND_LEFT_INDEX]);
    FillActionPath(vXRInstance, "/user/hand/right", &m_ControllerBasePaths[HAND_RIGHT_INDEX]);

    XrActionSetCreateInfo gameplay_actionset_info;
    gameplay_actionset_info.type = XR_TYPE_ACTION_SET_CREATE_INFO;
    gameplay_actionset_info.next = NULL;
    gameplay_actionset_info.priority = 0;
    strcpy(gameplay_actionset_info.actionSetName, "gameplay_actionset");
    strcpy(gameplay_actionset_info.localizedActionSetName, "Gameplay Actions");

    m_result = xrCreateActionSet(vXRInstance, &gameplay_actionset_info, &m_GamePlayActionSet);
    if (XR_FAILED(m_result)) {
        auto err = VRUtils::OpenXRResultString(m_result);
        LogVarError("xrCreateActionSet() failed to create action set : %s", err);
        CTOOL_DEBUG_BREAK;
        return false;
    }

    ////////////////////////////////

    {
        XrActionCreateInfo action_info;
        action_info.type = XR_TYPE_ACTION_CREATE_INFO;
        action_info.next = NULL;
        action_info.actionType = XR_ACTION_TYPE_POSE_INPUT;
        action_info.countSubactionPaths = (uint32_t)m_ControllerBasePaths.size();
        action_info.subactionPaths = m_ControllerBasePaths.data();
        strcpy(action_info.actionName, "controllerpose");  // no spaces here
        strcpy(action_info.localizedActionName, "Controller Pose");

        m_result = xrCreateAction(m_GamePlayActionSet, &action_info, &m_ControllerPoseAction);
        if (XR_FAILED(m_result)) {
            auto err = VRUtils::OpenXRResultString(m_result);
            LogVarError("xrCreateAction() failed to create hand pose action : %s", err);
            CTOOL_DEBUG_BREAK;
            return false;
        }
    }

    // poses can't be queried directly, we need to create a space for each
    m_ControllerPoseSpaces.clear();
    m_ControllerPoseSpaces.resize(HAND_COUNT);
    for (int hand = 0; hand < HAND_COUNT; hand++) {
        XrActionSpaceCreateInfo action_space_info;
        action_space_info.type = XR_TYPE_ACTION_SPACE_CREATE_INFO;
        action_space_info.next = NULL;
        action_space_info.action = m_ControllerPoseAction;
        action_space_info.poseInActionSpace = vBasePose;
        action_space_info.subactionPath = m_ControllerBasePaths[hand];

        m_result = xrCreateActionSpace(vXRSession, &action_space_info, &m_ControllerPoseSpaces[hand]);
        if (XR_FAILED(m_result)) {
            auto err = VRUtils::OpenXRResultString(m_result);
            LogVarError("xrCreateActionSpace() failed to create hand %d pose space : %s", hand, err);
            CTOOL_DEBUG_BREAK;
            return false;
        }
    }

    if (!m_TriggerInput.CreateAction(  //
            "triggerfloat",            //
            "Trigger",                 //
            m_ControllerBasePaths,     //
            m_GamePlayActionSet,       //
            XrActionType::XR_ACTION_TYPE_FLOAT_INPUT)) {
        return false;
    }
    if (!m_SqueezeInput.CreateAction(  //
            "squeezefloat",            //
            "Squeeze",                 //
            m_ControllerBasePaths,     //
            m_GamePlayActionSet,       //
            XrActionType::XR_ACTION_TYPE_FLOAT_INPUT)) {
        return false;
    }
    if (!m_ThumbstickXInput.CreateAction(  //
            "thumbstickxfloat",            //
            "Thumbstick X",                //
            m_ControllerBasePaths,         //
            m_GamePlayActionSet,           //
            XrActionType::XR_ACTION_TYPE_FLOAT_INPUT)) {
        return false;
    }
    if (!m_ThumbstickYInput.CreateAction(  //
            "thumbstickyfloat",            //
            "Thumbstick Y",                //
            m_ControllerBasePaths,         //
            m_GamePlayActionSet,           //
            XrActionType::XR_ACTION_TYPE_FLOAT_INPUT)) {
        return false;
    }

    // Grabbing objects is not actually implemented in this demo, it only gives some  haptic feebdack.
    {
        XrActionCreateInfo action_info;
        action_info.type = XR_TYPE_ACTION_CREATE_INFO;
        action_info.next = NULL;
        action_info.actionType = XR_ACTION_TYPE_FLOAT_INPUT;
        action_info.countSubactionPaths = (uint32_t)m_ControllerBasePaths.size();
        action_info.subactionPaths = m_ControllerBasePaths.data();
        strcpy(action_info.actionName, "grabobjectfloat");
        strcpy(action_info.localizedActionName, "Grab Object");

        m_result = xrCreateAction(m_GamePlayActionSet, &action_info, &m_GrabActionFloat);
        if (XR_FAILED(m_result)) {
            auto err = VRUtils::OpenXRResultString(m_result);
            LogVarError("xrCreateAction() failed to create grab action : %s", err);
            CTOOL_DEBUG_BREAK;
            return false;
        }
    }

    {
        XrActionCreateInfo action_info;
        action_info.type = XR_TYPE_ACTION_CREATE_INFO;
        action_info.next = NULL;
        action_info.actionType = XR_ACTION_TYPE_VIBRATION_OUTPUT;
        action_info.countSubactionPaths = (uint32_t)m_ControllerBasePaths.size();
        action_info.subactionPaths = m_ControllerBasePaths.data();
        strcpy(action_info.actionName, "haptic");
        strcpy(action_info.localizedActionName, "Haptic Vibration");

        m_result = xrCreateAction(m_GamePlayActionSet, &action_info, &m_HapticAction);
        if (XR_FAILED(m_result)) {
            auto err = VRUtils::OpenXRResultString(m_result);
            LogVarError("xrCreateAction() failed to create haptic action : %s", err);
            CTOOL_DEBUG_BREAK;
            return false;
        }
    }

    // suggest actions
    // not an issue if there is no supported controllers
    {
        SuggestController_Simple(vXRInstance);
        SuggestController_OculusTouch(vXRInstance);
        SuggestController_ValveIndex(vXRInstance);
    }

    ////////////////////////////////

    XrSessionActionSetsAttachInfo actionset_attach_info;
    actionset_attach_info.type = XR_TYPE_SESSION_ACTION_SETS_ATTACH_INFO, actionset_attach_info.next = nullptr;
    actionset_attach_info.countActionSets = 1;
    actionset_attach_info.actionSets = &m_GamePlayActionSet;
    m_result = xrAttachSessionActionSets(vXRSession, &actionset_attach_info);
    if (XR_FAILED(m_result)) {
        auto err = VRUtils::OpenXRResultString(m_result);
        LogVarError("xrAttachSessionActionSets() failed to attach action set : %s", err);
        CTOOL_DEBUG_BREAK;
        return false;
    }

    return true;
}

void VRActions::DestroyXRActions() {
    xrDestroyActionSet(m_GamePlayActionSet);
    xrDestroyAction(m_ControllerPoseAction);
    for (auto& handPoseSpace : m_ControllerPoseSpaces) {
        xrDestroySpace(handPoseSpace);
    }
    xrDestroyAction(m_GrabActionFloat);
    xrDestroyAction(m_HapticAction);
}

bool VRActions::SyncActions(const XrSession& vXRSession) {
    const XrActiveActionSet activeActionSet = {m_GamePlayActionSet, XR_NULL_PATH};
    const XrActionsSyncInfo actions_sync_info = {XR_TYPE_ACTIONS_SYNC_INFO, nullptr, 1U, &activeActionSet};
    const auto& m_result = xrSyncActions(vXRSession, &actions_sync_info);
    if (XR_FAILED(m_result)) {
        auto err = VRUtils::OpenXRResultString(m_result);
        LogVarError("xrSyncActions() failed to sync actions : %s", err);
        CTOOL_DEBUG_BREAK;
        return false;
    }
    return true;
}

void VRActions::GetActionRealTimeInfos(const XrSession& vXRSession, const XrSpace& vPlaySpace, const XrFrameState& vFrameState) {
    // query each value / location with a subaction path != XR_NULL_PATH
    // resulting in individual values per hand/.

    for (int i = 0; i < HAND_COUNT; i++) {
        auto& controllerData = m_ControllerDatas[i];

        XrActionStatePose hand_pose_state;
        hand_pose_state.type = XR_TYPE_ACTION_STATE_POSE;
        hand_pose_state.next = NULL;
        {
            XrActionStateGetInfo get_info;
            get_info.type = XR_TYPE_ACTION_STATE_GET_INFO;
            get_info.next = NULL;
            get_info.action = m_ControllerPoseAction;
            get_info.subactionPath = m_ControllerBasePaths[i];
            m_result = xrGetActionStatePose(vXRSession, &get_info, &hand_pose_state);
            if (XR_FAILED(m_result)) {
                auto err = VRUtils::OpenXRResultString(m_result);
                LogVarError("xrGetActionStatePose() failed to get pose value : %s", err);
                CTOOL_DEBUG_BREAK;
            }
        }

        controllerData.controllerLocation.type = XR_TYPE_SPACE_LOCATION;
        controllerData.controllerLocation.next = NULL;

        m_result = xrLocateSpace(m_ControllerPoseSpaces[i], vPlaySpace, vFrameState.predictedDisplayTime, &controllerData.controllerLocation);
        if (XR_FAILED(m_result)) {
            auto err = VRUtils::OpenXRResultString(m_result);
            LogVarError("xrGetActionStatePose() failed to get locate value space %d : %s", i, err);
            CTOOL_DEBUG_BREAK;
        }

        controllerData.grabValue.type = XR_TYPE_ACTION_STATE_FLOAT;
        controllerData.grabValue.next = NULL;
        XrActionStateGetInfo get_info;
        get_info.type = XR_TYPE_ACTION_STATE_GET_INFO;
        get_info.next = NULL;
        get_info.action = m_GrabActionFloat;
        get_info.subactionPath = m_ControllerBasePaths[i];
        m_result = xrGetActionStateFloat(vXRSession, &get_info, &controllerData.grabValue);
        if (XR_FAILED(m_result)) {
            auto err = VRUtils::OpenXRResultString(m_result);
            LogVarError("xrGetActionStatePose() failed to get grab value : %s", err);
            CTOOL_DEBUG_BREAK;
        }

        if (controllerData.grabValue.isActive && controllerData.grabValue.currentState > 0.75f) {
            XrHapticVibration vibration;
            vibration.type = XR_TYPE_HAPTIC_VIBRATION;
            vibration.next = NULL;
            vibration.amplitude = 0.25f;
            vibration.duration = XR_MIN_HAPTIC_DURATION;
            vibration.frequency = XR_FREQUENCY_UNSPECIFIED;

            XrHapticActionInfo haptic_action_info;
            haptic_action_info.type = XR_TYPE_HAPTIC_ACTION_INFO;
            haptic_action_info.next = NULL;
            haptic_action_info.action = m_HapticAction;
            haptic_action_info.subactionPath = m_ControllerBasePaths[i];

            m_result = xrApplyHapticFeedback(vXRSession, &haptic_action_info, (const XrHapticBaseHeader*)&vibration);
            if (XR_FAILED(m_result)) {
                auto err = VRUtils::OpenXRResultString(m_result);
                LogVarError("xrGetActionStatePose() failed to apply haptic feedback : %s", err);
                CTOOL_DEBUG_BREAK;
            }
        }

        m_TriggerInput.CaptureDatas(vXRSession, i, m_ControllerBasePaths, controllerData.triggerValue);
        m_SqueezeInput.CaptureDatas(vXRSession, i, m_ControllerBasePaths, controllerData.squeezeValue);
        m_ThumbstickXInput.CaptureDatas(vXRSession, i, m_ControllerBasePaths, controllerData.thumbstickXValue);
        m_ThumbstickYInput.CaptureDatas(vXRSession, i, m_ControllerBasePaths, controllerData.thumbstickYValue);

        if (CameraSystem::Instance()->m_CameraSettings.m_UseVRControllerForControlCamera) {
            if (controllerData.squeezeValue.currentState > 0.5f) {
                if (i == 0) {
                    if (CameraSystem::Instance()->m_CameraSettings.m_CameraMode == CAMERA_MODE_Enum::CAMERA_MODE_TURNTABLE_Y) {
                        if (IS_FLOAT_DIFFERENT(controllerData.thumbstickYValue.currentState, 0.0f)) {
                            CameraSystem::Instance()->IncRotateXYZ(ct::fvec3(
                                0.0f, 0.0f,
                                controllerData.thumbstickXValue.currentState * CameraSystem::Instance()->m_CameraSettings.m_VRRotStepFactor.z));
                            CameraSystem::Instance()->IncZoom(controllerData.thumbstickYValue.currentState *
                                                              CameraSystem::Instance()->m_CameraSettings.m_VRZoomStepFactor);
                        }
                    } else if (CameraSystem::Instance()->m_CameraSettings.m_CameraMode == CAMERA_MODE_Enum::CAMERA_MODE_FREE) {
                        if (IS_FLOAT_DIFFERENT(controllerData.thumbstickYValue.currentState, 0.0f)) {
                            CameraSystem::Instance()->IncRotateXYZ(ct::fvec3(
                                0.0f, 0.0f,
                                controllerData.thumbstickXValue.currentState * CameraSystem::Instance()->m_CameraSettings.m_VRRotStepFactor.z));
                        }

                        if (controllerData.triggerValue.currentState > 0.0f) {
                            CameraSystem::Instance()->IncFlyingPosition(controllerData.triggerValue.currentState *
                                                                        CameraSystem::Instance()->m_CameraSettings.m_SpeedFactor * -1.0f);
                        }
                    }
                } else {
                    if (CameraSystem::Instance()->m_CameraSettings.m_CameraMode == CAMERA_MODE_Enum::CAMERA_MODE_TURNTABLE_Y) {
                        if (IS_FLOAT_DIFFERENT(controllerData.thumbstickXValue.currentState, 0.0f) ||
                            IS_FLOAT_DIFFERENT(controllerData.thumbstickYValue.currentState, 0.0f)) {
                            CameraSystem::Instance()->IncRotateXYZ(ct::fvec3(
                                controllerData.thumbstickXValue.currentState * CameraSystem::Instance()->m_CameraSettings.m_VRRotStepFactor.x,
                                controllerData.thumbstickYValue.currentState * CameraSystem::Instance()->m_CameraSettings.m_VRRotStepFactor.y, 0.0f));
                        }
                    } else if (CameraSystem::Instance()->m_CameraSettings.m_CameraMode == CAMERA_MODE_Enum::CAMERA_MODE_FREE) {
                        if (IS_FLOAT_DIFFERENT(controllerData.thumbstickXValue.currentState, 0.0f) ||
                            IS_FLOAT_DIFFERENT(controllerData.thumbstickYValue.currentState, 0.0f)) {
                            CameraSystem::Instance()->IncRotateXYZ(ct::fvec3(
                                controllerData.thumbstickXValue.currentState * CameraSystem::Instance()->m_CameraSettings.m_VRRotStepFactor.x,
                                controllerData.thumbstickYValue.currentState * CameraSystem::Instance()->m_CameraSettings.m_VRRotStepFactor.y, 0.0f));
                        }

                        if (controllerData.triggerValue.currentState > 0.0f) {
                            CameraSystem::Instance()->IncFlyingPosition(controllerData.triggerValue.currentState *
                                                                        CameraSystem::Instance()->m_CameraSettings.m_SpeedFactor);
                        }
                    }
                }
            }
        }

        // m_LastControllerDatas[i] = controllerData;
    };
}

void VRActions::EventInteractionProfileChnaged(const XrInstance& vXRInstance, const XrSession& vXRSession, const XrEventDataBuffer& vRunTimeEvent) {
    LogVarError("XR Event : interaction profile changed!\n");
    XrEventDataInteractionProfileChanged* event = (XrEventDataInteractionProfileChanged*)&vRunTimeEvent;
    (void)event;

    XrInteractionProfileState state = {};
    state.type = XR_TYPE_INTERACTION_PROFILE_STATE;

    for (int i = 0; i < HAND_COUNT; i++) {
        XrResult res = xrGetCurrentInteractionProfile(vXRSession, m_ControllerBasePaths[i], &state);
        if (XR_FAILED(m_result)) {
            auto err = VRUtils::OpenXRResultString(m_result);
            LogVarError("Fail to get interaction profile for %d : %s", i, err);
            CTOOL_DEBUG_BREAK;
            continue;
        }

        XrPath prof = state.interactionProfile;

        uint32_t strl;
        char profile_str[XR_MAX_PATH_LENGTH];
        res = xrPathToString(vXRInstance, prof, XR_MAX_PATH_LENGTH, &strl, profile_str);
        if (XR_FAILED(m_result)) {
            auto err = VRUtils::OpenXRResultString(m_result);
            LogVarError("Fail to get interaction profile path str for %d : %s", i, err);
            CTOOL_DEBUG_BREAK;
            continue;
        }

        LogVarError("XR Event : Interaction profile changed for %d: %s\n", i, profile_str);
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
//// PRIVATE : SUGGESTIONS FOR DEVICE CONTROLLERS ////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////

bool VRActions::FillActionPath(const XrInstance& vXRInstance, const char* vStringPath, XrPath* vActionPath) {
    assert(vActionPath);

    m_result = xrStringToPath(vXRInstance, vStringPath, vActionPath);
    if (XR_FAILED(m_result)) {
        auto err = VRUtils::OpenXRResultString(m_result);
        LogVarError("xrStringToPath() failed to get user hand left input select click : %s", err);
        CTOOL_DEBUG_BREAK;
        return false;
    }

    return true;
}

bool VRActions::SuggestController_Simple(const XrInstance& vXRInstance) {
    // https://microsoft.github.io/OpenXR-MixedReality/openxr_preview/specs/openxr.html#semantic-path-interaction-profiles
    // Khronos Simple Controller Profile
    /*
    Path: /interaction_profiles/khr/simple_controller
    Valid for user paths :
        /user/hand/left
        /user/hand/right
    This interaction profile provides basic pose, button, and haptic support for applications with simple input needs.
    There is no hardware associated with the profile, and runtimes which support this profile should map the input paths
    provided to whatever the appropriate paths are on the actual hardware.
    Supported component paths:
        �/input/select/click
        �/input/menu/click
        �/input/grip/pose
        �/input/aim/pose
        �/output/haptic
    */
    // compatible profiles : simple, oculus touch, valve index
    m_GripPosePath.clear();
    m_GripPosePath.resize(HAND_COUNT);
    FillActionPath(vXRInstance, "/user/hand/left/input/grip/pose", &m_GripPosePath[HAND_LEFT_INDEX]);
    FillActionPath(vXRInstance, "/user/hand/right/input/grip/pose", &m_GripPosePath[HAND_RIGHT_INDEX]);

    // compatible profiles : simple
    m_SelectClickPath.clear();
    m_SelectClickPath.resize(HAND_COUNT);
    FillActionPath(vXRInstance, "/user/hand/left/input/select/click", &m_SelectClickPath[HAND_LEFT_INDEX]);
    FillActionPath(vXRInstance, "/user/hand/right/input/select/click", &m_SelectClickPath[HAND_RIGHT_INDEX]);

    // compatible profiles : simple, oculus touch, valve index
    m_HapticPath.clear();
    m_HapticPath.resize(HAND_COUNT);
    FillActionPath(vXRInstance, "/user/hand/left/output/haptic", &m_HapticPath[HAND_LEFT_INDEX]);
    FillActionPath(vXRInstance, "/user/hand/right/output/haptic", &m_HapticPath[HAND_RIGHT_INDEX]);

    std::vector<XrActionSuggestedBinding> bindings;
    bindings.push_back({m_ControllerPoseAction, m_GripPosePath[HAND_LEFT_INDEX]});
    bindings.push_back({m_ControllerPoseAction, m_GripPosePath[HAND_RIGHT_INDEX]});
    bindings.push_back({m_GrabActionFloat, m_SelectClickPath[HAND_LEFT_INDEX]});
    bindings.push_back({m_GrabActionFloat, m_SelectClickPath[HAND_RIGHT_INDEX]});
    bindings.push_back({m_HapticAction, m_HapticPath[HAND_LEFT_INDEX]});
    bindings.push_back({m_HapticAction, m_HapticPath[HAND_RIGHT_INDEX]});

    // for simple controller
    FillActionPath(vXRInstance, "/interaction_profiles/khr/simple_controller", &m_InteractionProfilePath);
    if (XR_FAILED(m_result)) {
        auto err = VRUtils::OpenXRResultString(m_result);
        LogVarError("xrStringToPath() failed to get interaction profile for generic simple controller : %s", err);
        CTOOL_DEBUG_BREAK;
        return false;
    }

    XrInteractionProfileSuggestedBinding suggested_bindings;
    suggested_bindings.type = XR_TYPE_INTERACTION_PROFILE_SUGGESTED_BINDING;
    suggested_bindings.next = NULL;
    suggested_bindings.interactionProfile = m_InteractionProfilePath;
    suggested_bindings.countSuggestedBindings = (uint32_t)bindings.size();
    suggested_bindings.suggestedBindings = bindings.data();

    m_result = xrSuggestInteractionProfileBindings(vXRInstance, &suggested_bindings);
    if (XR_FAILED(m_result)) {
        auto err = VRUtils::OpenXRResultString(m_result);
        LogVarError("xrSuggestInteractionProfileBindings() failed to suggest bindings : %s", err);
        CTOOL_DEBUG_BREAK;
        return false;
    }

    return true;
}

bool VRActions::SuggestController_OculusTouch(const XrInstance& vXRInstance) {
    // https://microsoft.github.io/OpenXR-MixedReality/openxr_preview/specs/openxr.html#semantic-path-interaction-profiles
    // Oculus Touch Controller Profile
    /*
    Path: /interaction_profiles/oculus/touch_controller
    Valid for user paths:
        /user/hand/left
        /user/hand/right
    This interaction profile represents the input sources and haptics on the Oculus Touch controller.
    Supported component paths:
        On /user/hand/left only:
            �/input/x/click
            �/input/x/touch
            �/input/y/click
            �/input/y/touch
            �/input/menu/click
        On /user/hand/right only:
            �/input/a/click
            �/input/a/touch
            �/input/b/click
            �/input/b/touch
            �/input/system/click (may not be available for application use)
        �/input/squeeze/value
        �/input/trigger/value
        �/input/trigger/touch
        �/input/thumbstick/x
        �/input/thumbstick/y
        �/input/thumbstick/click
        �/input/thumbstick/touch
        �/input/thumbrest/touch
        �/input/grip/pose
        �/input/aim/pose
        �/output/haptic
    */

    // compatible profiles : simple
    // m_SelectClickPath.clear();
    // m_SelectClickPath.resize(HAND_COUNT);
    // FillActionPath(vXRInstance, "/user/hand/left/input/select/click", &m_SelectClickPath[HAND_LEFT_INDEX]);
    // FillActionPath(vXRInstance, "/user/hand/right/input/select/click", &m_SelectClickPath[HAND_RIGHT_INDEX]);

    // compatible profiles : oculus touch, valve index
    m_TriggerInput.paths.clear();
    m_TriggerInput.paths.resize(HAND_COUNT);
    FillActionPath(vXRInstance, "/user/hand/left/input/trigger/value", &m_TriggerInput.paths[HAND_LEFT_INDEX]);
    FillActionPath(vXRInstance, "/user/hand/right/input/trigger/value", &m_TriggerInput.paths[HAND_RIGHT_INDEX]);

    // compatible profiles : oculus touch, valve index
    m_SqueezeInput.paths.clear();
    m_SqueezeInput.paths.resize(HAND_COUNT);
    FillActionPath(vXRInstance, "/user/hand/left/input/squeeze/value", &m_SqueezeInput.paths[HAND_LEFT_INDEX]);
    FillActionPath(vXRInstance, "/user/hand/right/input/squeeze/value", &m_SqueezeInput.paths[HAND_RIGHT_INDEX]);

    // compatible profiles : simple, oculus touch, valve index
    m_ThumbstickXInput.paths.clear();
    m_ThumbstickXInput.paths.resize(HAND_COUNT);
    FillActionPath(vXRInstance, "/user/hand/left/input/thumbstick/x", &m_ThumbstickXInput.paths[HAND_LEFT_INDEX]);
    FillActionPath(vXRInstance, "/user/hand/right/input/thumbstick/x", &m_ThumbstickXInput.paths[HAND_RIGHT_INDEX]);

    // compatible profiles : simple, oculus touch, valve index
    m_ThumbstickYInput.paths.clear();
    m_ThumbstickYInput.paths.resize(HAND_COUNT);
    FillActionPath(vXRInstance, "/user/hand/left/input/thumbstick/y", &m_ThumbstickYInput.paths[HAND_LEFT_INDEX]);
    FillActionPath(vXRInstance, "/user/hand/right/input/thumbstick/y", &m_ThumbstickYInput.paths[HAND_RIGHT_INDEX]);

    // compatible profiles : simple, oculus touch, valve index
    m_GripPosePath.clear();
    m_GripPosePath.resize(HAND_COUNT);
    FillActionPath(vXRInstance, "/user/hand/left/input/grip/pose", &m_GripPosePath[HAND_LEFT_INDEX]);
    FillActionPath(vXRInstance, "/user/hand/right/input/grip/pose", &m_GripPosePath[HAND_RIGHT_INDEX]);

    // compatible profiles : simple, oculus touch, valve index
    m_HapticPath.clear();
    m_HapticPath.resize(HAND_COUNT);
    FillActionPath(vXRInstance, "/user/hand/left/output/haptic", &m_HapticPath[HAND_LEFT_INDEX]);
    FillActionPath(vXRInstance, "/user/hand/right/output/haptic", &m_HapticPath[HAND_RIGHT_INDEX]);

    std::vector<XrActionSuggestedBinding> bindings;
    bindings.push_back({m_ControllerPoseAction, m_GripPosePath[HAND_LEFT_INDEX]});
    bindings.push_back({m_ControllerPoseAction, m_GripPosePath[HAND_RIGHT_INDEX]});
    bindings.push_back({m_TriggerInput.action, m_TriggerInput.paths[HAND_LEFT_INDEX]});
    bindings.push_back({m_TriggerInput.action, m_TriggerInput.paths[HAND_RIGHT_INDEX]});
    bindings.push_back({m_SqueezeInput.action, m_SqueezeInput.paths[HAND_LEFT_INDEX]});
    bindings.push_back({m_SqueezeInput.action, m_SqueezeInput.paths[HAND_RIGHT_INDEX]});
    bindings.push_back({m_ThumbstickXInput.action, m_ThumbstickXInput.paths[HAND_LEFT_INDEX]});
    bindings.push_back({m_ThumbstickXInput.action, m_ThumbstickXInput.paths[HAND_RIGHT_INDEX]});
    bindings.push_back({m_ThumbstickYInput.action, m_ThumbstickYInput.paths[HAND_LEFT_INDEX]});
    bindings.push_back({m_ThumbstickYInput.action, m_ThumbstickYInput.paths[HAND_RIGHT_INDEX]});
    bindings.push_back({m_HapticAction, m_HapticPath[HAND_LEFT_INDEX]});
    bindings.push_back({m_HapticAction, m_HapticPath[HAND_RIGHT_INDEX]});

    // for simple controller
    FillActionPath(vXRInstance, "/interaction_profiles/oculus/touch_controller", &m_InteractionProfilePath);
    if (XR_FAILED(m_result)) {
        auto err = VRUtils::OpenXRResultString(m_result);
        LogVarError("xrStringToPath() failed to get interaction profile for oculus touch controller : %s", err);
        CTOOL_DEBUG_BREAK;
        return false;
    }

    XrInteractionProfileSuggestedBinding suggested_bindings;
    suggested_bindings.type = XR_TYPE_INTERACTION_PROFILE_SUGGESTED_BINDING;
    suggested_bindings.next = NULL;
    suggested_bindings.interactionProfile = m_InteractionProfilePath;
    suggested_bindings.countSuggestedBindings = (uint32_t)bindings.size();
    suggested_bindings.suggestedBindings = bindings.data();

    m_result = xrSuggestInteractionProfileBindings(vXRInstance, &suggested_bindings);
    if (XR_FAILED(m_result)) {
        auto err = VRUtils::OpenXRResultString(m_result);
        LogVarError("xrSuggestInteractionProfileBindings() failed to suggest bindings : %s", err);
        CTOOL_DEBUG_BREAK;
        return false;
    }

    return true;
}

bool VRActions::SuggestController_ValveIndex(const XrInstance& vXRInstance) {
    // https://microsoft.github.io/OpenXR-MixedReality/openxr_preview/specs/openxr.html#semantic-path-interaction-profiles
    // Valve Index Controller Profile
    /*
    Path: /interaction_profiles/valve/index_controller
    Valid for user paths:
        /user/hand/left
        /user/hand/right
    This interaction profile represents the input sources and haptics on the Valve Index controller.
    Supported component paths:
        �/input/system/click (may not be available for application use)
        �/input/system/touch (may not be available for application use)
        �/input/a/click
        �/input/a/touch
        �/input/b/click
        �/input/b/touch
        �/input/squeeze/value
        �/input/squeeze/force
        �/input/trigger/click
        �/input/trigger/value
        �/input/trigger/touch
        �/input/thumbstick/x
        �/input/thumbstick/y
        �/input/thumbstick/click
        �/input/thumbstick/touch
        �/input/trackpad/x
        �/input/trackpad/y
        �/input/trackpad/force
        �/input/trackpad/touch
        �/input/grip/pose
        �/input/aim/pose
        �/output/haptic
    */

    // compatible profiles : simple
    // m_SelectClickPath.clear();
    // m_SelectClickPath.resize(HAND_COUNT);
    // FillActionPath(vXRInstance, "/user/hand/left/input/select/click", &m_SelectClickPath[HAND_LEFT_INDEX]);
    // FillActionPath(vXRInstance, "/user/hand/right/input/select/click", &m_SelectClickPath[HAND_RIGHT_INDEX]);

    // compatible profiles : oculus touch, valve index
    m_TriggerInput.paths.clear();
    m_TriggerInput.paths.resize(HAND_COUNT);
    FillActionPath(vXRInstance, "/user/hand/left/input/trigger/value", &m_TriggerInput.paths[HAND_LEFT_INDEX]);
    FillActionPath(vXRInstance, "/user/hand/right/input/trigger/value", &m_TriggerInput.paths[HAND_RIGHT_INDEX]);

    // compatible profiles : oculus touch, valve index
    m_SqueezeInput.paths.clear();
    m_SqueezeInput.paths.resize(HAND_COUNT);
    FillActionPath(vXRInstance, "/user/hand/left/input/squeeze/value", &m_SqueezeInput.paths[HAND_LEFT_INDEX]);
    FillActionPath(vXRInstance, "/user/hand/right/input/squeeze/value", &m_SqueezeInput.paths[HAND_RIGHT_INDEX]);

    // compatible profiles : simple, oculus touch, valve index
    m_ThumbstickXInput.paths.clear();
    m_ThumbstickXInput.paths.resize(HAND_COUNT);
    FillActionPath(vXRInstance, "/user/hand/left/input/thumbstick/x", &m_ThumbstickXInput.paths[HAND_LEFT_INDEX]);
    FillActionPath(vXRInstance, "/user/hand/right/input/thumbstick/x", &m_ThumbstickXInput.paths[HAND_RIGHT_INDEX]);

    // compatible profiles : simple, oculus touch, valve index
    m_ThumbstickYInput.paths.clear();
    m_ThumbstickYInput.paths.resize(HAND_COUNT);
    FillActionPath(vXRInstance, "/user/hand/left/input/thumbstick/y", &m_ThumbstickYInput.paths[HAND_LEFT_INDEX]);
    FillActionPath(vXRInstance, "/user/hand/right/input/thumbstick/y", &m_ThumbstickYInput.paths[HAND_RIGHT_INDEX]);

    // compatible profiles : simple, oculus touch, valve index
    m_GripPosePath.clear();
    m_GripPosePath.resize(HAND_COUNT);
    FillActionPath(vXRInstance, "/user/hand/left/input/grip/pose", &m_GripPosePath[HAND_LEFT_INDEX]);
    FillActionPath(vXRInstance, "/user/hand/right/input/grip/pose", &m_GripPosePath[HAND_RIGHT_INDEX]);

    // compatible profiles : simple, oculus touch, valve index
    m_HapticPath.clear();
    m_HapticPath.resize(HAND_COUNT);
    FillActionPath(vXRInstance, "/user/hand/left/output/haptic", &m_HapticPath[HAND_LEFT_INDEX]);
    FillActionPath(vXRInstance, "/user/hand/right/output/haptic", &m_HapticPath[HAND_RIGHT_INDEX]);

    std::vector<XrActionSuggestedBinding> bindings;
    bindings.push_back({m_ControllerPoseAction, m_GripPosePath[HAND_LEFT_INDEX]});
    bindings.push_back({m_ControllerPoseAction, m_GripPosePath[HAND_RIGHT_INDEX]});
    bindings.push_back({m_TriggerInput.action, m_TriggerInput.paths[HAND_LEFT_INDEX]});
    bindings.push_back({m_TriggerInput.action, m_TriggerInput.paths[HAND_RIGHT_INDEX]});
    bindings.push_back({m_SqueezeInput.action, m_SqueezeInput.paths[HAND_LEFT_INDEX]});
    bindings.push_back({m_SqueezeInput.action, m_SqueezeInput.paths[HAND_RIGHT_INDEX]});
    bindings.push_back({m_ThumbstickXInput.action, m_ThumbstickXInput.paths[HAND_LEFT_INDEX]});
    bindings.push_back({m_ThumbstickXInput.action, m_ThumbstickXInput.paths[HAND_RIGHT_INDEX]});
    bindings.push_back({m_ThumbstickYInput.action, m_ThumbstickYInput.paths[HAND_LEFT_INDEX]});
    bindings.push_back({m_ThumbstickYInput.action, m_ThumbstickYInput.paths[HAND_RIGHT_INDEX]});
    bindings.push_back({m_HapticAction, m_HapticPath[HAND_LEFT_INDEX]});
    bindings.push_back({m_HapticAction, m_HapticPath[HAND_RIGHT_INDEX]});

    // for simple controller
    FillActionPath(vXRInstance, "/interaction_profiles/valve/index_controller", &m_InteractionProfilePath);
    if (XR_FAILED(m_result)) {
        auto err = VRUtils::OpenXRResultString(m_result);
        LogVarError("xrStringToPath() failed to get interaction profile for valve index controller : %s", err);
        CTOOL_DEBUG_BREAK;
        return false;
    }

    XrInteractionProfileSuggestedBinding suggested_bindings;
    suggested_bindings.type = XR_TYPE_INTERACTION_PROFILE_SUGGESTED_BINDING;
    suggested_bindings.next = NULL;
    suggested_bindings.interactionProfile = m_InteractionProfilePath;
    suggested_bindings.countSuggestedBindings = (uint32_t)bindings.size();
    suggested_bindings.suggestedBindings = bindings.data();

    m_result = xrSuggestInteractionProfileBindings(vXRInstance, &suggested_bindings);
    if (XR_FAILED(m_result)) {
        auto err = VRUtils::OpenXRResultString(m_result);
        LogVarError("xrSuggestInteractionProfileBindings() failed to suggest bindings : %s", err);
        CTOOL_DEBUG_BREAK;
        return false;
    }

    return true;
}

#endif  // USE_VR