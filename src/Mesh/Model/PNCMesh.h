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

#include <memory>
#include <Mesh/Model/BaseModel.h>
#include <Mesh/Utils/VertexStruct.h>

class PNCMesh;
typedef std::shared_ptr<PNCMesh> PNCMeshPtr;
typedef std::weak_ptr<PNCMesh> PNCMeshWeak;

class PNCMesh
{
public:
	typedef std::vector<VertexStruct::P3_N3_C4> VerticeArray;
	typedef std::vector<VertexStruct::I1> IndiceArray;

public:
	GuiBackend_Window m_Window;
	BaseMeshDatas<VertexStruct::P3_N3_C4> m_MeshDatas;
	PNCMeshWeak m_This;
	bool m_HaveNormals = false;
	bool m_HaveVertexColors = false;
	bool m_HaveIndices = false;
	uint64_t m_VerticesCount = 0;
	uint64_t m_IndicesCount = 0;
	bool m_IsLoaded = false;
	bool m_CanWeRender = true;

public:
	static PNCMeshPtr Create();
	static PNCMeshPtr Create(const GuiBackend_Window& vWin);
	static PNCMeshPtr Create(
		const GuiBackend_Window& vWin, 
		const VerticeArray& vVerticeArray, 
		const IndiceArray& vIndiceArray);

public:
	PNCMesh();
	PNCMesh(const GuiBackend_Window& vWin);
	PNCMesh(
		const GuiBackend_Window& vWin, 
		const VerticeArray& vVerticeArray, 
		const IndiceArray& vIndiceArray);
	~PNCMesh();

	bool Init();
	void Unit();

	bool empty();
	bool HasNormals();
	bool HasVertexColors();
	bool HasIndices();

	VerticeArray& GetVertices();
	uint64_t GetVerticesCount();

	IndiceArray& GetIndices();
	uint64_t GetIndicesCount();

	void HaveNormals();
	void HaveVertexColors();
	void HaveIndices();

	void SetCanWeRender(bool vFlag);

	void DrawModel(
		const std::string& vName,
		const uint32_t& vIdx,
		const GuiBackend_Window& vWin,
		const GLenum& vRenderMode,		
		const bool& vUseTesselation,
		const uint64_t& vIndicesCountToShow,
		const uint64_t& vPatchVerticesCount,
		const uint64_t& vInstanceCount);

	uint32_t GetVaoID();
	uint32_t GetVboID();
	uint32_t GetIboID();

	bool UpdateMesh(const VerticeArray& vVertexs, const IndiceArray& vIndices);

private:
	bool PreparePNC(bool vUpdate);
};
