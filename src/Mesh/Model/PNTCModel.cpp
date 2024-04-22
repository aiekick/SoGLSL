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

#include "PNTCModel.h"

#include <cassert>
#include <ctools/Logger.h>
#include <ImGuiPack.h>
#include <InAppGpuProfiler/InAppGpuProfiler.h>
#include <Profiler/TracyProfiler.h>
#include <Mesh/Utils/VertexStruct.h>

PNTCModelPtr PNTCModel::Create(const GuiBackend_Window& vWin) {
    auto res = std::make_shared<PNTCModel>(vWin);
    res->m_This = res;
    res->ReLoadModel();
    if (!res->IsValid())
        res.reset();
    return res;
}

PNTCModel::PNTCModel(const GuiBackend_Window& vWin) : BaseModel(vWin) {
    m_MeshType = BaseMeshEnum::PRIMITIVE_TYPE_MESH;
    m_BaseMeshFormat = BaseMeshFormatEnum::PRIMITIVE_FORMAT_PNTC;
    m_MeshDatas.m_Vertices = {VertexStruct::P3_N3_T2_C4()};
    m_MeshDatas.m_Indices = {VertexStruct::I1(0)};
    m_IsLoaded = PreparePNTC(false);
}

PNTCModel::~PNTCModel() {
    Clear();
}

void PNTCModel::Clear() {
    m_MeshDatas.Clear(m_Window);
}

bool PNTCModel::IsValid() {
    return m_IsLoaded;
}

void PNTCModel::DrawModel(const std::string& vName, const GLenum& vRenderMode, const bool& vUseTesselation) {
    GuiBackend::MakeContextCurrent(m_Window);

    AIGPScoped(vName, "Draw Mesh PNTC");

    TracyGpuZone("PNTCModel::DrawModel");

    if (glIsVertexArray(m_MeshDatas.m_Vao) == GL_TRUE) {
        if (m_IndicesCountToShow > 0 && m_IndicesCountToShow <= m_IndicesCount) {
            // bind
            glBindVertexArray(m_MeshDatas.m_Vao);  // select first VAO
            LogGlError();
            glEnableVertexAttribArray(0);  // pos
            LogGlError();
            glEnableVertexAttribArray(1);  // nor
            LogGlError();
            glEnableVertexAttribArray(2);  // tex
            LogGlError();
            glEnableVertexAttribArray(3);  // col
            LogGlError();

            if (vUseTesselation && m_PatchVerticesCount && glPatchParameteri)
                glPatchParameteri(GL_PATCH_VERTICES, (GLint)m_PatchVerticesCount);

            if (m_InstanceCount > 1U) {
                glDrawElementsInstanced(vRenderMode,
                                        (GLsizei)m_IndicesCountToShow,
                                        GL_UNSIGNED_INT,
                                        nullptr,
                                        (GLsizei)m_InstanceCount);  // draw first object 6 => decalage 3 coord * 2 (float)
                LogGlError();
            } else {
                glDrawElements(vRenderMode, (GLsizei)m_IndicesCountToShow, GL_UNSIGNED_INT, nullptr);
                LogGlError();
            }

            // unbind
            glDisableVertexAttribArray(3);  // col
            LogGlError();
            glDisableVertexAttribArray(2);  // tex
            LogGlError();
            glDisableVertexAttribArray(1);  // nor
            LogGlError();
            glDisableVertexAttribArray(0);  // pos
            LogGlError();
            glBindVertexArray(0);
            LogGlError();
        }
    }
}

uint32_t PNTCModel::GetVaoID(uint32_t /*vMeshID*/) {
    return m_MeshDatas.GetVaoID();
}

uint32_t PNTCModel::GetVboID(uint32_t /*vMeshID*/) {
    return m_MeshDatas.GetVboID();
}

uint32_t PNTCModel::GetIboID(uint32_t /*vMeshID*/) {
    return m_MeshDatas.GetIboID();
}

bool PNTCModel::PreparePNTC(bool vUpdate) {
    GuiBackend::MakeContextCurrent(m_Window);

    TracyGpuZone("ModelRendering::PrepareSceneForModelPNTC");

    if (!m_MeshDatas.m_Vertices.empty()) {
        if (vUpdate) {
            SetVerticesCount(m_MeshDatas.m_Vertices.size());
            SetIndicesCount(0);
            SetIndicesCountToShow(0);

            glBindVertexArray(m_MeshDatas.m_Vao);
            LogGlError();

            glBindBuffer(GL_ARRAY_BUFFER, m_MeshDatas.m_Vbo);
            LogGlError();

            glBufferData(GL_ARRAY_BUFFER, m_MeshDatas.verticeSize * m_MeshDatas.m_Vertices.size(), m_MeshDatas.m_Vertices.data(), GL_DYNAMIC_DRAW);
            LogGlError();

            if (!m_MeshDatas.m_Indices.empty()) {
                SetIndicesCount(m_MeshDatas.m_Indices.size());
                SetIndicesCountToShow(m_MeshDatas.m_Indices.size());

                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_MeshDatas.m_Ibo);
                LogGlError();

                glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_MeshDatas.indiceSize * m_MeshDatas.m_Indices.size(), m_MeshDatas.m_Indices.data(), GL_DYNAMIC_DRAW);
                LogGlError();
            }

            glBindVertexArray(0);
            LogGlError();
            glBindBuffer(GL_ARRAY_BUFFER, 0);  // bien unbind les buffer apres le vao sinon le contexte est v�rol�
            LogGlError();

            if (!m_MeshDatas.m_Indices.empty()) {
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);  // bien unbind les buffer apres le vao sinon le contexte est v�rol�
                LogGlError();
            }
        } else {
            SetVerticesCount(m_MeshDatas.m_Vertices.size());
            SetIndicesCount(0);
            SetIndicesCountToShow(0);

            if (m_MeshDatas.m_Vbo > 0) {
                glDeleteBuffers(1, &m_MeshDatas.m_Vbo);
                LogGlError();
            }

            if (m_MeshDatas.m_Ibo > 0) {
                glDeleteBuffers(1, &m_MeshDatas.m_Ibo);
                LogGlError();
            }

            if (m_MeshDatas.m_Vao > 0) {
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

            glBufferData(GL_ARRAY_BUFFER, m_MeshDatas.verticeSize * m_MeshDatas.m_Vertices.size(), m_MeshDatas.m_Vertices.data(), GL_DYNAMIC_DRAW);
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

            // tex
            glEnableVertexAttribArray(2);
            LogGlError();
            glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, m_MeshDatas.verticeSize, (void*)(sizeof(float) * 6));
            LogGlError();
            glDisableVertexAttribArray(2);
            LogGlError();

            // col
            glEnableVertexAttribArray(3);
            LogGlError();
            glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, m_MeshDatas.verticeSize, (void*)(sizeof(float) * 8));
            LogGlError();
            glDisableVertexAttribArray(3);
            LogGlError();

            if (!m_MeshDatas.m_Indices.empty()) {
                SetIndicesCount(m_MeshDatas.m_Indices.size());
                SetIndicesCountToShow(m_MeshDatas.m_Indices.size());

                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_MeshDatas.m_Ibo);
                LogGlError();

                glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_MeshDatas.indiceSize * m_MeshDatas.m_Indices.size(), m_MeshDatas.m_Indices.data(), GL_DYNAMIC_DRAW);
                LogGlError();
            }

            // unbind
            glBindVertexArray(0);
            LogGlError();
            glBindBuffer(GL_ARRAY_BUFFER, 0);  // bien unbind les buffer apres le vao sinon le contexte est v�rol�
            LogGlError();
            if (!m_MeshDatas.m_Indices.empty()) {
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
                LogGlError();
            }
        }
    }

    return true;
}