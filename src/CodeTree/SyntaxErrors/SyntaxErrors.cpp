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

#include "SyntaxErrors.h"
#include <CodeTree/ShaderKey.h>
#include <CodeTree/CodeTree.h>
#include <Renderer/Shader.h>
#include <Messaging.h>
#include <Res/CustomFont.h>

SyntaxErrors::SyntaxMessagingFunction SyntaxErrors::s_SyntaxMessagingFunction = 0;

SyntaxErrors::SyntaxErrors()
{
	puIsThereSomeErrors = false;
	puIsThereSomeWarnings = false;
}

SyntaxErrors::~SyntaxErrors() = default;

void SyntaxErrors::clear(const std::string & vErrorConcern)
{
	if (!vErrorConcern.empty())
		puInfos[vErrorConcern].clear();
	else
		puInfos.clear();

	puIsThereSomeErrors = false;
	puIsThereSomeWarnings = false;

	//m_ParentKeyName;
}

void SyntaxErrors::SetSyntaxError(
	std::weak_ptr<ShaderKey> vKey, const std::string & vErrorConcern,
	const std::string & vErrorType, bool vErrorOrWarnings,
	const LineFileErrors & vLineFileErrorsType)
{
	auto vKeyPtr = vKey.lock();
	if (vKeyPtr.use_count())
	{
		for (auto& it : vLineFileErrorsType.errLine)
		{
			for (auto& it2 : it.second)
			{
				vKeyPtr->GetSyntaxErrors()->puInfos[vErrorConcern][vErrorOrWarnings][vErrorType][it2.first][it.first] = it2.second;
				if (s_SyntaxMessagingFunction)
				{
					s_SyntaxMessagingFunction(vErrorConcern, (vErrorOrWarnings ? "error" : "warning"), vErrorType, it2.second);
				}
			}
		}

		if (vErrorOrWarnings)
			puIsThereSomeErrors |= true;
		else
			puIsThereSomeWarnings |= true;
	}
}

bool SyntaxErrors::isThereSomeSyntaxMessages(std::weak_ptr<ShaderKey> vKey, bool vErrorOrWarnings)
{
	bool res = false;

	auto vKeyPtr = vKey.lock();
	if (vKeyPtr.use_count())
	{
		auto codetree = vKeyPtr->puParentCodeTree;
		if (codetree)
		{
			for (const auto& m_IncludeFileName : vKeyPtr->puIncludeFileNames)
			{
				auto key = codetree->GetIncludeKey(m_IncludeFileName.first);
				if (key)
				{
					res |= isThereSomeSyntaxMessages(key, vErrorOrWarnings);
				}
			}
		}

		for (auto& m_Error : vKeyPtr->GetSyntaxErrors()->puInfos)
		{
			if (res)
				break;

			for (auto& it2 : m_Error.second)
			{
				if (it2.first == vErrorOrWarnings)
				{
					if (!it2.second.empty())
					{
						res = true;

						break;
					}
				}
			}
		}
	}

	return res;
}

std::string SyntaxErrors::toString(std::weak_ptr<ShaderKey> vKey, bool vErrorOrWarnings)
{
	std::string res;

	auto vKeyPtr = vKey.lock();
	if (vKeyPtr.use_count())
	{
		/*
		for (auto & m_Error : vKey->GetSyntaxErrors()->puInfos)
		{
			for (auto & it2 : m_Error.second)
			{
				if (it2.first == vErrorOrWarnings)
				{
					for (auto & it3 : it2.second)
					{
						for (auto & it4 : it3.second)
						{
							for (auto & it5 : it4->second)
							{
								res += it5->second;
							}
						}
					}
				}
			}
		}
		*/

		if (vKeyPtr->puParentCodeTree)
		{
			for (auto incFile : vKeyPtr->puIncludeFileNames)
			{
				auto key = vKeyPtr->puParentCodeTree->GetIncludeKey(incFile.first);
				if (key)
				{
					res += toString(key, vErrorOrWarnings);
				}
			}
		}
	}

	return res;
}

/////////////////////////////////////////////


bool SyntaxErrors::CollapsingHeaderError(const char* vLabel, bool vForceExpand, bool vShowEditButton, bool* vEditCatched)
{
	bool res = false;

	ImGui::PushID(ImGui::IncPUSHID());

	if (puIsThereSomeErrors)
	{
		ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.8f, 0.2f, 0.2f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.8f, 0.6f, 0.6f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(0.8f, 0.4f, 0.4f, 1.0f));
		char buf[256];
		snprintf(buf, 256, "%s (There is some %s)", vLabel, "Errors");
		if (*vLabel == 0)
			vShowEditButton = false;
		res = ImGui::CollapsingHeader_Button(buf, -1, vForceExpand, ICON_NDP_PENCIL_SQUARE_O, vShowEditButton, vEditCatched);
		ImGui::PopStyleColor(3);
	}
	else
	{
		res
			= ImGui::CollapsingHeader_Button(vLabel, -1, vForceExpand, ICON_NDP_PENCIL_SQUARE_O, vShowEditButton, vEditCatched);
	}

	ImGui::PopID();

	return res;
}

bool SyntaxErrors::CollapsingHeaderWarnings(const char* vLabel, bool vForceExpand, bool vShowEditButton, bool* vEditCatched)
{
	bool res = false;

	ImGui::PushID(ImGui::IncPUSHID());

	if (puIsThereSomeWarnings)
	{
		ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.8f, 0.5f, 0.2f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.8f, 0.6f, 0.2f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(0.8f, 0.4f, 0.2f, 1.0f));
		char buf[256];
		snprintf(buf, 256, "%s (There is some %s)", vLabel, "Warnings");
		res = ImGui::CollapsingHeader_Button(buf, -1, vForceExpand, ICON_NDP_PENCIL_SQUARE_O, vShowEditButton, vEditCatched);
		ImGui::PopStyleColor(3);
	}
	else
	{
		res = ImGui::CollapsingHeader_Button(vLabel, -1, vForceExpand, ICON_NDP_PENCIL_SQUARE_O, vShowEditButton, vEditCatched);
	}

	ImGui::PopID();

	return res;
}

bool SyntaxErrors::ImGui_DisplayMessages(ShaderKeyPtr vKey, const char* /*vLabel*/, bool vForceExpand, bool vShowEditButton, bool* vEditCatched)
{
	bool res = false;

	if (puIsThereSomeErrors)
	{
		res |= ImGui_DisplayMessages(true, vKey, vForceExpand, vShowEditButton, vEditCatched);
	}

	if (puIsThereSomeWarnings)
	{
		res |= ImGui_DisplayMessages(false, vKey, vForceExpand, vShowEditButton, vEditCatched);
	}

	return res;
}

bool SyntaxErrors::ImGui_DisplayMessages(bool vErrorOrWarnings, ShaderKeyPtr vKey, bool vForceExpand, bool vShowEditButton, bool* vEditCatched)
{
	bool res = false;
	
	if (vKey)
	{
		char buffer[2048] = "\0";
		char title[512] = "\0";

		// puErrors[vErrorConcern][vErrorOrWarnings][vErrorType][file][line] = error
		for (const auto& err : vKey->GetSyntaxErrors()->puInfos) // vErrorConcern
		{
			//ImGui::Text(err.first.c_str());

			for (const auto& err2 : err.second) // vErrorOrWarnings
			{
				if (err2.first == vErrorOrWarnings)
				{
					for (const auto& err3 : err2.second) // vErrorType
					{
						for (const auto& err4 : err3.second) // file
						{
							snprintf(title, 511, "%s : %s", err3.first.c_str(), err4.first.c_str());
							if (vErrorOrWarnings)
							{
								res = CollapsingHeaderError(title, vForceExpand, vShowEditButton, vEditCatched);
							}
							else
							{
								res = CollapsingHeaderWarnings(title, vForceExpand, vShowEditButton, vEditCatched);
							}
							if (*vEditCatched)
							{
								if (err4.first != vKey->puMainSection->relativeFile)
									CTOOL_DEBUG_BREAK;

								FileHelper::Instance()->OpenFile(err4.first);
								*vEditCatched = false;
							}
							if (res)
							{
								for (const auto& err5 : err4.second) // line -> errors
								{
									size_t idx = 0U;
									for (const auto& err6 : err5.second.fragments)
									{
										if (idx++)
											ImGui::SameLine();

										if (err6.file.size())
										{
											ImGui::ClickableTextFile(err6.error.c_str(), err6.file.c_str());
										}
										else
										{
											ImGui::Text(err6.error.c_str());
										}
									}
								}
							}
						}
					}
				}
			}
		}

		if (vKey->puParentCodeTree)
		{
			for (auto it = vKey->puIncludeFileNames.begin(); it != vKey->puIncludeFileNames.end(); ++it)
			{
				const auto key = vKey->puParentCodeTree->GetIncludeKey(it->first);
				if (key)
				{
					res |= ImGui_DisplayMessages(vErrorOrWarnings, key, vForceExpand, vShowEditButton, vEditCatched);
				}
			}
		}
	}

	return res;
}

void SyntaxErrors::CompleteWithShader(ShaderKeyPtr vKey, ShaderPtr vShader)
{
	if (vShader && vKey)
	{
		puIsThereSomeErrors = isThereSomeSyntaxMessages(vKey, true);
		puIsThereSomeWarnings = isThereSomeSyntaxMessages(vKey, false);

		if (vShader->puState.isThereSomeErrors())
		{
			if (!vShader->GetLastShaderErrorString(ShaderTypeEnum::SHADER_TYPE_VERTEX).empty())
			{
				auto errorFileLine = vKey->GetShaderErrorWithGoodLineNumbers(vShader, ShaderTypeEnum::SHADER_TYPE_VERTEX, true);
				SetSyntaxError(vKey, "Compilation error :", "Vertex Error", true, errorFileLine);
			}
			if (!vShader->GetLastShaderErrorString(ShaderTypeEnum::SHADER_TYPE_GEOMETRY).empty())
			{
				auto errorFileLine = vKey->GetShaderErrorWithGoodLineNumbers(vShader, ShaderTypeEnum::SHADER_TYPE_GEOMETRY, true);
				SetSyntaxError(vKey, "Compilation error :", "Geometry Error", true, errorFileLine);
			}
			if (!vShader->GetLastShaderErrorString(ShaderTypeEnum::SHADER_TYPE_TESSELATION_CONTROL).empty())
			{
				auto errorFileLine = vKey->GetShaderErrorWithGoodLineNumbers(vShader, ShaderTypeEnum::SHADER_TYPE_TESSELATION_CONTROL, true);
				SetSyntaxError(vKey, "Compilation error :", "Tesselation Control Error", true, errorFileLine);
			}
			if (!vShader->GetLastShaderErrorString(ShaderTypeEnum::SHADER_TYPE_TESSELATION_EVAL).empty())
			{
				auto errorFileLine = vKey->GetShaderErrorWithGoodLineNumbers(vShader, ShaderTypeEnum::SHADER_TYPE_TESSELATION_EVAL, true);
				SetSyntaxError(vKey, "Compilation error :", "Tesselation Eval Error", true, errorFileLine);
			}
			if (!vShader->GetLastShaderErrorString(ShaderTypeEnum::SHADER_TYPE_FRAGMENT).empty())
			{
				auto errorFileLine = vKey->GetShaderErrorWithGoodLineNumbers(vShader, ShaderTypeEnum::SHADER_TYPE_FRAGMENT, true);
				SetSyntaxError(vKey, "Compilation error :", "Fragment Error", true, errorFileLine);
			}
			if (!vShader->GetLastShaderErrorString(ShaderTypeEnum::SHADER_TYPE_COMPUTE).empty())
			{
				auto errorFileLine = vKey->GetShaderErrorWithGoodLineNumbers(vShader, ShaderTypeEnum::SHADER_TYPE_COMPUTE, true);
				SetSyntaxError(vKey, "Compilation error :", "Compute Error", true, errorFileLine);
			}
			if (!vShader->GetLastShaderErrorString(ShaderTypeEnum::LINK_SPECIAL_TYPE).empty())
			{
				auto errorFileLine = vKey->GetShaderErrorWithGoodLineNumbers(vShader, ShaderTypeEnum::LINK_SPECIAL_TYPE, true);
				SetSyntaxError(vKey, "Compilation error :", "Link Error", true, errorFileLine);
			}
		}

		if (vShader->puState.isThereSomeWarnings())
		{
			if (!vShader->GetLastShaderWarningsString(ShaderTypeEnum::SHADER_TYPE_VERTEX).empty())
			{
				auto errorFileLine = vKey->GetShaderErrorWithGoodLineNumbers(vShader, ShaderTypeEnum::SHADER_TYPE_VERTEX, false);
				SetSyntaxError(vKey, "Compilation error :", "Vertex Warning", false, errorFileLine);
			}
			if (!vShader->GetLastShaderWarningsString(ShaderTypeEnum::SHADER_TYPE_GEOMETRY).empty())
			{
				auto errorFileLine = vKey->GetShaderErrorWithGoodLineNumbers(vShader, ShaderTypeEnum::SHADER_TYPE_GEOMETRY, false);
				SetSyntaxError(vKey, "Compilation error :", "Geometry Warning", false, errorFileLine);
			}
			if (!vShader->GetLastShaderErrorString(ShaderTypeEnum::SHADER_TYPE_TESSELATION_CONTROL).empty())
			{
				auto errorFileLine = vKey->GetShaderErrorWithGoodLineNumbers(vShader, ShaderTypeEnum::SHADER_TYPE_TESSELATION_CONTROL, false);
				SetSyntaxError(vKey, "Compilation error :", "Tesselation Control Warning", false, errorFileLine);
			}
			if (!vShader->GetLastShaderErrorString(ShaderTypeEnum::SHADER_TYPE_TESSELATION_EVAL).empty())
			{
				auto errorFileLine = vKey->GetShaderErrorWithGoodLineNumbers(vShader, ShaderTypeEnum::SHADER_TYPE_TESSELATION_EVAL, false);
				SetSyntaxError(vKey, "Compilation error :", "Tesselation Eval Warning", false, errorFileLine);
			}
			if (!vShader->GetLastShaderWarningsString(ShaderTypeEnum::SHADER_TYPE_FRAGMENT).empty())
			{
				auto errorFileLine = vKey->GetShaderErrorWithGoodLineNumbers(vShader, ShaderTypeEnum::SHADER_TYPE_FRAGMENT, false);
				SetSyntaxError(vKey, "Compilation error :", "Fragment Warning", false, errorFileLine);
			}
			if (!vShader->GetLastShaderWarningsString(ShaderTypeEnum::SHADER_TYPE_COMPUTE).empty())
			{
				auto errorFileLine = vKey->GetShaderErrorWithGoodLineNumbers(vShader, ShaderTypeEnum::SHADER_TYPE_COMPUTE, false);
				SetSyntaxError(vKey, "Compilation error :", "Compute Warning", false, errorFileLine);
			}
			if (!vShader->GetLastShaderWarningsString(ShaderTypeEnum::LINK_SPECIAL_TYPE).empty())
			{
				auto errorFileLine = vKey->GetShaderErrorWithGoodLineNumbers(vShader, ShaderTypeEnum::LINK_SPECIAL_TYPE, false);
				SetSyntaxError(vKey, "Compilation error :", "Link Warning", false, errorFileLine);
			}
		}
	}
}