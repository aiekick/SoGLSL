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

#include "PNTBTCModel.h"

#include <cassert>
#include <ctools/Logger.h>
#include <ImGuiPack.h>
#include <Profiler/TracyProfiler.h>
#include <Mesh/Utils/VertexStruct.h>

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// STATIC ////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

PNTBTCMeshPtr PNTBTCMesh::Create()
{
	PNTBTCMeshPtr res = std::make_shared<PNTBTCMesh>();
	res->m_This = res;
	return res;
}

PNTBTCMeshPtr PNTBTCMesh::Create(const GuiBackend_Window& vWin)
{
	PNTBTCMeshPtr res = std::make_shared<PNTBTCMesh>(vWin);
	res->m_This = res;
	return res;
}

PNTBTCMeshPtr PNTBTCMesh::Create(
	const GuiBackend_Window& vWin, 
	const VerticeArray& vVerticeArray, 
	const IndiceArray& vIndiceArray)
{
	PNTBTCMeshPtr res = std::make_shared<PNTBTCMesh>(vWin, vVerticeArray, vIndiceArray);
	res->m_This = res;
	if (!res->Init())
		res.reset();
	return res;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// CTOR / DTOR ///////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

PNTBTCMesh::PNTBTCMesh()
{

}

PNTBTCMesh::PNTBTCMesh(const GuiBackend_Window& vWin)
	: m_Window(vWin)
{

}

PNTBTCMesh::PNTBTCMesh(
	const GuiBackend_Window& vWin, 
	const VerticeArray& vVerticeArray, 
	const IndiceArray& vIndiceArray)
	: m_Window(vWin)
{
	m_MeshDatas.m_Vertices = vVerticeArray;
	m_MeshDatas.m_Indices = vIndiceArray;
}

PNTBTCMesh::~PNTBTCMesh()
{
	Unit();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// INIT / UNIT ///////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool PNTBTCMesh::Init()
{
	m_IsLoaded = PreparePNTBTC(m_IsLoaded);
	return m_IsLoaded;
}

void PNTBTCMesh::Unit()
{
	m_MeshDatas.Clear(m_Window);
	m_IsLoaded = false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// TESTS /////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool PNTBTCMesh::empty()
{
	return 
		m_MeshDatas.m_Vertices.empty() || 
		m_MeshDatas.m_Indices.empty();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// GETTERS ///////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

PNTBTCMesh::VerticeArray* PNTBTCMesh::GetVertices()
{
	return &m_MeshDatas.m_Vertices;
}

uint64_t PNTBTCMesh::GetVerticesCount()
{
	return (uint64_t)m_MeshDatas.m_Vertices.size();
}

PNTBTCMesh::IndiceArray* PNTBTCMesh::GetIndices()
{
	return &m_MeshDatas.m_Indices;
}

uint64_t PNTBTCMesh::GetIndicesCount()
{
	return (uint64_t)m_MeshDatas.m_Indices.size();
}

bool PNTBTCMesh::HasNormals()
{
	return m_HaveNormals;
}

void PNTBTCMesh::HaveNormals()
{
	m_HaveNormals = true;
}

bool PNTBTCMesh::HasTangeants()
{
	return m_HaveTangeants;
}

void PNTBTCMesh::HaveTangeants()
{
	m_HaveTangeants = true;
}

bool PNTBTCMesh::HasBiTangeants()
{
	return m_HaveBiTangeants;
}

void PNTBTCMesh::HaveBiTangeants()
{
	m_HaveBiTangeants = true;
}

bool PNTBTCMesh::HasTextureCoords()
{
	return m_HaveTextureCoords;
}

void PNTBTCMesh::HaveTextureCoords()
{
	m_HaveTextureCoords = true;
}

bool PNTBTCMesh::HasVertexColors()
{
	return m_HaveVertexColors;
}

void PNTBTCMesh::HaveVertexColors()
{
	m_HaveVertexColors = true;
}

bool PNTBTCMesh::HasIndices()
{
	return m_HaveIndices;
}

void PNTBTCMesh::HaveIndices()
{
	m_HaveIndices = true;
}

void PNTBTCMesh::SetCanWeRender(bool vFlag)
{
	m_CanWeRender = vFlag;
}

void PNTBTCMesh::DrawModel(
	const std::string& vName,
	const uint32_t& vIdx,
	const GuiBackend_Window& vWin,
	const GLenum& vRenderMode, 
	const bool& vUseTesselation,
	const uint64_t& vIndicesCountToShow,
	const uint64_t& vPatchVerticesCount,
	const uint64_t& vInstanceCount)
{
	if (m_IsLoaded && m_CanWeRender)
	{
		GuiBackend::MakeContextCurrent(vWin);

		AIGPScoped(vName, "Sub Mesh %u", vIdx);

		TracyGpuZone("PNTBTCModel::DrawModel");

		if (glIsVertexArray(m_MeshDatas.m_Vao) == GL_TRUE)
		{
			if (vIndicesCountToShow > 0 && vIndicesCountToShow <= m_IndicesCount)
			{
				// bind
				glBindVertexArray(m_MeshDatas.m_Vao); // select first VAO
				LogGlError();
				glEnableVertexAttribArray(0); // pos
				LogGlError();
				glEnableVertexAttribArray(1); // nor
				LogGlError();
				glEnableVertexAttribArray(2); // tan
				LogGlError();
				glEnableVertexAttribArray(3); // btan
				LogGlError();
				glEnableVertexAttribArray(4); // tex
				LogGlError();
				glEnableVertexAttribArray(5); // col
				LogGlError();

				if (vUseTesselation && vPatchVerticesCount && glPatchParameteri)
					glPatchParameteri(GL_PATCH_VERTICES, (GLint)vPatchVerticesCount);

				if (vInstanceCount > 1U)
				{
					glDrawElementsInstanced(vRenderMode, (GLsizei)vIndicesCountToShow, GL_UNSIGNED_INT, nullptr, (GLsizei)vInstanceCount);	// draw first object 6 => decalage 3 coord * 2 (float)
					LogGlError();
				}
				else
				{
					glDrawElements(vRenderMode, (GLsizei)vIndicesCountToShow, GL_UNSIGNED_INT, nullptr);
					LogGlError();
				}

				// unbind
				glDisableVertexAttribArray(5); // col
				LogGlError();
				glDisableVertexAttribArray(4); // tex
				LogGlError();
				glDisableVertexAttribArray(6); // btan
				LogGlError();
				glDisableVertexAttribArray(2); // tan
				LogGlError();
				glDisableVertexAttribArray(1); // nor
				LogGlError();
				glDisableVertexAttribArray(0); // pos
				LogGlError();
				glBindVertexArray(0);
				LogGlError();
			}
		}
	}
}

uint32_t PNTBTCMesh::GetVaoID()
{
	return m_MeshDatas.GetVaoID();
}

uint32_t PNTBTCMesh::GetVboID()
{
	return m_MeshDatas.GetVboID();
}

uint32_t PNTBTCMesh::GetIboID()
{
	return m_MeshDatas.GetIboID();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// PRIVATE ///////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool PNTBTCMesh::PreparePNTBTC(bool vUpdate)
{
	//GuiBackend::MakeContextCurrent(m_Window);

	TracyGpuZone("PNTBTCModel::PreparePNTBTC");

	if (!m_MeshDatas.m_Vertices.empty())
	{
		if (vUpdate)
		{
			m_VerticesCount = m_MeshDatas.m_Vertices.size();

			glBindVertexArray(m_MeshDatas.m_Vao);
			LogGlError();

			glBindBuffer(GL_ARRAY_BUFFER, m_MeshDatas.m_Vbo);
			LogGlError();

			glBufferData(GL_ARRAY_BUFFER,
				m_MeshDatas.verticeSize * m_MeshDatas.m_Vertices.size(),
				m_MeshDatas.m_Vertices.data(), GL_DYNAMIC_DRAW);
			LogGlError();

			if (!m_MeshDatas.m_Indices.empty())
			{
				m_IndicesCount = m_MeshDatas.m_Indices.size();

				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_MeshDatas.m_Ibo);
				LogGlError();

				glBufferData(GL_ELEMENT_ARRAY_BUFFER,
					m_MeshDatas.indiceSize * m_MeshDatas.m_Indices.size(),
					m_MeshDatas.m_Indices.data(), GL_DYNAMIC_DRAW);
				LogGlError();
			}

			glBindVertexArray(0);
			LogGlError();
			glBindBuffer(GL_ARRAY_BUFFER, 0); // bien unbind les buffer apres le vao sinon le contexte est v�rol�
			LogGlError();

			if (!m_MeshDatas.m_Indices.empty())
			{
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0); // bien unbind les buffer apres le vao sinon le contexte est v�rol�
				LogGlError();
			}
		}
		else
		{
			m_VerticesCount = m_MeshDatas.m_Vertices.size();

			if (m_MeshDatas.m_Vbo > 0)
			{
				glDeleteBuffers(1, &m_MeshDatas.m_Vbo);
				LogGlError();
			}

			if (m_MeshDatas.m_Ibo > 0)
			{
				glDeleteBuffers(1, &m_MeshDatas.m_Ibo);
				LogGlError();
			}

			if (m_MeshDatas.m_Vao > 0)
			{
				glDeleteVertexArrays(1, &m_MeshDatas.m_Vao);
				LogGlError();
			}


			// gen
			glGenVertexArrays(1, &m_MeshDatas.m_Vao);
			LogGlError();
			glGenBuffers(1, &m_MeshDatas.m_Vbo);
			LogGlError();
			glGenBuffers(1, &m_MeshDatas.m_Ibo);
			LogGlError();

			glBindVertexArray(m_MeshDatas.m_Vao);
			LogGlError();
			glBindBuffer(GL_ARRAY_BUFFER, m_MeshDatas.m_Vbo);
			LogGlError();

			glBufferData(GL_ARRAY_BUFFER,
				m_MeshDatas.verticeSize * m_MeshDatas.m_Vertices.size(),
				m_MeshDatas.m_Vertices.data(), GL_DYNAMIC_DRAW);
			LogGlError();

			// pos
			glEnableVertexAttribArray(0);
			LogGlError();
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, m_MeshDatas.verticeSize, (void*)nullptr);
			LogGlError();
			glDisableVertexAttribArray(0);
			LogGlError();

			// nor
			glEnableVertexAttribArray(1);
			LogGlError();
			glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, m_MeshDatas.verticeSize, (void*)(sizeof(float) * 3));
			LogGlError();
			glDisableVertexAttribArray(1);
			LogGlError();

			// tangent
			glEnableVertexAttribArray(2);
			LogGlError();
			glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, m_MeshDatas.verticeSize, (void*)(sizeof(float) * 6));
			LogGlError();
			glDisableVertexAttribArray(2);
			LogGlError();

			// bi tangent
			glEnableVertexAttribArray(3);
			LogGlError();
			glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, m_MeshDatas.verticeSize, (void*)(sizeof(float) * 9));
			LogGlError();
			glDisableVertexAttribArray(3);
			LogGlError();

			// tex
			glEnableVertexAttribArray(4);
			LogGlError();
			glVertexAttribPointer(4, 2, GL_FLOAT, GL_FALSE, m_MeshDatas.verticeSize, (void*)(sizeof(float) * 12));
			LogGlError();
			glDisableVertexAttribArray(4);
			LogGlError();

			// col
			glEnableVertexAttribArray(5);
			LogGlError();
			glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, m_MeshDatas.verticeSize, (void*)(sizeof(float) * 14));
			LogGlError();
			glDisableVertexAttribArray(5);
			LogGlError();

			if (!m_MeshDatas.m_Indices.empty())
			{
				m_IndicesCount = m_MeshDatas.m_Indices.size();

				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_MeshDatas.m_Ibo);
				LogGlError();

				glBufferData(GL_ELEMENT_ARRAY_BUFFER,
					m_MeshDatas.indiceSize * m_MeshDatas.m_Indices.size(),
					m_MeshDatas.m_Indices.data(), GL_DYNAMIC_DRAW);
				LogGlError();
			}

			// unbind
			glBindVertexArray(0);
			LogGlError();
			glBindBuffer(GL_ARRAY_BUFFER, 0); // bien unbind les buffer apres le vao sinon le contexte est v�rol�
			LogGlError();
			if (!m_MeshDatas.m_Indices.empty())
			{
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
				LogGlError();
			}
		}
	}

	glFinish();

	return true;
}
