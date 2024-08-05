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

#include "QuadModel.h"

#include <cassert>
#include <ctools/Logger.h>
#include <ImGuiPack.h>
#include <iagp/iagp.h>
#include <Profiler/TracyProfiler.h>
#include <Mesh/Utils/VertexStruct.h>

QuadModelPtr QuadModel::Create(const GuiBackend_Window& vWin) {
    auto res = std::make_shared<QuadModel>(vWin);
    res->m_This = res;
    if (!res->IsValid())
        res.reset();
    return res;
}

QuadModel::QuadModel(const GuiBackend_Window& vWin) : BaseModel(vWin) {
    m_MeshType = BaseMeshEnum::PRIMITIVE_TYPE_QUAD;
    m_IsLoaded = PrepareQuad();
}

QuadModel::~QuadModel() {
    Clear();
}

void QuadModel::Clear() {
    m_MeshDatas.Clear(m_Window);
}

bool QuadModel::IsValid() {
    return m_IsLoaded;
}

void QuadModel::DrawModel(const std::string& vName, const GLenum& vRenderMode, const bool& vUseTesselation) {
    GuiBackend::MakeContextCurrent(m_Window);

    AIGPScoped(vName, "Draw Quad");

    TracyGpuZone("QuadModel::DrawModel");

    if (glIsVertexArray(m_MeshDatas.m_Vao) == GL_TRUE) {
        // bind
        glBindVertexArray(m_MeshDatas.m_Vao);  // select first VAO
        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);

        if (vUseTesselation && m_PatchVerticesCount && glPatchParameteri)
            glPatchParameteri(GL_PATCH_VERTICES, (GLint)m_PatchVerticesCount);

        if (m_InstanceCount > 1U) {
            glDrawElementsInstanced(vRenderMode, (GLsizei)6, GL_UNSIGNED_INT, nullptr, (GLsizei)m_InstanceCount);  // draw first object 6 => decalage 3 coord * 2 (float)
        } else {
            glDrawElements(vRenderMode, (GLsizei)6, GL_UNSIGNED_INT, nullptr);
        }

        // unbind
        glDisableVertexAttribArray(1);
        glDisableVertexAttribArray(0);
        glBindVertexArray(0);
    }
}

uint32_t QuadModel::GetVaoID(uint32_t /*vMeshID*/) {
    return m_MeshDatas.GetVaoID();
}

uint32_t QuadModel::GetVboID(uint32_t /*vMeshID*/) {
    return m_MeshDatas.GetVboID();
}

uint32_t QuadModel::GetIboID(uint32_t /*vMeshID*/) {
    return m_MeshDatas.GetIboID();
}

bool QuadModel::PrepareQuad() {
    GuiBackend::MakeContextCurrent(m_Window);

    TracyGpuZone("QuadModel::PrepareQuad");

    m_MeshDatas.m_Vertices = {
        VertexStruct::P2_T2(ct::fvec2(-1.0f), ct::fvec2(0.0f)),
        VertexStruct::P2_T2(ct::fvec2(1.0f, -1.0f), ct::fvec2(1.0f, 0.0f)),
        VertexStruct::P2_T2(ct::fvec2(1.0f), ct::fvec2(1.0f)),
        VertexStruct::P2_T2(ct::fvec2(-1.0f, 1.0f), ct::fvec2(0.0f, 1.0f)),
    };

    m_MeshDatas.m_Indices = {0, 1, 2, 0, 2, 3};

    SetVerticesCount(m_MeshDatas.m_Vertices.size());
    SetIndicesCount(0);
    SetIndicesCountToShow(0);

    if (m_MeshDatas.m_Vao > 0) {
        glDeleteVertexArrays(1, &m_MeshDatas.m_Vao);
        LogGlError();
    }

    if (m_MeshDatas.m_Vbo > 0) {
        glDeleteBuffers(1, &m_MeshDatas.m_Vbo);
        LogGlError();
    }

    if (m_MeshDatas.m_Ibo > 0) {
        glDeleteBuffers(1, &m_MeshDatas.m_Ibo);
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
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, m_MeshDatas.verticeSize, (void*)nullptr);
    LogGlError();
    glDisableVertexAttribArray(0);
    LogGlError();

    // tex
    glEnableVertexAttribArray(1);
    LogGlError();
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, m_MeshDatas.verticeSize, (void*)(sizeof(float) * 2));
    LogGlError();
    glDisableVertexAttribArray(1);
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

    return true;
}