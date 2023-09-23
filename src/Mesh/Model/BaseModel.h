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

#include <string>
#include <cstdint>
#include <unordered_map>
#include <ctools/cTools.h>
#include <Gui/GuiBackend.h>
#include <Mesh/Utils/VertexStruct.h>
#include <Headers/RenderPackHeaders.h>
#include <CodeTree/Parsing/ShaderStageParsing.h>

template<typename T>
class BaseMeshDatas
{
public:
	uint32_t m_Vbo = 0;
	uint32_t m_Ibo = 0;
	uint32_t m_Vao = 0;

	std::vector<T> m_Vertices;
	std::vector<VertexStruct::I1> m_Indices;

	const GLsizei verticeSize = sizeof(T);
	const GLsizei indiceSize = sizeof(VertexStruct::I1);

public:
	uint32_t GetVaoID() { return m_Vbo;	}
	uint32_t GetVboID() { return m_Ibo; }
	uint32_t GetIboID() { return m_Vao; }

	void Clear(const GuiBackend_Window& vWin)
	{
		GuiBackend::MakeContextCurrent(vWin);

		SAFE_DELETE_GL_BUFFER(m_Vbo);
		//LogGlError();

		// attention a bien utiliser la bonne instruction
		// avant j'utilisait glDeleteVertexArrays(1, &puibo)
		// or un IBO ca ce detruit avec glDeleteBuffers
		// et ca detruisait un vao qui avait le meme id que puibo
		// et par malheur c'etait le vao du template vide, du coup ca faisait crasher le driver
		// de rendu du quad 2d.. c'est con, mais pour trouver le bug, bonjour, c'etait long et galere.
		SAFE_DELETE_GL_BUFFER(m_Ibo);
		//LogGlError();

		SAFE_DELETE_GL_VERTEX_ARRAY(m_Vao);
		//LogGlError();
	}
};

class BaseModel
{
protected:
	GuiBackend_Window m_Window;

	BaseMeshEnum m_MeshType = BaseMeshEnum::PRIMITIVE_TYPE_NONE;
	BaseMeshFormatEnum m_BaseMeshFormat = BaseMeshFormatEnum::PRIMITIVE_FORMAT_PNTBTC;

	uint64_t m_VerticesCount = 0;
	uint64_t m_IndicesCount = 0;
	uint64_t m_IndicesCountToShow = 0;

	uint64_t m_VerticesCountToShow = 0;
	uint64_t m_FirstVerticeToShow = 0;
	uint64_t m_LastVerticeToShow = 0;

	ct::fAABBCC m_BoundingBox;
	ct::fvec3 m_ModelOffset;

	uint64_t m_MaxPoints = 0;

	uint64_t m_InstanceCount = 1U;
	uint64_t m_PatchVerticesCount = 3U;

	bool m_IsLoaded = false;

	std::string m_FilePathName;

	std::vector<std::string> m_Layouts;

public:
	BaseModel();
	BaseModel(const GuiBackend_Window& vWin);
	virtual ~BaseModel();

	virtual void Clear();
	virtual bool IsValid();
	virtual void Reset();

	virtual uint32_t GetVaoID(uint32_t vMeshID = 0U);
	virtual uint32_t GetVboID(uint32_t vMeshID = 0U);
	virtual uint32_t GetIboID(uint32_t vMeshID = 0U);

	BaseMeshEnum GetMeshType();
	BaseMeshFormatEnum GetMeshFormat();

	void SetFilePathName(const std::string& vFilePathName);
	const std::string& GetFilePathName() const;

	void SetLayouts(const std::vector<std::string>& vLayouts);
	const std::vector<std::string>& GetLayouts() const;

	uint64_t GetInstancesCount();
	uint64_t GetPatchVerticesCount();
	uint64_t GetVerticesCount();
	uint64_t GetIndicesCount();
	uint64_t GetIndicesCountToShow();
	ct::fAABBCC GetBoundingBox();
	ct::fvec3 GetModelOffset();

	virtual void SetVerticesCount(uint64_t vVerticesCount);
	void SetIndicesCount(uint64_t vCountIdx);
	void SetIndicesCountToShow(uint64_t vCountIdxToShow);
	void SetInstancesCount(uint64_t vInstanceCount);
	void SetPatchVerticesCount(uint64_t vPatchVerticesCount);
	void SetVertexRangeToShow(uint64_t vFirst, uint64_t vLast);

	virtual bool ReLoadModel();
	virtual void DrawModel(const std::string& vName, const GLenum& vRenderMode = GL_TRIANGLES, const bool& vUseTesselation = false);
};