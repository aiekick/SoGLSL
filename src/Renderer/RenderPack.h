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

#include <Headers/RenderPackHeaders.h>

#include <Renderer/Shader.h>
#include <Mesh/Model/BaseModel.h>
#include <Uniforms/UniformVariant.h>
#include <ctools/ConfigAbstract.h>
#include <Buffer/ExportBuffer.h>
#include <Buffer/FrameBuffersPipeLine.h>
#include <Renderer/CommandBuffer.h>
#include <Buffer/FloatBuffer.h>

#include <Mesh/Gui/GuiModel.h>

#include <string>

#include <Systems/Interfaces/CameraInterface.h>
#include <Systems/Interfaces/MouseInterface.h>

#include <CodeTree/CodeTree.h>

#include <atomic>
#include <unordered_map>

//#define CONFIG_MARK_START "[[["
//#define CONFIG_MARK_END "]]]"

class BufferCallMap
{
private:
	// bien utiliser des shared et non des weaks car la 
	// on sauve les childs sinon ils seront detruits
	std::unordered_map<std::string, RenderPackPtr> dico;
	std::list<RenderPackWeak> iterable_buffers;

public:
	std::list<RenderPackWeak>::iterator begin();
	std::list<RenderPackWeak>::iterator end();
	size_t size();
	bool empty();
	void clear();
	bool exist(const std::string& vName);
	void erase(const std::string& vName);
	void add(const std::string& vName, RenderPackPtr vBuffer);
	RenderPackWeak get(const std::string& vName);
	void finalize();
};

class RenderPack : public conf::ConfigAbstract
{
public:
	static RenderPackPtr Create(const GuiBackend_Window& vWin);
	static RenderPackPtr createComputeWithFileWithoutLoading(
		const GuiBackend_Window& vWin, const std::string& vName, ct::ivec3 vSize, ShaderKeyPtr vShaderKey);
	static RenderPackPtr createComputeWithFile(
		const GuiBackend_Window& vWin, const std::string& vName, ct::ivec3 vSize, ShaderKeyPtr vShaderKey);

	static RenderPackPtr createBufferWithFileWithoutLoading(
		const GuiBackend_Window& vWin, const std::string& vName, ct::ivec3 vSize, ShaderKeyPtr vShaderKey,
		bool vUseZBuffer, bool vUseFBO, bool vUseRenderBuffer = false, bool vUseFloatBuffer = false);
	static RenderPackPtr createBufferWithFile(
		const GuiBackend_Window& vWin, const std::string& vName, ct::ivec3 vSize, ShaderKeyPtr vShaderKey,
		bool vUseZBuffer, bool vUseFBO, bool vUseRenderBuffer = false, 
		TextureParamsStruct *vTexParam = nullptr, bool vUseFloatBuffer = false);
		
	static ct::frect GetScreenRectWithSize(ct::ivec2 vItemSize, ct::ivec2 vMaxSize);
	static ct::frect GetScreenRectWithRatio(float vRatio, ct::ivec2 vMaxSize);

	
private: 
	RenderPackWeak m_This;
	FrameBuffersPipeLinePtr puFrameBuffer = nullptr;
	ShaderPtr puShader = nullptr;
	BaseModelPtr puModel_Render = nullptr;
	ShaderKeyPtr puShaderKey = nullptr;
	std::shared_ptr<ExportBuffer> puExportBuffer = nullptr;
	std::shared_ptr<RecordBuffer> puRecordBuffer = nullptr;
	
	CommandBuffer m_CommandBuffer;

	RenderPack_Type puRenderPackType = RenderPack_Type::RENDERPACK_TYPE_BUFFER;

	bool puSomeErrorsFromLastShader = false;
	bool puSomeWarningsFromLastShader = false;
	bool puCanWeRender = false;
	bool puNewCompilationNeeded = false;
	std::string puCurrentShaderName;
	std::string puLastCompiledShaderName;

	///////////////////////////////////////
	///////////////////////////////////////
	///////////////////////////////////////

	int puCountAttachments = 0;
	bool puBufferIdsToShow[8] = { true, true, true, true, true, true, true, true };

	///////////////////////////////////////
	// FRAME //////////////////////////////
	///////////////////////////////////////

	int puFrameIdx = 0;
	float puLastRenderTime = 0.0f;

	///////////////////////////////////////
	// RECORDING //////////////////////////
	///////////////////////////////////////
	
	BaseMeshEnum puMeshType = BaseMeshEnum::PRIMITIVE_TYPE_NONE;
	std::string puMeshTypeString;
	//std::string puMeshFilePathName;

	///////////////////////////////////////
	// RENDERING MODE LIST ////////////////
	///////////////////////////////////////
	std::vector<std::string> puDisplayModeArray;
	int puDisplayModeArrayIndex = 0;

	bool puLoaded = false;

public:
	bool puCanWeTuneMouse = false;
	bool puCanWeTuneCamera = false;
	bool puCanBeIntegratedInExternalPipeline = true;

public:
	GuiBackend_Window puWindow;

	RenderPackPtr puMainRenderPack = nullptr;
	BufferCallMap puBuffers;		// for working with input (mouse) buffer must be in normal order from shadertoy A>B>C>D
	BufferCallMap puSceneBuffers;  // for working with input (mouse) buffer must be in normal order from shadertoy A>B>C>D

	bool puDontSaveConfigFiles = false;

	///////////////////////////////////////
	// FBO RESIZE AND DISPLAY /////////////
	///////////////////////////////////////

	//std::string puInFileBufferName; // le buffer name in file de ce renderpack
	SectionConfigStruct puSectionConfig;

	ct::frect puMeshRect; // affichage du rect du fbo, x,y,z,w x,y origin, z,w size 
	
	bool puScreenRectAlreadyCalculated = false; // dela calcul�
	ct::ivec3 puMaxScreenSize; // surface max de l'ecran de l'app

	///////////////////////////////////////////
	// TEX PARAMS /////////////////////////////
	// doit etre idem pour les 8 attachments //
	// sous peine de veroller le fbo //////////
	///////////////////////////////////////////

	bool puTexParamsAlreadyInit = false; // init des arrayindex des combobox c'est tout
	bool puTexParamCustomized = false; // si le buffer est concern�n par une config
	TextureParamsStruct puTexParams;

	std::vector<std::string> puFragColorNames;
	int puZeroBasedMaxSliceBufferId = 0; // max 8 => 0 to 7

	GLenum puGeometryOutputRenderMode;
	GLenum puGeometryInputRenderMode;

	//settings
	ct::uvec4 puCountVertices = ct::uvec4(0U); // inf, sup, default, value
	ct::uvec4 puCountInstances = ct::uvec4(0U); // inf, sup, default, value
	ct::uvec4 puCountIterations = ct::uvec4(1U, 100U, 1U, 1U); // inf, sup, default, value
	ct::uvec4 puCountFrameToJump = ct::uvec4(0U, 100U, 0, 1U); // inf, sup, default, value
	ct::uvec4 puPatchsCountVertices = ct::uvec4(0U); // inf, sup, default, value
	ct::uvec4 puCountIndicesToShow = ct::uvec4(0U); // inf, sup, default, value
	GLenum puLastLastRenderMode = 0;
	GLenum puLastRenderMode = 0;
	bool puCreateZBuffer = false;
	bool puUseFXAA = false;
	int puCountFXAASamples = 0;
	bool puUseFloatBuffer = false;
	std::string puTracyRenderShaderName;
	std::string puName;
	bool puUseFBO = false;
	bool puUseRBuffer = false;
	bool puUsePointSize = false;

	// imgui
	bool puShowZBufferButton = false;
	bool puShowTransparentButton = false;
	bool puShowCullingButton = false;
	bool puShowBlendingButton = false;
	bool puShowOpenModelButton = false;

	// shader
	ShaderNoteStruct puLastShaderNote;
	ShaderParsedStruct puLastShaderCode;

	int puMeshFrontFace = 0;
	int puMeshCullFace = 0;
	int puMeshDepthFunc = 0;
	int puMeshBlendFuncSource = 0;
	int puMeshBlendFuncDestination = 0;
	int puMeshBlendEquation = 0;

	GLenum puFrontFace = 0;
	GLenum puCullFace = 0;
	GLenum puDepthFunc = 0;
	GLenum puBlendSFactor = 0;
	GLenum puBlendDFactor = 0;
	GLenum puBlendEquation = 0;

	ct::ivec2 puLastFBOValuePosRead;

	uint32_t prCurrentIteration = 0U;
	uint32_t prCountIterations = 1U;
	uint32_t prCountFramesToJump = 0U;

	GuiModel m_GuiModel;

public:
	bool IsLoaded() { return puLoaded; }
	ShaderKeyPtr GetShaderKey() { return puShaderKey; }
	SyntaxErrors* GetSyntaxErrors() { if (puShaderKey) return &puShaderKey->puSyntaxErrors; return nullptr; }
	void SetCanWeRender(bool vCanWeRender) { puCanWeRender = vCanWeRender; }
	bool GetCanWeRender() { return puCanWeRender; }
	int GetFBOId() { if (puFrameBuffer != nullptr) return puFrameBuffer->getBackFboID(); return -1; }
	FrameBuffersPipeLinePtr GetPipe() { return puFrameBuffer; }
	BaseModelWeak GetModel() { return puModel_Render; }
	std::shared_ptr<RecordBuffer> GetRecordBuffer() { return puRecordBuffer; }
	
	ct::ivec3 GetSize() { return puMaxScreenSize; }
	int GetTextureId(int i = 0) { if (puFrameBuffer != nullptr) return puFrameBuffer->getBackTextureID(i); return -1; }
	ctTexturePtr GetTexture(int i = 0) { if (puFrameBuffer != nullptr) return puFrameBuffer->getBackTexture(i); return nullptr; }
	int GetRenderBufferId(int i = 0) { if (puFrameBuffer != nullptr) return puFrameBuffer->getBackRenderBufferID(i); return -1; }
	bool IsUseZBuffer() { return puShaderKey->puShaderGlobalSettings.useZBuffer; }
	ShaderPtr GetShader() { return puShader; }
	bool IsNewCompilationNeeded() { return puNewCompilationNeeded; }
	void NeedNewCompilation() { puNewCompilationNeeded = true; }
	RenderPack_Type GetRenderPackType() { return puRenderPackType; }

	GuiBackend_Window GetGuiBackendWindow() { return puWindow; }

private: // DrawBufferParams()
	int _CurrentSelectedBufferidToTune = 0;
	int _BufferWrap = 0;
	int _BufferFilter = 0;
	int _BufferFormat = 0;

public:
	RenderPack(const GuiBackend_Window& vWin);
	~RenderPack();

	void Clear();

	bool Load(const std::string& vInFileBufferName = "", bool vReplaceRenderPackName = false);
	void LoadMeshFileFromVertexSection();

	////////////////////////////////////////////////
	// GPU
	////////////////////////////////////////////////
	bool BindFBO(
		FrameBuffersPipeLinePtr vPipe = nullptr,
		const bool& vDontUseAnyFBO = false);
	bool RenderShader(GLenum* vRenderMode = nullptr, FrameBuffersPipeLinePtr vPipe = nullptr);
	void UploadUniforms();
	bool TransformFeedbackShader();
	bool PixelShader(FrameBuffersPipeLinePtr vPipe);
	bool ComputeShader();
	void UnBindFBO(
		const bool& vUpdateMipMap = true,
		const bool& vSwitchBuffers = true,
		FrameBuffersPipeLinePtr vPipe = nullptr,
		const bool& vDontUseAnyFBO = false);
	void UpdateMipMap();
	void UploadMesh(VertexStruct::P3_N3_T2_C4* vPoints, int vCountPoints, VertexStruct::I1* vIndices, int vCountIndices);

	////////////////////////////////////////////////
	// CPU
	////////////////////////////////////////////////
	bool RenderNode(
		GLenum* vRenderMode = nullptr, 
		FrameBuffersPipeLinePtr vPipe = nullptr,
		std::atomic<size_t>* vCurrentIteration = nullptr, 
		std::atomic<bool>* vWorking = nullptr, 
		const bool& vDontUseAnyFBO = false); // pour les threads
	////////////////////////////////////////////////
	void UpdateInfileBufferName(const std::string& vInFileBufferName);
	void SetComputeDefaultParams(
		const std::string& vName, ct::ivec3 vSize, ShaderKeyPtr vShaderKey);
	void SetBufferDefaultParams(
		const std::string& vName, ct::ivec3 vSize, ShaderKeyPtr vShaderKey,
		bool vUseZBuffer, bool vUseFBO, bool vUseRenderBuffer = false, bool vUseFloatBuffer = false);
	void SetShaderKey(ShaderKeyPtr vShaderKey, bool vReplaceRenderPackName = false);
	void UpdatePreDefinedUniforms(std::list<UniformVariantPtr> vUniforms);
	// vMouseRect : xy => screen mouse pos / zw => rect screen size 
	void UpdateUniforms(
		UpdateUniformFuncSignature vSpecificUniformsFunc,
		DisplayQualityType vDisplayQuality = 1.0f, 
		MouseInterface *vMouse = nullptr,
		CameraInterface* vCamera = nullptr,
		ct::ivec2 vScreenSize = ct::ivec2(),
		MeshRectType* vMouseRect = nullptr);
	void UpdateUniform(UniformVariantPtr vUniPtr, DisplayQualityType vDisplayQuality, MouseInterface* vMouse, CameraInterface* vCamera, MeshRectType* vMouseRect = nullptr);
	void UpdateMouseWidgets(UniformVariantPtr vUniPtr, DisplayQualityType vDisplayQuality, MouseInterface* vMouse, CameraInterface* vCamera, MeshRectType* vMouseRect = nullptr);
	void UpdateTimeWidgets(float vDeltaTime);

	bool Resize(ct::ivec3 vNewSize, bool vForceResize);
	
	void UpdateSectionConfig(const std::string& vInFileBufferName = "");
	bool ParseAndCompilShader(const std::string& vInFileBufferName = "", const GuiBackend_Window& vWin = GuiBackend_Window());
	bool UpdateShaderChanges(bool vForceUpdate, std::string vForceUpdateIfReplaceCodeKeyIsPresent = "");
	
	// Buffer
	void CreateOrUpdatePipe(FrameBufferShaderConfigStruct vFrameBufferShaderConfigStruct);
	bool ResizeWithFramebufferConfig(ct::ivec3 vNewScreenSize, bool vForceResize, FrameBufferShaderConfigStruct vFrameBufferShaderConfigStruct);
	
	// Compute : a voir si on en a besoin
	//void CreateOrUpdateCompute(ComputeShaderConfigStruct vComputeShaderConfigStruct);
	//bool ResizeWithComputeConfig(ct::ivec3 vNewScreenSize, ComputeShaderConfigStruct vComputeShaderConfigStruct);

	void ClearColor();

	std::shared_ptr<FloatBuffer> GetFloatBuffer(int vAttachmentId, bool vReadFBO, int vMipMapLvl);
	
	void UpdateCountVertex(uint32_t vCountVertexs);
	void UpdateCountIndices(uint32_t vCountIndexs);
	void UpdateCountInstances(uint32_t vCountInstances);
	void UpdateCountPatchVertices(uint32_t vCountPatchCountVertices);

	bool SaveFBOToPng(const std::string& vFilePathName, bool vFlipY, int vSubSamplesCount, ct::ivec2 vNewSize, int vAttachmentId);
	bool SaveFBOToBmp(const std::string& vFilePathName, bool vFlipY, int vSubSamplesCount, ct::ivec2 vNewSize, int vAttachmentId);
	bool SaveFBOToHdr(const std::string& vFilePathName, bool vFlipY, int vSubSamplesCount, ct::ivec2 vNewSize, int vAttachmentId);
	bool SaveFBOToJpg(const std::string& vFilePathName, bool vFlipY, int vSubSamplesCount, int vQualityFrom0To100, ct::ivec2 vNewSize, int vAttachmentId);
	bool SaveFBOToTga(const std::string& vFilePathName, bool vFlipY, int vSubSamplesCount, ct::ivec2 vNewSize, int vAttachmentId);

	// tesselation
	void InitCountPatchVertices();
	void ChangeCountPatchVertices(uint32_t vNewvPatchCountVertices);
	bool DrawCountPatchVerticesSlider(float vWidth);

	void ChangeCountMeshPoints(uint32_t vNewCountPoints);
	bool DrawCountMeshPointsSlider(float vWidth);

	void InitCountMeshIndicesFromModel();
	void ChangeCountMeshIndicesToShow(uint32_t vNewCountPoints);
	bool DrawCountMeshIndicesToShowSlider(float vWidth);

	void ChangeCountMeshInstances(uint32_t vNewCountInstances);
	bool DrawCountMeshInstancesSlider(float vWidth);

	void ChangeCountRenderingIterations(uint32_t vNewCountRenderingIterations);
	bool DrawCountRenderingIterationsSlider(float vWidth);

	void ChangeCountFramesToJump(uint32_t vNewCountFramesToJump);
	bool DrawCountFramesToJumpSlider(float vWidth);

	void SetFXAAUse(bool vUseFXAA, int vCountFXAASamples);

	void AddCustomWidgetNameAndPropagateToChilds(uType::uTypeEnum vGlslType, const std::string& vName, int vArrayCount = 0);
	void PropagateCustomWidgetNamesToChilds();

	void ClearBuffers(bool vSaveConfig);
	bool DestroyChildBuffer(const std::string& vBufferId);
	RenderPackPtr CreateChildBuffer(const std::string& vBufferId, const std::string& vBufferFileName, const std::string& vInFileBufferName = "");
	RenderPackPtr CreateChildCompute(const std::string& vBufferId, const std::string& vBufferFileName);
	RenderPackPtr CreateSceneBuffer(const std::string& vBufferId, const std::string& vBufferFileName, const std::string& vInFileBufferName = "");

	void ResetFrame();
	void ResetTime();
	void ResetSoundPlayBack();

	void SetLineWidth(float vThick);
	float GetLineWidth();

	bool IsUsingSmoothLine();

	bool DrawCountSliders();
	bool DrawComboBoxs();
	bool DrawRenderingOptions(bool vExpertMode);
	bool DrawRenderingButtons(const char *vRenderPackString);
	bool DrawImGuiUniformWidget(float vFirstColumnWidth);
	bool CollapsingHeader(const char* vLabel, bool vForceExpand = false, bool vShowEditButton = false, bool *vEditCatched = nullptr);
	bool DrawDialogsAndPopups(ct::ivec2 vScreenSize);

	void Init_Blending();
	bool DrawRenderingOptions_BlendFunc_Src();
	bool DrawRenderingOptions_BlendFunc_Dst();
	bool DrawRenderingOptions_BlendFunc_Equation();

	void Init_Depth();
	bool DrawRenderingOptions_DepthFunc();

	void Init_CullFace();
	bool DrawRenderingOptions_CullFace();

	ModelRenderModeEnum GetLastRenderMode_NoGeopuNoTess();
	bool DrawRenderingOptions_Model();
	bool DrawRenderingOptions_FrameBuffer();

	bool DrawBufferParams(bool vUseMipMapChecking);
	bool DrawFeedBackBufferParams();
	bool DrawTextureParamComboBox(bool vMipMap = true, bool vS = true, bool vT = true, bool vMin = true, bool vMag = true);
	void ChangeTexParameters(TextureParamsStruct *vTexParams);
	void GetTexParameters(TextureParamsStruct *vTexParams);
	void ConfigureBufferParams(std::string vFormat = "float", bool vMipMap = false, int vMaxMipMaplvl = 1000, std::string vWrap = "clamp", std::string vFilter = "linear", bool vReloadFBO = true);

	std::string getXml(const std::string& vOffset, const std::string& vUserDatas) override;
	bool setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas) override;

	void DisplayMessageOfRenderPack(bool vHideWarnings);

	bool IsTheseSomeWarnings();
	bool IsTheseSomeErrors();
	bool IsTheseSomeErrorsOrWarnings();

	void Finish(bool vSaveConfig = true);

	bool GetFBOValuesAtPixelPos(ct::ivec2 vPos, ct::fvec4 *vArr, int vAttachmentCount, bool vIfPosChanged = true);
	bool GetFBOValuesAtNormalizedPos(ct::fvec2 vPos, ct::fvec4 *vArr, int vAttachmentCount, bool vIfPosChanged = true);
	
	//bool GetFBOValuesUnderLineWithPos(ct::fvec2 vStartPos, ct::fvec2 vEndPos, ct::fvec4 *vArr, int vArrCount, int vAttachment);
	//bool GetFBOValuesUnderLineWithNormalizedPos(ct::fvec2 vStartPos, ct::fvec2 vEndPos, ct::fvec4 *vArr, int vArrCount, int vAttachment);
	
	void UpdateAroundModelChange(); // when the model is change

	void SaveRenderPackConfig(CONFIG_TYPE_Enum vConfigType);

private:
	void ReloadModelIfNeeded(bool vNeedUpdate);

	bool InitComputeWithFile(
		const GuiBackend_Window& vWin, const std::string& vName, ct::ivec3 vSize, ShaderKeyPtr vShaderKey);
	bool InitBufferWithFile(
		const GuiBackend_Window& vWin, const std::string& vName, ct::ivec3 vSize, ShaderKeyPtr vShaderKey,
		bool vUseZBuffer, bool vUseFBO, bool vUseRenderBuffer, 
		TextureParamsStruct *vTexParam, bool vUseFloatBuffer);
};
