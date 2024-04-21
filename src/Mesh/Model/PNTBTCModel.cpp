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

// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "PNTBTCModel.h"

#include <cassert>
#include <ctools/Logger.h>
#include <ImGuiPack.h>
#include <InAppGpuProfiler/InAppGpuProfiler.h>
#include <Profiler/TracyProfiler.h>
#include <Mesh/Utils/VertexStruct.h>

PNTBTCModelPtr PNTBTCModel::Create(const GuiBackend_Window& vWin)
{
	auto res = std::make_shared<PNTBTCModel>(vWin);
	res->m_This = res;
	return res;
}

PNTBTCModel::PNTBTCModel(const GuiBackend_Window& vWin)
	: BaseModel(vWin)
{
	m_MeshType = BaseMeshEnum::PRIMITIVE_TYPE_MESH;
	m_BaseMeshFormat = BaseMeshFormatEnum::PRIMITIVE_FORMAT_PNTBTC;
	//m_IsLoaded = true;
}

PNTBTCModel::~PNTBTCModel()
{
	Clear();
}

void PNTBTCModel::Clear()
{
	m_Meshs.clear();
	BaseModel::Clear();
}

bool PNTBTCModel::IsValid()
{
	return m_IsLoaded;
}

void PNTBTCModel::DrawModel(const std::string& vName, const GLenum& vRenderMode, const bool& vUseTesselation)
{
	if (IsValid())
	{
		GuiBackend::MakeContextCurrent(m_Window);

		AIGPScoped(vName, "Draw Model");

		TracyGpuZone("PNTBTCModel::DrawModel");

		//uint64_t indices_to_show = m_IndicesCountToShow;
		uint32_t idx = 0U;
		for (auto meshPtr : m_Meshs)
		{
			if (meshPtr)
			{
				//if (indices_to_show > meshPtr->GetIndicesCount())
				{
					//indices_to_show -= meshPtr->GetIndicesCount();
				}

				meshPtr->DrawModel(
					vName,
					idx++,
					m_Window,
					vRenderMode,
					vUseTesselation,
					meshPtr->GetIndicesCount(),
					m_PatchVerticesCount,
					m_InstanceCount);
			}
		}
	}
}

uint32_t PNTBTCModel::GetVaoID(uint32_t vMeshID)
{
	if (m_Meshs.size() > vMeshID)
	{
		if (m_Meshs.at(vMeshID))
		{
			return m_Meshs[vMeshID]->GetVaoID();
		}
	}
	return 0U;
}

uint32_t PNTBTCModel::GetVboID(uint32_t vMeshID)
{
	if (m_Meshs.size() > vMeshID)
	{
		if (m_Meshs.at(vMeshID))
		{
			return m_Meshs[vMeshID]->GetVboID();
		}
	}
	return 0U;
}

uint32_t PNTBTCModel::GetIboID(uint32_t vMeshID)
{
	if (m_Meshs.size() > vMeshID)
	{
		if (m_Meshs.at(vMeshID))
		{
			return m_Meshs[vMeshID]->GetIboID();
		}
	}
	return 0U;
}

uint32_t PNTBTCModel::GetMeshCount()
{
	return (uint32_t)m_Meshs.size();
}

PNTBTCMeshPtr PNTBTCModel::AddMesh(
	const PNTBTCMesh::VerticeArray& vVerticeArray,
	const PNTBTCMesh::IndiceArray& vIndiceArray)
{
	PNTBTCMeshPtr res = PNTBTCMesh::Create(m_Window, vVerticeArray, vIndiceArray);
	m_IndicesCountToShow += res->GetIndicesCount();
	m_Meshs.push_back(res);
	return res;
}

PNTBTCMeshPtr PNTBTCModel::AddMesh()
{
	PNTBTCMeshPtr res = PNTBTCMesh::Create(m_Window);
	m_IndicesCountToShow += res->GetIndicesCount();
	m_Meshs.push_back(res);
	return res;
}

void PNTBTCModel::AddMesh(PNTBTCMeshPtr vMeshPtr)
{
	if (vMeshPtr)
	{
		m_Meshs.push_back(vMeshPtr);
		m_IndicesCountToShow += vMeshPtr->GetIndicesCount();
	}
}

bool PNTBTCModel::ReLoadModel()
{
	m_IsLoaded = false;

	if (!m_Meshs.empty())
	{
		m_IsLoaded = true;

		for (auto meshPtr : m_Meshs)
		{
			if (meshPtr)
			{
				m_IsLoaded &= meshPtr->Init();
			}
		}
	}

	return m_IsLoaded;
}

std::vector<PNTBTCMeshPtr>* PNTBTCModel::GetMeshs()
{
	return &m_Meshs;
}
