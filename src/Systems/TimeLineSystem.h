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

#include <ImGuiPack/ImGuiPack.h>
#include <ctools/cTools.h>
#include <ctools/ConfigAbstract.h>
#include <Uniforms/UniformVariant.h>
#include <Headers/RenderPackHeaders.h>
#include <Interfaces/RenderingInterface.h>

#include <unordered_set>

enum TimeLineItemAxis
{
	TIMELINE_AXIS_RED = (1 << 0),
	TIMELINE_AXIS_GREEN = (1 << 1),
	TIMELINE_AXIS_BLUE = (1 << 2),
	TIMELINE_AXIS_ALPHA = (1 << 3),
};

enum class TimeLineSegmentInterpolation
{
	TIMELINE_INTERPOLATION_LINEAR = 0, // points of segment have no control points
	TIMELINE_INTERPOLATION_SPLINE_QUADRATIC, // only one point on two of segment have a control point
	TIMELINE_INTERPOLATION_SPLINE_CUBIC, // both points have control points of segment
	TIMELINE_INTERPOLATION_Count
};

enum class TimeLineHandlerType
{
	TIMELINE_HANDLER_TYPE_CONTROL_POINT_NONE = 0,
	TIMELINE_HANDLER_TYPE_CONTROL_POINT_LEFT,
	TIMELINE_HANDLER_TYPE_CONTROL_POINT_RIGHT,
	TIMELINE_HANDLER_TYPE_CONTROL_POINT_BOTH,
	TIMELINE_HANDLER_TYPE_CONTROL_POINT_Count
};

enum class TimeLineSplineHandler
{
	TIMELINE_SPLINE_HANDLER_FREE = 0,
	TIMELINE_SPLINE_HANDLER_ALIGNED,
	TIMELINE_SPLINE_HANDLER_ALIGNED_SYMMETRIC,
	TIMELINE_SPLINE_HANDLER_Count
};

enum class TimeLineMouseSelectionType
{
	TIMELINE_MOUSE_SELECTION_TYPE_NONE = 0,
	TIMELINE_MOUSE_SELECTION_TYPE_POINT,
	TIMELINE_MOUSE_SELECTION_TYPE_MOVE_POINT,
	TIMELINE_MOUSE_SELECTION_TYPE_CONTROL_POINT,
	TIMELINE_MOUSE_SELECTION_TYPE_RECTANGLE,
	TIMELINE_MOUSE_SELECTION_TYPE_GRABBER_MOVING,
	TIMELINE_MOUSE_SELECTION_TYPE_Count
};

class UploadableUniform
{
public:
	float movingFrame = 0.0f; // sert just pour le depalcement de keys avec frame offset�s a la souris

	std::string uniformName;

	//bool selected = false;
	uType::uTypeEnum glslType = uType::uTypeEnum::U_VOID;
	int useFloat[4] = { 0,0,0,0 }; // mit a 1 si utilis�
    float xyzw[4]             = {0.0f, 0.0f, 0.0f, 0.0f};
    int useBool[4]            = {0, 0, 0, 0};
    int useInt[4]             = {0, 0, 0, 0};
    int ixyzw[4]             = {0, 0, 0, 0};
	int useMat = 0;
	glm::mat2 mat2 = glm::mat2(1.0f);
	glm::mat3 mat3 = glm::mat3(1.0f);
	glm::mat4 mat4 = glm::mat4(1.0f);

	//float fxyzw[4] = { 0.0f,0.0f, 0.0f, 0.0f };
	//bool bool0123[4] = { false, false, false, false };
	//int ixyzw[4] = { 0, 0, 0, 0 };
	//glm::mat4 m = glm::mat4(1.0f);

	ct::fvec2 bezierControlStartPoint = ct::fvec2(1.0f, 0.0f);
	ct::fvec2 bezierContorlEndPoint = ct::fvec2(1.0f, 0.0f);

	TimeLineHandlerType timeLineHandlerType = TimeLineHandlerType::TIMELINE_HANDLER_TYPE_CONTROL_POINT_BOTH;

public:
    UploadableUniform();
	void copy(std::shared_ptr<UploadableUniform> v);
	void copyValuesForTimeLine(UniformVariantPtr v);
	bool GetValue(float* val);
	bool GetValue(float* val, int vChannel);
	bool SetValue(float val, int vChannel);
};

class UniformTimeKey
{
public:
	std::string uniformName;
	std::string widget;
	uType::uTypeEnum glslType = uType::uTypeEnum::U_VOID;

	// component, frame, uniform value
	std::map<int, std::map<int, std::shared_ptr<UploadableUniform>>> keys;

	// calculated values / activations
	// uniform value (interpolated)
	std::unordered_map<int, std::shared_ptr<UploadableUniform>> values;

	// l'interpolation a utilsier depend des point de chaque segments. si les point ont des points de controls ou pas
	//TimeLineSegmentInterpolation interpolationMode = TimeLineSegmentInterpolation::TIMELINE_INTERPOLATION_SPLINE_CUBIC;

	// interpolation spline
	bool splineStartButtonDown = false;
	bool splineEndButtonDown = false;
	int currentStartButton = 0;
	int currentEndButton = 0;
	int currentAxisButton = 0;
	std::shared_ptr<UploadableUniform> currentEditedPoint = nullptr;

public:
	UniformTimeKey();
};

class TimeLineInfos
{
public:
	ct::ivec2 rangeFrames = ct::ivec2(0, 90);

	// TimeLine +> uniform name, struct (contain keys, values, activations)
	std::map<std::string, UniformTimeKey> timeLine;

public:
    TimeLineInfos() {
        rangeFrames = ct::ivec2(0, 90);
    }
};

class RenderPack;
class ShaderKey;
class CodeTree;
struct ImGuiContext;
struct ImRect;
class TimeLineSystem : public conf::ConfigAbstract, public RenderingInterface
{
private:
	bool puActive = false; // montre le panneau du timeline
	bool puUploadToGpu = true; // permet l'upload des uniforms dans le gpu
	int puCurrentFrame = 0;
	int puFrameRate = 30; // 30 frame per seconds
	long puFrameRateInMS = 33; // 1000 / puFrameRate
	bool puFrameChanged = false; // frame changed
	ct::ActionTime puAnimationTimer; // framed rate animation
	float puTimeLineScale=1; // timeline scale
	// control
	bool puRecord = false; // permet l'upload des uniforms dans le gpu
	bool puPlayTimeLineReverse = false;
	bool puPlayTimeLine = false;

	// uniform name, bit mask
	std::unordered_map<std::string, int> puUniformsToEdit;
	int puTimeLineItemAxisMasks[4] = {
		TimeLineItemAxis::TIMELINE_AXIS_RED,
		TimeLineItemAxis::TIMELINE_AXIS_GREEN,
		TimeLineItemAxis::TIMELINE_AXIS_BLUE,
		TimeLineItemAxis::TIMELINE_AXIS_ALPHA
	};

	// timeline widget
	float puPaneWidth = 150;
	int puStepScale = 10; // saut entre chaque step
	float puStepSize = 50; // espace entre points
	float puTextOffsetY = 6;
	float putimeBarHeight = 30;
	bool puShowUnUsed = false;
	bool puShowControlPoints = true;
	bool puShowInterpolatedPoints = true;

	// curve graph widget
	float pucurveStepScale = 1; // saut entre chaque step
	float pucurveStepSize = 50; // espace entre points
	float pucurveBarWidth = 50;

	TimeLineSplineHandler puBezierHandler = TimeLineSplineHandler::TIMELINE_SPLINE_HANDLER_ALIGNED_SYMMETRIC;

	// Mouse Selection
	TimeLineMouseSelectionType puUseMouseSelectionType = TimeLineMouseSelectionType::TIMELINE_MOUSE_SELECTION_TYPE_NONE;
	bool puMouseStartOverButton = false;
	bool puMouseOverButton = false;
	bool puMouseHasDragged = false;
	ct::fvec2 puStartMouseClick;
	ct::fvec2 puEndMouseClick;
	std::unordered_set<std::shared_ptr<UploadableUniform>> puSelectedKeys;
	//std::shared_ptr<UploadableUniform> puSelectedKey = 0;

private: // temp var for display
	float puPaneOffsetX = 200.0f; // view offset x
	float puPaneOffsetY = 50.0f; // view offset x
	float puUniformsPaneOffsetY = 0.0f; // uniform names offset y

private:
	ShaderKeyPtr puActiveKey = nullptr;

private: // ImGui Style
	ImVec4 puThickLinesDark;
	ImVec4 puThickLinesLight;
	ImVec4 puRangeFrame;
	ImVec4 puFrameBg;

public:
	static TimeLineSystem* Instance()
	{
		static TimeLineSystem _instance;
		return &_instance;
	}

protected:
	TimeLineSystem(); // Prevent construction
	TimeLineSystem(const TimeLineSystem&) {}; // Prevent construction by copying
	TimeLineSystem& operator =(const TimeLineSystem&) { return *this; }; // Prevent assignment
	~TimeLineSystem(); // Prevent unwanted destruction
	
public:
	void ClearLocalVar();

public:
	ct::ivec4 ShowUI(ImGuiContext *vContext, const ct::ivec4& vSize, RenderPackWeak vRenderPack, bool *vChange);
	void SetActive(bool vFlag);
	bool IsActive();
	bool CanWeRecord();
	bool IsPlaying();
	void SetActiveKey(ShaderKeyPtr vKey);
	ShaderKeyPtr GetActiveKey();
	void Resize(ct::ivec2 vNewSize);
	bool DoAnimation_WithoutUiTriggering(ShaderKeyPtr vKey);
	bool UpdateUniforms(ShaderKeyPtr vKey, UniformVariantPtr vUniPtr);

public: // key manage
	void AddShaderKeyForCurrentFrame(ShaderKeyPtr vKey);
	void DelShaderKeyForCurrentFrame(ShaderKeyPtr vKey);
	void AddIncludeKeyForCurrentFrame(CodeTreePtr vCodeTree, std::string vIncludeKey);
	void DelIncludeKeyForCurrentFrame(CodeTreePtr vCodeTree, std::string vIncludeKey);
	void AddKeyForCurrentFrame(ShaderKeyPtr vKey, UniformVariantPtr v, int vComponent);
	void DelKeyForCurrentFrame(ShaderKeyPtr vKey, UniformVariantPtr v, int vComponent);
	void DelKeyForCurrentFrame(ShaderKeyPtr vKey, std::string vUniformName, int vComponent, int vFrame);
	void DelUniform(ShaderKeyPtr vKey, std::string vUniformName);
	void DelUniformComponent(ShaderKeyPtr vKey, std::string vUniformName, int vComponent);
	bool IsKeyExist(ShaderKeyPtr vKey, UniformVariantPtr v, int vComponent);
	bool IsKeyExist(ShaderKeyPtr vKey, std::string vUniformName, int vComponent);
	bool IsKeyExistForCurrentFrame(ShaderKeyPtr vKey, UniformVariantPtr v, int vComponent);
	bool IsKeyExistForCurrentFrame(ShaderKeyPtr vKey, std::string vUniformName, int vComponent);
	bool IsKeyExistForFrame(ShaderKeyPtr vKey, std::string vUniformName, int vComponent, int vFrame);
	void ReSizeControlPoints(ShaderKeyPtr vKey, std::string vUniformName, int vComponent, int vOldFrame, std::vector<int> vOldFrameKeys, int vNewFrame, std::vector<int> vNewFrameKeys);
	std::vector<int> GetFramesToList(ShaderKeyPtr vKey, std::string vUniformName, int vComponent);
	
public: // mouse selection
	void DelSelectedKeys(ShaderKeyPtr vKey);
	void DeselectAllKeys(ShaderKeyPtr vKey);
	void SelectAllKeysInMouseSelectionRect(ShaderKeyPtr vKey, ImRect vZone);
	void SelectIfContainedInMouseSelectionRect(std::shared_ptr<UploadableUniform> vStruct, ImVec2 vPoint);
	void ImGui_DrawMouseSelectionRect(ShaderKeyPtr vKey, ImVec4 vColor, float vThick);
	void MoveSelectedKeysWithMouse(ShaderKeyPtr vKey, ImRect vZone, ct::fvec2 vStartPos, ct::fvec2 vEndPos);
	void ShowMouseInfos(ShaderKeyPtr vKey, ImRect vZone, ct::fvec2 vMousePos);
	void AddKeyToSelection(std::shared_ptr<UploadableUniform> vStruct);
	void RemoveKeyFromSelection(std::shared_ptr<UploadableUniform> vStruct);
	bool IsKeyInSelection(std::shared_ptr<UploadableUniform> vStruct);
	bool IsSelectionExist();
	bool DoSelection(ShaderKeyPtr vKey, ImGuiID vId, ImRect vWidgetZone, ImRect vViewZone, ImRect vTimebarZone, ImRect vValueBarZone);

public: // Interpolation 
	TimeLineSegmentInterpolation GetTimeLineSegmentInterpolationMode(std::shared_ptr<UploadableUniform> vStart, std::shared_ptr<UploadableUniform> vEnd);
	float Interpolate_Linear(const float& vStart, const float& vEnd, const float& vRatio);
	float Interpolate_Quadratic(
		const int& vStartFrame, const float& vStartValue, std::shared_ptr<UploadableUniform> vStart,
		const int& vEndFrame, const float& vEndValue, std::shared_ptr<UploadableUniform> vEnd,
		const float& vRatio, bool vUseDerivation);
	float Interpolate_Bezier(
		const int& vStartFrame, const float& vStartValue, std::shared_ptr<UploadableUniform> vStart,
		const int& vEndFrame, const float& vEndValue, std::shared_ptr<UploadableUniform> vEnd, 
		const float& vRatio, bool vUseDerivation);
	float Interpolate(
		UniformTimeKey *vTimeKey,
		const int& vStartFrame, const float& vStartValue, std::shared_ptr<UploadableUniform> vStart,
		const int& vEndFrame, const float& vEndValue, std::shared_ptr<UploadableUniform> vEnd, const float& vRatio,
                      bool vUseDerivation = false);

public: // Interpolation Computation
    void ReComputeInterpolation(ShaderKeyPtr vKey);
	void ReComputeInterpolation(ShaderKeyPtr vKey, std::string vUniformName);
	void ReComputeInterpolation(TimeLineInfos *vTimeLineInfos, std::string vUniformName);
	void ReComputeInterpolation(ShaderKeyPtr vKey, std::string vUniformName, int vComponent);
	void ReComputeInterpolation(TimeLineInfos *vTimeLineInfos, std::string vUniformName, int vComponent);
	void ReComputeInterpolation(UniformTimeKey *vUniformTimeKeyStruct, int vComponent);

public: // load save
	TimeLineInfos LoadTimeLineConfig(std::string vConfigFile);
	bool SaveTimeLineConfig(std::string vConfigFile, TimeLineInfos vTimeLineInfos);
	
	std::string getXml(const std::string& vOffset, const std::string& vUserDatas) override;
    bool setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas) override;

public: // Rendering flag
    std::string GetRenderingFilePathNameForCurrentFrame() override;

public:
	bool DrawBar(ShaderKeyPtr vKey, ct::ivec2 vScreenSize);
	bool DrawTimeLine(const char* label, ShaderKeyPtr vKey);

private:
	void ShowHelpPopup();
	void ShowDialog(ShaderKeyPtr vKey, ct::ivec2 vScreenSize);
	void GoToNextKey(ShaderKeyPtr vKey, int vCurrentFrame);
	void GoToPreviousKey(ShaderKeyPtr vKey, int vCurrentFrame);
	void GoToFrame(int vFrame);
	void ScaleTimeLine(ShaderKeyPtr vKey, float vScale);
	void ReArrangeForViewContent(ShaderKeyPtr vKey, ImRect vZone, float *vPaneOffsetX, float *vPaneOffsetY);
	void DoLoopLimitKeys(ShaderKeyPtr vKey, bool vStartOrEnd, bool vSamePos, bool vSameGradient, bool vSameGradientLength);
	void ChangeHandlerTypeOfSelectedKeys(ShaderKeyPtr vKey, TimeLineHandlerType vTimeLineHandlerType);

public: // frame/value scales converter
	int GetLocalPosFromFrame(const int& vFrame);
	float GetLocalPosFromFrame(const float& vFrame);
	float GetLocalPosFromValue(const float& vValue);
	float GetValueFromLocalPos(const float& vPos);
	int GetFrameFromLocalPos(const int& vPos);
	float GetFrameFromLocalPos(const float& vPos);
	ct::fvec2 GetFrameValueFromLocalPos(const ct::fvec2& vPos);
	ct::fvec2 GetLocalPosFromFrameValue(const ct::fvec2& vFrameValue);
	ct::fvec2 GetFrameValueFromWorldPos(const ct::fvec2& vPos, const ImRect& vZone);
	ct::fvec2 GetWorldPosFromFrameValue(const ct::fvec2& vFrameValue, const ImRect& vZone);

private: // ui
	bool ImGui_AutoKeyingButton(const char *vName, bool *vAutoKeying);
	bool ImGui_DrawTrashButton(bool  vHovered, const ImVec2& pos);
	bool ImGui_DrawCheckButton(bool  vHovered, const ImVec2& pos, bool vFlag);
	bool ImGui_DragFloat(const float width, const char* label, const char* help, float* value);
	bool ImGui_DragInt(const float width, const char* label, const char* help, int* value);
    bool ImGui_AbortButton(const char *vName);
	void ImGui_DrawGrabber(const ImGuiID& vId, const int& vFrame, const int& vOffset, const ImRect& frame_bb);
	bool ImGui_DrawTimeLinePointList(ShaderKeyPtr vKey, float offsetY, float paneOffsetX, float paneOffsetY, ImRect frame_bb, int startFrame, int endFrame, bool hovered);
	bool ImGui_DrawTimeLineCurveGraph(ShaderKeyPtr vKey, float offsetY, float paneOffsetX, float paneOffsetY, ImRect frame_bb, int startFrame, int endFrame, bool hovered);
	bool ImGui_DrawHTimeBar(float paneOffsetX, ImRect frame_bb, int countFrames, int startFrame);
	void ScaleViewForFrames(int vCountFrames, float vMouseWheel);
	bool ImGui_DrawVValueBar(float paneOffsetY, ImRect frame_bb);
	void ScaleViewForValues(ImRect vFrameBb, float vMouseWheel);
	bool ImGui_DrawButton(const char* vLabel, const char* vHelp, ImVec2 vStart, ImVec2 vEnd, bool vHovered);
	void ImGui_DrawGraphPoint(ImVec2 vPoint, ImVec4 vColor);
	bool ImGui_DrawGraphPointButton(
		UniformTimeKey *vKeyStruct, std::shared_ptr<UploadableUniform> vStruct, int vFrame, ImVec2 vPoint, bool vHovered);
	bool ImGui_DrawGraphSplinePointButton(
		UniformTimeKey *vKeyStruct, int vFrame, ImVec2 vBasePoint, ImVec2 vControlPoint,
		std::shared_ptr<UploadableUniform> vStruct, int vCountFrames, bool vIsStartOrEnd, int vAxis, ImRect vFrame_bb);
	bool ImGui_DrawGraphLineButton(
		UniformTimeKey *vKeyStruct, 
		int vLastFrame, int vFrame,
		ImVec2 vStartPoint, ImVec2 vEndPoint,
		std::shared_ptr<UploadableUniform> vStartStruct, std::shared_ptr<UploadableUniform> vEndStruct,
		ImVec4 vColor, ImRect vFrame_bb, int vAxis);
	
private: // curve graph show/hide
	void ShowCurveForUniform(ShaderKeyPtr vKey, std::string vUniformName);
	void HideCurveForUniform(std::string vUniformName);
	void ShowCurveForUniformAxis(std::string vUniformName, int vAxis);
	void HideCurveForUniformAxis(std::string vUniformName, int vAxis);
	bool IsShowingCurves(ShaderKeyPtr vKey);

private:
	void Clean_All(ShaderKeyPtr vKey);
	void Clean_KeepOnlyChangedUniforms(ShaderKeyPtr vKey);
};
