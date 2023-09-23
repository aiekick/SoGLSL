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

#include <Mesh/Model/BaseModel.h>
#include <Mesh/Utils/VertexStruct.h>
#include <Mesh/Model/PNCMesh.h>

class PNCModel;
typedef std::shared_ptr<PNCModel> PNCModelPtr;
typedef std::weak_ptr<PNCModel> PNCModelWeak;

class PNCModel : public BaseModel
{
private:
	std::vector<PNCMeshPtr> m_Meshs;

public:
	static PNCModelPtr Create(const GuiBackend_Window& vWin);

protected:
	PNCModelWeak m_This;

public:
	PNCModel(const GuiBackend_Window& vWin);
	~PNCModel() override;

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
	PNCMeshPtr AddMesh(
		const PNCMesh::VerticeArray& vVerticeArray,
		const PNCMesh::IndiceArray& vIndiceArray);

	/// <summary>
	/// add a empy mesh and retrun it
	/// </summary>
	/// <returns></returns>
	PNCMeshPtr AddMesh();

	/// <summary>
	/// add a already build mesh
	/// </summary>
	/// <param name="vMesh"></param>
	void AddMesh(PNCMeshPtr vMeshPtr);

	bool ReLoadModel();

	std::vector<PNCMeshPtr>& GetMeshs();
};