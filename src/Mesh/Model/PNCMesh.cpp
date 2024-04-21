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

#include "PNCModel.h"

#include <cassert>
#include <ctools/Logger.h>
#include <ImGuiPack.h>
#include <InAppGpuProfiler/InAppGpuProfiler.h>
#include <Profiler/TracyProfiler.h>
#include <Mesh/Utils/VertexStruct.h>

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// STATIC ////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

PNCMeshPtr PNCMesh::Create()
{
	PNCMeshPtr res = std::make_shared<PNCMesh>();
	res->m_This = res;
	return res;
}

PNCMeshPtr PNCMesh::Create(const GuiBackend_Window& vWin)
{
	PNCMeshPtr res = std::make_shared<PNCMesh>(vWin);
	res->m_This = res;
	return res;
}

PNCMeshPtr PNCMesh::Create(
	const GuiBackend_Window& vWin, 
	const VerticeArray& vVerticeArray, 
	const IndiceArray& vIndiceArray)
{
	PNCMeshPtr res = std::make_shared<PNCMesh>(vWin, vVerticeArray, vIndiceArray);
	res->m_This = res;
	if (!res->Init())
		res.reset();
	return res;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// CTOR / DTOR ///////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

PNCMesh::PNCMesh()
{

}

PNCMesh::PNCMesh(const GuiBackend_Window& vWin)
	: m_Window(vWin)
{

}

PNCMesh::PNCMesh(
	const GuiBackend_Window& vWin, 
	const VerticeArray& vVerticeArray, 
	const IndiceArray& vIndiceArray)
	: m_Window(vWin)
{
	m_MeshDatas.m_Vertices = vVerticeArray;
	m_MeshDatas.m_Indices = vIndiceArray;
}

PNCMesh::~PNCMesh()
{
	Unit();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// INIT / UNIT ///////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool PNCMesh::Init()
{
	m_IsLoaded = PreparePNC(m_IsLoaded);
	return m_IsLoaded;
}

void PNCMesh::Unit()
{
	m_MeshDatas.Clear(m_Window);
	m_IsLoaded = false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// TESTS /////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool PNCMesh::empty()
{
	return 
		m_MeshDatas.m_Vertices.empty() || 
		m_MeshDatas.m_Indices.empty();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// GETTERS ///////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

PNCMesh::VerticeArray& PNCMesh::GetVertices()
{
	return m_MeshDatas.m_Vertices;
}

uint64_t PNCMesh::GetVerticesCount()
{
	return (uint64_t)m_MeshDatas.m_Vertices.size();
}

PNCMesh::IndiceArray& PNCMesh::GetIndices()
{
	return m_MeshDatas.m_Indices;
}

uint64_t PNCMesh::GetIndicesCount()
{
	return (uint64_t)m_MeshDatas.m_Indices.size();
}

bool PNCMesh::HasNormals()
{
	return m_HaveNormals;
}

void PNCMesh::HaveNormals()
{
	m_HaveNormals = true;
}

bool PNCMesh::HasVertexColors()
{
	return m_HaveVertexColors;
}

void PNCMesh::HaveVertexColors()
{
	m_HaveVertexColors = true;
}

bool PNCMesh::HasIndices()
{
	return m_HaveIndices;
}

void PNCMesh::HaveIndices()
{
	m_HaveIndices = true;
}

void PNCMesh::SetCanWeRender(bool vFlag)
{
	m_CanWeRender = vFlag;
}

void PNCMesh::DrawModel(
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

		TracyGpuZone("PNCModel::DrawModel");

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
				glEnableVertexAttribArray(2); // col
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
				glDisableVertexAttribArray(2); // col
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

uint32_t PNCMesh::GetVaoID()
{
	return m_MeshDatas.GetVaoID();
}

uint32_t PNCMesh::GetVboID()
{
	return m_MeshDatas.GetVboID();
}

uint32_t PNCMesh::GetIboID()
{
	return m_MeshDatas.GetIboID();
}

bool PNCMesh::UpdateMesh(const VerticeArray& vVertexs, const IndiceArray& vIndices)
{
	m_MeshDatas.m_Vertices = vVertexs;
	m_MeshDatas.m_Indices = vIndices;
	return PreparePNC(true);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// PRIVATE ///////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool PNCMesh::PreparePNC(bool vUpdate)
{
	//GuiBackend::MakeContextCurrent(m_Window);

	TracyGpuZone("PNCModel::PreparePNC");

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

			// col
			glEnableVertexAttribArray(2);
			LogGlError();
			glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, m_MeshDatas.verticeSize, (void*)(sizeof(float) * 6));
			LogGlError();
			glDisableVertexAttribArray(2);
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
