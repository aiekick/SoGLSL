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

#include "UniformVariant.h"
#include <Renderer/RenderPack.h>
#include <CodeTree/CodeTree.h>
#include <Texture/Texture2D.h>
#include <Texture/TextureCube.h>
#include <Texture/Texture3D.h>
#include <Texture/TextureSound.h>
#include <Profiler/TracyProfiler.h>

#ifdef DEBUG_UNIFORMS
std::vector<std::string> UniformVariant::save = std::vector<std::string>();
#endif

size_t UniformVariant::counter = 0;

UniformVariantPtr UniformVariant::create()
{
	auto res = std::make_shared<UniformVariant>();
	res->m_This = res;
	return res;
}

#ifdef DEBUG_UNIFORMS
UniformVariantPtr UniformVariant::create(void *vParentPtr, const std::string& vName, std::string vDebug)
{
	ZoneScoped;

	UniformVariantPtr res = UniformVariant::create();
	UniformVariant::counter++;
	res->m_This = res;
	res->owner = vParentPtr;
    res->name = vName;
    res->nameForSearch = ct::toLower(vName);
	res->ownerString = vDebug;
	save.emplace_back(res->ownerString + "_" + vName);
	return res;
}
#else
UniformVariantPtr UniformVariant::create(ShaderKeyPtr vParentPtr, const std::string& vName)
{
	ZoneScoped;

	UniformVariantPtr res = UniformVariant::create();
	UniformVariant::counter++;
	res->m_This = res;
	res->owner = vParentPtr;
	res->name = vName;
    res->nameForSearch = ct::toLower(vName);
	return res;
}
#endif

#ifdef DEBUG_UNIFORMS
bool UniformVariant::destroy(UniformVariantPtr vPtr, void *vParentPtr, std::string vDebug)
{
	ZoneScoped;

	if (vPtr)
	{
		if (vPtr->owner && vPtr->owner == vParentPtr)
		{
			std::string todel = vPtr->ownerString + "_" + vPtr->name;
			int idx = -1;
			for (auto it = save.begin(); it != save.end(); ++it)
			{
				idx++;
				if (*it == todel)
					break;
			}
			if (idx >= 0)
				save.erase(save.begin() + idx);

			vPtr->owner = 0;
			delete vPtr;
			UniformVariant::counter--;
			vPtr = 0;
			return true;
		}
	}
	return false;
} 
#else
bool UniformVariant::destroy(UniformVariantPtr vPtr, ShaderKeyPtr vParentPtr)
{
	ZoneScoped;

	if (vPtr)
	{
		if (vPtr->owner && vPtr->owner == vParentPtr)
		{
			vPtr->owner = nullptr;
			vPtr.reset();
			vPtr = nullptr;
			UniformVariant::counter--;
			return true;
		}
		else
		{
			//CTOOL_DEBUG_BREAK;
		}
	}
	return false;
}
#endif

UniformVariant::UniformVariant()
{
	ZoneScoped;

	owner = nullptr;
	parentMultiLoc = nullptr;

	canWeSave = true;
    noExport = false;
	constant = false;
	pipe = nullptr;
	record = nullptr;
	for (int i = 0; i < 8; i++) id[i] = 0;
	loc = -1;
	slot = 0;
	name.clear();
	widget.clear();
	widgetType.clear();
	glslType = uType::uTypeEnum::U_VOID;

	text.clear();
	uiOnly = false;

	x = 0;
	y = 0;
	z = 0;
	w = 0;
	bx = false;
	by = false;
	bz = false;
	bw = false;
	ix = 0;
	iy = 0;
	iz = 0;
	iw = 0;
	ux = 0;
	uy = 0;
	uz = 0;
	uw = 0;
	count = 1;
	IsIntegrated = false;
	inf = 0.0f;
	sup = 1.0f;
	def = 0.0f;
	step = 0.0f;
	uFloatArr = nullptr;
	uVec2Arr = nullptr;
	uVec3Arr = nullptr;
	uVec4Arr = nullptr;
	bdef = false;

	textureChoiceActivated = false;//on permet l'affichage de la choosebox de texture tout le temps
	textureFileChoosebox = false;
	textureFlipChoosebox = true;
	textureMipmapChoosebox = true;
	textureFilterChoosebox = true;
	textureWrapChoosebox = true;

	soundChoiceActivated = false;
	soundFileChoosebox = false;
	soundLoopChoosebox = true;

	bufferChoiceActivated = false;//on permet l'affichage de la choosebox de texture tout le temps
	bufferFileChoosebox = false;
	bufferFlipChoosebox = true;
	bufferMipmapChoosebox = true;
	bufferFilterChoosebox = true;
	bufferWrapChoosebox = true;

	uSampler2D = -1;
	uImage2D = -1;
	texture_ptr = nullptr;
	ownTexture = false;
	uSampler2DArr = nullptr;
	ownSampler2DArr = false;
	uSampler2DFromThread = false;
	ratioXY = 1.0f;

	lockedAgainstConfigLoading = false;

	uSampler3D = -1;
	uImage3D = -1;
	volume_ptr = nullptr;
	ownVolume = false;
	volumeFormat.clear();
	computeTextureFormat = GL_RGBA32F;

	uSampler1D = -1;
	uImage1D = -1;
	sound_ptr = nullptr;
	sound_histo_ptr = nullptr;
	soundVolume = 0.1f;
	ownSound = false;
	soundLoop = true;
	soundHisto = 0;

	uSamplerCube = -1;
	cubemap_ptr = nullptr;
	ownCubeMap = false;

	ownFloatArr = false;
	ownVec2Arr = false;
	ownVec3Arr = false;
	ownVec4Arr = false;
	bufferShaderName.clear();
	inFileBufferName.clear();
	computeShaderName.clear();
	attachment = 0;
	frame = 0;

	mat2 = glm::mat2(1.0f);
	mat2Def = glm::mat2(1.0f);
	mat3 = glm::mat3(1.0f);
	mat3Def = glm::mat3(1.0f);
	mat4 = glm::mat4(1.0f);
	mat4Def = glm::mat4(1.0f);

	mipmap = true;
	flip = false;
	filter = "linear";
	wrap = "repeat";
	maxmipmaplvl = 100;

	sectionName = "default";
	sectionOrder = 0;
	sectionCond.clear();

	useVisCheckCond = false;
	uniCheckCondName.clear();
	uniCheckCond = false;
	uniCheckCondPtr = nullptr;

	useVisComboCond = false;
	uniComboCondName.clear();
	uniComboCond = 0;
	uniComboCondPtr = nullptr;
	uniComboCondDir = true;

	useVisOpCond = 0;
	uniOpCondThreshold = 0;
	uniCondPtr = nullptr;

	comment = "";
	//commentBufferLen = 0;
	//commentBuffer[0] = '\0';

	SourceLinePos = 0;
	TargetLinePos = 0;

	camRotXY = 0.0f;
	camZoom = 1.0f;
	camTranslateXYZ = 0.0f;
	camDefRotXY = 0.0f;
	camDefZoom = 1.0f;
	camDefTranslateXYZ = 0.0f;
	camNeedRecalc = true;

	timeLineSupported = false;

	//counter++;
}

UniformVariant::~UniformVariant()
{
	ZoneScoped;

	if (ownSampler2DArr)
		SAFE_DELETE_ARRAY(uSampler2DArr);
	if (ownFloatArr)
		SAFE_DELETE_ARRAY(uFloatArr);
	if (ownVec2Arr)
		SAFE_DELETE_ARRAY(uVec2Arr);
	if (ownVec3Arr)
		SAFE_DELETE_ARRAY(uVec3Arr);
	if (ownVec4Arr)
		SAFE_DELETE_ARRAY(uVec4Arr);
	if (ownTexture)
		texture_ptr.reset();
	if (ownCubeMap)
		cubemap_ptr.reset();
	if (ownVolume)
		volume_ptr.reset();
	if (ownSound)
	{
		sound_ptr.reset();
		sound_histo_ptr.reset();
	}

	//counter--;
}

/// <summary>
/// reset uniform to default state
/// </summary>
void UniformVariant::ResetToDefault()
{

}

void UniformVariant::copy(UniformVariantPtr vUniPtr, RenderPackWeak /*vMainRenderPack*/)
{
	ZoneScoped;

	if (vUniPtr != nullptr)
	{
		owner = vUniPtr->owner;
#ifdef DEBUG_UNIFORMS
		ownerString = vUniPtr->ownerString;
#endif
		canWeSave = vUniPtr->canWeSave;
        noExport = vUniPtr->noExport;
		constant = vUniPtr->constant;
		pipe = vUniPtr->pipe;
		record = vUniPtr->record;
		uSampler2DArr = vUniPtr->uSampler2DArr;
		uSamplerCube = vUniPtr->uSamplerCube;
		for (int i = 0; i < 8; i++)
			id[i] = vUniPtr->id[i];
		loc = vUniPtr->loc;
		slot = vUniPtr->slot;
		name = vUniPtr->name;

		text = vUniPtr->text;
		uiOnly = vUniPtr->uiOnly;

		glslType = vUniPtr->glslType;
		widget = vUniPtr->widget;
		widgetType = vUniPtr->widgetType;
		timeLineSupported = vUniPtr->timeLineSupported;
		x = vUniPtr->x;
		y = vUniPtr->y;
		z = vUniPtr->z;
		w = vUniPtr->w;
		bx = vUniPtr->bx;
		by = vUniPtr->by;
		bz = vUniPtr->bz;
		bw = vUniPtr->bw;
		ix = vUniPtr->ix;
		iy = vUniPtr->iy;
		iz = vUniPtr->iz;
		iw = vUniPtr->iw;
		ux = vUniPtr->ux;
		uy = vUniPtr->uy;
		uz = vUniPtr->uz;
		uw = vUniPtr->uw;
		count = vUniPtr->count;
		IsIntegrated = vUniPtr->IsIntegrated;
		uFloatArr = vUniPtr->uFloatArr;
		inf = vUniPtr->inf;
		sup = vUniPtr->sup;
		def = vUniPtr->def;
		step = vUniPtr->step;
		uVec2Arr = vUniPtr->uVec2Arr;
		uVec3Arr = vUniPtr->uVec3Arr;
		uVec4Arr = vUniPtr->uVec4Arr;
		filePathNames = vUniPtr->filePathNames;
		target = vUniPtr->target;
		bdef = vUniPtr->bdef;

		buttonName0 = vUniPtr->buttonName0;
		buttonName1 = vUniPtr->buttonName1;
		buttonName2 = vUniPtr->buttonName2;
		buttonName3 = vUniPtr->buttonName3;

		lockedAgainstConfigLoading = vUniPtr->lockedAgainstConfigLoading;

		textureChoiceActivated = vUniPtr->textureChoiceActivated;
		textureFileChoosebox = vUniPtr->textureFileChoosebox;
		textureFlipChoosebox = vUniPtr->textureFlipChoosebox;
		textureMipmapChoosebox = vUniPtr->textureMipmapChoosebox;
		textureFilterChoosebox = vUniPtr->textureFilterChoosebox;
		textureWrapChoosebox = vUniPtr->textureWrapChoosebox;

		soundChoiceActivated = vUniPtr->soundChoiceActivated;
		soundFileChoosebox = vUniPtr->soundFileChoosebox;
		soundLoopChoosebox = vUniPtr->soundLoopChoosebox;

		bufferChoiceActivated = vUniPtr->bufferChoiceActivated;
		bufferFileChoosebox = vUniPtr->bufferFileChoosebox;
		bufferFlipChoosebox = vUniPtr->bufferFlipChoosebox;
		bufferMipmapChoosebox = vUniPtr->bufferMipmapChoosebox;
		bufferFilterChoosebox = vUniPtr->bufferFilterChoosebox;
		bufferWrapChoosebox = vUniPtr->bufferWrapChoosebox;

		uSampler2D = vUniPtr->uSampler2D;
		uImage2D = vUniPtr->uImage2D;
		texture_ptr = vUniPtr->texture_ptr;
		ownTexture = vUniPtr->ownTexture;
		ratioXY = vUniPtr->ratioXY;
		uSampler2DFromThread = vUniPtr->uSampler2DFromThread;

		uSampler3D = vUniPtr->uSampler3D;
		uImage3D = vUniPtr->uImage3D;
		volume_ptr = vUniPtr->volume_ptr;
		ownVolume = vUniPtr->ownVolume;
		volumeFormat = vUniPtr->volumeFormat;
		computeTextureFormat = vUniPtr->computeTextureFormat;

		uSampler1D = vUniPtr->uSampler1D;
		uImage1D = vUniPtr->uImage1D;
		sound_ptr = vUniPtr->sound_ptr;
		sound_histo_ptr = vUniPtr->sound_histo_ptr;
		ownSound = vUniPtr->ownSound;
		soundVolume = vUniPtr->soundVolume;
		soundLoop = vUniPtr->soundLoop;
		soundHisto = vUniPtr->soundHisto;

		cubemap_ptr = vUniPtr->cubemap_ptr;
		ownCubeMap = vUniPtr->ownCubeMap;

		ownSampler2DArr = vUniPtr->ownSampler2DArr;
		ownFloatArr = vUniPtr->ownFloatArr;
		ownVec2Arr = vUniPtr->ownVec2Arr;
		ownVec3Arr = vUniPtr->ownVec3Arr;
		ownVec4Arr = vUniPtr->ownVec4Arr;
		bufferShaderName = vUniPtr->bufferShaderName;
		inFileBufferName = vUniPtr->inFileBufferName;
		computeShaderName = vUniPtr->computeShaderName;
		attachment = vUniPtr->attachment;
		frame = vUniPtr->frame;

		mat2 = glm::mat2(1.0f);
		mat2Def = glm::mat2(1.0f);
		mat3 = glm::mat3(1.0f);
		mat3Def = glm::mat3(1.0f);
		mat4 = glm::mat4(1.0f);
		mat4Def = glm::mat4(1.0f);

		mipmap = vUniPtr->mipmap;
		flip = vUniPtr->flip;
		filter = vUniPtr->filter;
		wrap = vUniPtr->wrap;
		maxmipmaplvl = vUniPtr->maxmipmaplvl;

		if (vUniPtr->sectionName.empty())
			CTOOL_DEBUG_BREAK;

		sectionName = vUniPtr->sectionName;
		sectionOrder = vUniPtr->sectionOrder;
		sectionCond = vUniPtr->sectionCond;

		useVisCheckCond = vUniPtr->useVisCheckCond;
		uniCheckCondName = vUniPtr->uniCheckCondName;
		uniCheckCond = vUniPtr->uniCheckCond;
		uniCheckCondPtr = vUniPtr->uniCheckCondPtr;

		useVisComboCond = vUniPtr->useVisComboCond;
		uniComboCondName = vUniPtr->uniComboCondName;
		uniComboCond = vUniPtr->uniComboCond;
		uniComboCondPtr = vUniPtr->uniComboCondPtr;
		uniComboCondDir = vUniPtr->uniComboCondDir;

		useVisOpCond = vUniPtr->useVisOpCond;
		uniCondPtr = vUniPtr->uniCondPtr;
		uniOpCondThreshold = vUniPtr->uniOpCondThreshold;

		camRotXY = vUniPtr->camRotXY;
		camZoom = vUniPtr->camZoom;
		camTranslateXYZ = vUniPtr->camTranslateXYZ;
		camDefRotXY = vUniPtr->camDefRotXY;
		camDefZoom = vUniPtr->camDefZoom;
		camDefTranslateXYZ = vUniPtr->camDefTranslateXYZ;
		camNeedRecalc = vUniPtr->camNeedRecalc;

		comment = vUniPtr->comment;
/*
		if (commentBufferLen > 0)
		{
#ifdef MINGW32
			strncpy(
				commentBuffer,
				vUniPtr->commentBuffer, ct::mini<size_t>(commentBufferLen, MAX_UNIFORM_COMMENT_BUFFER_SIZE));
#else
			strncpy_s(
				commentBuffer, MAX_UNIFORM_COMMENT_BUFFER_SIZE,
				vUniPtr->commentBuffer, ct::mini<size_t>(commentBufferLen, MAX_UNIFORM_COMMENT_BUFFER_SIZE));
#endif
		}
*/
	}
}

void UniformVariant::copyValues(UniformVariantPtr vUniPtr, RenderPackWeak  /*vMainRenderPack*/)
{
	ZoneScoped;

	if (vUniPtr)
	{
		owner = vUniPtr->owner;
#ifdef DEBUG_UNIFORMS
		ownerString = vUniPtr->ownerString;
#endif
		constant = vUniPtr->constant;
		
		text = vUniPtr->text;
		uiOnly = vUniPtr->uiOnly;

		x = vUniPtr->x;
		y = vUniPtr->y;
		z = vUniPtr->z;
		w = vUniPtr->w;
		bx = vUniPtr->bx;
		by = vUniPtr->by;
		bz = vUniPtr->bz;
		bw = vUniPtr->bw;
		ix = vUniPtr->ix;
		iy = vUniPtr->iy;
		iz = vUniPtr->iz;
		iw = vUniPtr->iw;
		ux = vUniPtr->ux;
		uy = vUniPtr->uy;
		uz = vUniPtr->uz;
		uw = vUniPtr->uw;
		inf = vUniPtr->inf;
		sup = vUniPtr->sup;
		def = vUniPtr->def;
		step = vUniPtr->step;
		bdef = vUniPtr->bdef;
		if (vUniPtr->uSampler2D > 0)
		{
			uSampler2D = vUniPtr->uSampler2D;
			uImage2D = vUniPtr->uImage2D;
			texture_ptr = vUniPtr->texture_ptr;
			ownTexture = false;
			if (texture_ptr)
				ratioXY = texture_ptr->getRatio();
		}
		uSampler2DFromThread = vUniPtr->uSampler2DFromThread;
		if (vUniPtr->uSampler3D > 0)
		{
			uSampler3D = vUniPtr->uSampler3D;
			uImage3D = vUniPtr->uImage3D;
			volume_ptr = vUniPtr->volume_ptr;
			ownVolume = false;
			volumeFormat = vUniPtr->volumeFormat;
			computeTextureFormat = vUniPtr->computeTextureFormat;
		}
		if (vUniPtr->uSamplerCube > 0)
		{
			uSamplerCube = vUniPtr->uSamplerCube;
			cubemap_ptr = vUniPtr->cubemap_ptr;
			ownCubeMap = false;
		}
		if (vUniPtr->uSampler1D > 0 || 
			vUniPtr->uSampler2D > 0)
		{
			uSampler2D = vUniPtr->uSampler2D;
			uSampler1D = vUniPtr->uSampler1D;
			uImage2D = vUniPtr->uImage2D;
			uImage1D = vUniPtr->uImage1D;
			sound_ptr = vUniPtr->sound_ptr;
			sound_histo_ptr = vUniPtr->sound_histo_ptr;
			ownSound = false;
			soundVolume = vUniPtr->soundVolume;
			soundLoop = vUniPtr->soundLoop;
			soundHisto = vUniPtr->soundHisto;
		}

		pipe = vUniPtr->pipe;
		record = vUniPtr->record;
		bufferShaderName = vUniPtr->bufferShaderName;
		inFileBufferName = vUniPtr->inFileBufferName;
		computeShaderName = vUniPtr->computeShaderName;
		attachment = vUniPtr->attachment;
		frame = vUniPtr->frame;
		target = vUniPtr->target;

		lockedAgainstConfigLoading = vUniPtr->lockedAgainstConfigLoading;

		textureChoiceActivated = vUniPtr->textureChoiceActivated;
		textureFileChoosebox = vUniPtr->textureFileChoosebox;
		textureFlipChoosebox = vUniPtr->textureFlipChoosebox;
		textureMipmapChoosebox = vUniPtr->textureMipmapChoosebox;
		textureFilterChoosebox = vUniPtr->textureFilterChoosebox;
		textureWrapChoosebox = vUniPtr->textureWrapChoosebox;

		soundChoiceActivated = vUniPtr->soundChoiceActivated;
		soundFileChoosebox = vUniPtr->soundFileChoosebox;
		soundLoopChoosebox = vUniPtr->soundLoopChoosebox;

		bufferChoiceActivated = vUniPtr->bufferChoiceActivated;
		bufferFileChoosebox = vUniPtr->bufferFileChoosebox;
		bufferFlipChoosebox = vUniPtr->bufferFlipChoosebox;
		bufferMipmapChoosebox = vUniPtr->bufferMipmapChoosebox;
		bufferFilterChoosebox = vUniPtr->bufferFilterChoosebox;
		bufferWrapChoosebox = vUniPtr->bufferWrapChoosebox;

		mipmap = vUniPtr->mipmap;
		flip = vUniPtr->flip;
		filter = vUniPtr->filter;
		wrap = vUniPtr->wrap;
		maxmipmaplvl = vUniPtr->maxmipmaplvl;

		if (vUniPtr->sectionName.empty())
			CTOOL_DEBUG_BREAK;

		sectionName = vUniPtr->sectionName;
		sectionOrder = vUniPtr->sectionOrder;
        sectionCond = vUniPtr->sectionCond;

        noExport = vUniPtr->noExport;

		useVisCheckCond = vUniPtr->useVisCheckCond;
		uniCheckCondName = vUniPtr->uniCheckCondName;
		uniCheckCond = vUniPtr->uniCheckCond;
		uniCheckCondPtr = vUniPtr->uniCheckCondPtr;

		useVisComboCond = vUniPtr->useVisComboCond;
		uniComboCondName = vUniPtr->uniComboCondName;
		uniComboCond = vUniPtr->uniComboCond;
		uniComboCondPtr = vUniPtr->uniComboCondPtr;
		uniComboCondDir = vUniPtr->uniComboCondDir;

		useVisOpCond = vUniPtr->useVisOpCond;
		uniCondPtr = vUniPtr->uniCondPtr;
		uniOpCondThreshold = vUniPtr->uniOpCondThreshold;

		camRotXY = vUniPtr->camRotXY;
		camZoom = vUniPtr->camZoom;
		camTranslateXYZ = vUniPtr->camTranslateXYZ;
		camDefRotXY = vUniPtr->camDefRotXY;
		camDefZoom = vUniPtr->camDefZoom;
		camDefTranslateXYZ = vUniPtr->camDefTranslateXYZ;
		camNeedRecalc = vUniPtr->camNeedRecalc;

		buttonName0 = vUniPtr->buttonName0;
		buttonName1 = vUniPtr->buttonName1;
		buttonName2 = vUniPtr->buttonName2;
		buttonName3 = vUniPtr->buttonName3;

		mat2 = vUniPtr->mat2;
		mat2Def = vUniPtr->mat2Def;
		mat3 = vUniPtr->mat3;
		mat3Def = vUniPtr->mat3Def;
		mat4 = vUniPtr->mat4;
		mat4Def = vUniPtr->mat4Def;

		widgetType = vUniPtr->widgetType;
		timeLineSupported = vUniPtr->timeLineSupported;

		if (CodeTree::puCurrentGizmo == m_This)
		{
			CodeTree::puCurrentGizmo = vUniPtr;
		}

		comment = vUniPtr->comment;
/*
		if (commentBufferLen > 0)
		{
#ifdef MINGW32
			strncpy(
				commentBuffer,
				vUniPtr->commentBuffer, ct::mini<size_t>(commentBufferLen, MAX_UNIFORM_COMMENT_BUFFER_SIZE));
#else
			strncpy_s(
				commentBuffer, MAX_UNIFORM_COMMENT_BUFFER_SIZE,
				vUniPtr->commentBuffer, ct::mini<size_t>(commentBufferLen, MAX_UNIFORM_COMMENT_BUFFER_SIZE));
#endif
		}
*/
	}
}

void UniformVariant::copyValuesForTimeLine(UniformVariantPtr vUniPtr, RenderPackWeak /*vMainRenderPack*/)
{
	ZoneScoped;

	if (vUniPtr != nullptr)
	{
		x = vUniPtr->x;
		y = vUniPtr->y;
		z = vUniPtr->z;
		w = vUniPtr->w;
		bx = vUniPtr->bx;
		by = vUniPtr->by;
		bz = vUniPtr->bz;
		bw = vUniPtr->bw;
		ix = vUniPtr->ix;
		iy = vUniPtr->iy;
		iz = vUniPtr->iz;
		iw = vUniPtr->iw;
		ux = vUniPtr->ux;
		uy = vUniPtr->uy;
		uz = vUniPtr->uz;
		uw = vUniPtr->uw;

		mat2 = vUniPtr->mat2;
		mat3 = vUniPtr->mat3;
		mat4 = vUniPtr->mat4;
		uFloatArr = glm::value_ptr(mat4[0]);
	}
}