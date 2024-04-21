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

#include <Mesh/Model/BaseModel.h>
#include <Mesh/Utils/VertexStruct.h>
#include <Mesh/Model/PNTBTCMesh.h>

class PNTBTCModel;
typedef std::shared_ptr<PNTBTCModel> PNTBTCModelPtr;
typedef std::weak_ptr<PNTBTCModel> PNTBTCModelWeak;

class PNTBTCModel : public BaseModel
{
private:
	std::vector<PNTBTCMeshPtr> m_Meshs;

public:
	static PNTBTCModelPtr Create(const GuiBackend_Window& vWin);

protected:
	PNTBTCModelWeak m_This;

public:
	PNTBTCModel(const GuiBackend_Window& vWin);
	~PNTBTCModel() override;

	void Clear() override;
	bool IsValid() override;

	void DrawModel(const std::string& vName, const GLenum& vRenderMode = GL_TRIANGLES, const bool& vUseTesselation = false) override;

	uint32_t GetVaoID(uint32_t vMeshID = 0U) override;
	uint32_t GetVboID(uint32_t vMeshID = 0U) override;
	uint32_t GetIboID(uint32_t vMeshID = 0U) override;

	uint32_t GetMeshCount();

	/// <summary>
	/// add, fill and build a mesh and return it
	/// </summary>
	/// <param name="vVerticeArray"></param>
	/// <param name="vIndiceArray"></param>
	/// <returns></returns>
	PNTBTCMeshPtr AddMesh(
		const PNTBTCMesh::VerticeArray& vVerticeArray, 
		const PNTBTCMesh::IndiceArray& vIndiceArray);

	/// <summary>
	/// add a empy mesh and retrun it
	/// </summary>
	/// <returns></returns>
	PNTBTCMeshPtr AddMesh();

	/// <summary>
	/// add a already build mesh
	/// </summary>
	/// <param name="vMesh"></param>
	void AddMesh(PNTBTCMeshPtr vMeshPtr);

	bool ReLoadModel();

	std::vector<PNTBTCMeshPtr>* GetMeshs();
};