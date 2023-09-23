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

class PNTBTCMesh;
typedef std::shared_ptr<PNTBTCMesh> PNTBTCMeshPtr;
typedef std::weak_ptr<PNTBTCMesh> PNTBTCMeshWeak;

class PNTBTCMesh
{
public:
	typedef std::vector<VertexStruct::P3_N3_TA3_BTA3_T2_C4> VerticeArray;
	typedef std::vector<VertexStruct::I1> IndiceArray;

public:
	GuiBackend_Window m_Window;
	BaseMeshDatas<VertexStruct::P3_N3_TA3_BTA3_T2_C4> m_MeshDatas;
	PNTBTCMeshWeak m_This;
	bool m_HaveNormals = false;
	bool m_HaveTangeants = false;
	bool m_HaveBiTangeants = false;
	bool m_HaveTextureCoords = false;
	bool m_HaveVertexColors = false;
	bool m_HaveIndices = false;
	uint64_t m_VerticesCount = 0;
	uint64_t m_IndicesCount = 0;
	bool m_IsLoaded = false;
	bool m_CanWeRender = true;

public:
	static PNTBTCMeshPtr Create();
	static PNTBTCMeshPtr Create(const GuiBackend_Window& vWin);
	static PNTBTCMeshPtr Create(
		const GuiBackend_Window& vWin, 
		const VerticeArray& vVerticeArray, 
		const IndiceArray& vIndiceArray);

public:
	PNTBTCMesh();
	PNTBTCMesh(const GuiBackend_Window& vWin);
	PNTBTCMesh(
		const GuiBackend_Window& vWin, 
		const VerticeArray& vVerticeArray, 
		const IndiceArray& vIndiceArray);
	~PNTBTCMesh();

	bool Init();
	void Unit();

	bool empty();
	bool HasNormals();
	bool HasTangeants();
	bool HasBiTangeants();
	bool HasTextureCoords();
	bool HasVertexColors();
	bool HasIndices();

	VerticeArray* GetVertices();
	uint64_t GetVerticesCount();

	IndiceArray* GetIndices();
	uint64_t GetIndicesCount();

	void HaveNormals();
	void HaveTangeants();
	void HaveBiTangeants();
	void HaveTextureCoords();
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

private:
	bool PreparePNTBTC(bool vUpdate);
};
