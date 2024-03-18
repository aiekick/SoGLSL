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

#include <Mesh/Model/BaseModel.h>

#include <cassert>
#include <ctools/Logger.h>
#include <ImGuiPack.h>
#include <Profiler/TracyProfiler.h>

#include <Mesh/Model/PNCModel.h>
#include <Mesh/Model/PNTCModel.h>
#include <Mesh/Model/QuadModel.h>
#include <Mesh/Model/PointModel.h>
#include <Mesh/Model/PNTBTCModel.h>

/////////////////////////////////////////////////////////////////////
/////// BaseModel ///////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////

BaseModel::BaseModel()
{

}

BaseModel::BaseModel(const GuiBackend_Window& vWin) 
	: m_Window(vWin)
{
	m_MeshType = BaseMeshEnum::PRIMITIVE_TYPE_NONE;
}

BaseModel::~BaseModel()
{
	Clear();
}

void BaseModel::Clear()
{
	m_Layouts.clear();
	m_FilePathName.clear();
}

bool BaseModel::IsValid()
{
	return false;
}

void BaseModel::Reset()
{
	Clear();
}

void BaseModel::SetVerticesCount(uint64_t vVerticesCount)
{
	if (vVerticesCount > 0)
		m_VerticesCount = vVerticesCount;
}

void BaseModel::SetIndicesCount(uint64_t vIndicesCount)
{
	if (vIndicesCount > 0)
		m_IndicesCount = vIndicesCount;
}

void BaseModel::SetIndicesCountToShow(uint64_t vIndicesCountToShow)
{
	if (vIndicesCountToShow >= 0 && vIndicesCountToShow <= m_IndicesCount)
		m_IndicesCountToShow = vIndicesCountToShow;
}

void BaseModel::SetInstancesCount(uint64_t vInstanceCount)
{
	m_InstanceCount = vInstanceCount;
}

void BaseModel::SetPatchVerticesCount(uint64_t vPatchVerticesCount)
{
	m_PatchVerticesCount = vPatchVerticesCount;
}

void BaseModel::SetVertexRangeToShow(uint64_t vFirst, uint64_t vLast)
{
	if (vLast >= vFirst)
	{
		m_FirstVerticeToShow = vFirst;
		m_LastVerticeToShow = vLast;
		m_VerticesCountToShow = vLast - vFirst;
		m_VerticesCountToShow = ct::maxi(m_VerticesCount, (uint64_t)1U);
	}
}

void BaseModel::SetFilePathName(const std::string& vFilePathName)
{
	m_FilePathName = vFilePathName;
}

const std::string& BaseModel::GetFilePathName() const
{
	return m_FilePathName;
}

void BaseModel::SetLayouts(const std::vector<std::string>& vLayouts)
{
	m_Layouts = vLayouts;
}

const std::vector<std::string>& BaseModel::GetLayouts() const
{
	return m_Layouts;
}

uint32_t BaseModel::GetVaoID(uint32_t /*vMeshID*/)
{
	return 0U;
}

uint32_t BaseModel::GetVboID(uint32_t /*vMeshID*/)
{
	return 0U;
}

uint32_t BaseModel::GetIboID(uint32_t /*vMeshID*/)
{
	return 0U;
}

BaseMeshEnum BaseModel::GetMeshType()
{
	return m_MeshType;
}

BaseMeshFormatEnum BaseModel::GetMeshFormat()
{
	return m_BaseMeshFormat;
}

uint64_t BaseModel::GetInstancesCount()
{
	return m_InstanceCount;
}

uint64_t BaseModel::GetPatchVerticesCount()
{
	return m_PatchVerticesCount;
}

uint64_t BaseModel::GetVerticesCount()
{
	return m_VerticesCount;
}

uint64_t BaseModel::GetIndicesCount()
{
	return m_IndicesCount;
}

uint64_t BaseModel::GetIndicesCountToShow()
{
	return m_IndicesCountToShow;
}

ct::fAABBCC BaseModel::GetBoundingBox()
{
	return m_BoundingBox;
}

ct::fvec3 BaseModel::GetModelOffset()
{
	return m_ModelOffset;
}

bool BaseModel::ReLoadModel()
{
	return false;
}

void BaseModel::DrawModel(const std::string& /*vName*/, const GLenum& /*vRenderMode*/, const bool& /*vUseTesselation*/) {
	assert(nullptr); // why are we here ? not normal
	return;
}