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

class PNTCModel;
typedef std::shared_ptr<PNTCModel> PNTCModelPtr;
typedef std::weak_ptr<PNTCModel> PNTCModelWeak;

class PNTCModel : public BaseModel
{
private:
	BaseMeshDatas<VertexStruct::P3_N3_T2_C4> m_MeshDatas;

public:
	static PNTCModelPtr Create(const GuiBackend_Window& vWin);

protected:
	PNTCModelWeak m_This;

public:
	PNTCModel(const GuiBackend_Window& vWin);
	~PNTCModel() override;

	void Clear() override;
	bool IsValid() override;

	void DrawModel(const std::string& vName, const GLenum& vRenderMode = GL_TRIANGLES, const bool& vUseTesselation = false) override;

	uint32_t GetVaoID(uint32_t vMeshID = 0U) override;
	uint32_t GetVboID(uint32_t vMeshID = 0U) override;
	uint32_t GetIboID(uint32_t vMeshID = 0U) override;

private:
	bool PreparePNTC(bool vUpdate);
};
