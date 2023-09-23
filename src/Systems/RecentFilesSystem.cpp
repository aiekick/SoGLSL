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

#include "RecentFilesSystem.h"
#include <ctools/cTools.h>
#include <ctools/FileHelper.h>
#include <algorithm> // std::sort
#include <functional> // lambda
#include <chrono>
#include <ctime>

RecentFilesSystem::RecentFilesSystem() = default;
RecentFilesSystem::~RecentFilesSystem() = default;

void RecentFilesSystem::Clear()
{
	prRecentFilesForCalc.clear();
	prRecentFilesForDiplay.clear();
	puSelectedFile.clear();
}

void RecentFilesSystem::AddFile(const std::string& vFilePathName, const BaseMeshEnum& vType, const size_t& vTimeStamp)
{
	if (vType != BaseMeshEnum::PRIMITIVE_TYPE_NONE && !vFilePathName.empty())
	{
		const std::string path = FileHelper::Instance()->GetPathRelativeToApp(vFilePathName);
		if (!path.empty())
		{
			auto& block = prRecentFilesForCalc[path];
			if (block.use_count() == 0)
			{
				block = std::make_shared<RecentFilesInfos>();
			}

			time_t timeStamp = (time_t)block->timestamp;
			if (vTimeStamp == 0U)
			{
				time(&timeStamp);
			}
			else
			{
				block->timestamp = vTimeStamp;
				timeStamp = (time_t)block->timestamp;
			}

			static char buf[80] = "";
			auto ts = localtime(&timeStamp);
			size_t n = strftime(buf, sizeof(buf), "%Y/%m/%d %Hh %Mm %Ss", ts);
			block->date = std::string(buf, n);

			block->path = path;
			block->type = vType;
			block->timestamp = timeStamp;

			// Re Build list for display
			prRecentFilesForDiplay.clear();
			for (auto b : prRecentFilesForCalc)
			{
				prRecentFilesForDiplay.push_back(b.second);
			}
			if (prRecentFilesForDiplay.size() > 1U)
			{
				std::sort(prRecentFilesForDiplay.begin(), prRecentFilesForDiplay.end(),
					[](const std::weak_ptr<RecentFilesInfos>& a, const std::weak_ptr<RecentFilesInfos>& b)
					{
						auto ptr_a = a.lock();
						auto ptr_b = b.lock();
						if (!ptr_a || !ptr_b)
							return false;
						return (ptr_a->timestamp > ptr_b->timestamp);
					});
			}
		}
	}
}

bool RecentFilesSystem::DrawMenu(const char *vTitle)
{
	bool change = false;

	if (!prRecentFilesForDiplay.empty())
	{
		if (ImGui::BeginMenu(vTitle))
		{
			if (ImGui::MenuItem("Clear"))
			{
				Clear();
			}

			ImGui::Separator();

			static ImGuiTableFlags flags = 
				ImGuiTableFlags_SizingFixedFit | 
				ImGuiTableFlags_RowBg |
				ImGuiTableFlags_Hideable | 
				ImGuiTableFlags_ScrollY |
				ImGuiTableFlags_NoHostExtendY;
			if (ImGui::BeginTable("##RecentFiles", 3, flags)) //-V112
			{
				ImGui::TableSetupScrollFreeze(0, 1); // Make header always visible
				ImGui::TableSetupColumn("File", ImGuiTableColumnFlags_WidthStretch, -1, 0);
				ImGui::TableSetupColumn("Date", ImGuiTableColumnFlags_WidthFixed, -1, 1);
				ImGui::TableSetupColumn("Type", ImGuiTableColumnFlags_WidthFixed, -1, 2);
				ImGui::TableHeadersRow();

				for (const auto& block : prRecentFilesForDiplay)
				{
					auto ptr_block = block.lock();
					if (ptr_block.use_count())
					{
						const char* type = "?";
						switch (ptr_block->type)
						{
						case BaseMeshEnum::PRIMITIVE_TYPE_QUAD:
							type = "Quad";
							break;
						case BaseMeshEnum::PRIMITIVE_TYPE_POINTS:
							type = "Points";
							break;
						case BaseMeshEnum::PRIMITIVE_TYPE_MESH:
							type = "Mesh";
							break;
						default:
							break;
						}

						if (ImGui::TableNextColumn()) // path
						{
							if (ImGui::Selectable(ptr_block->path.c_str()))
							{
								puSelectedFile = ptr_block->path;
								change = true;
							}
						}
						if (ImGui::TableNextColumn()) // date
						{
							ImGui::Text(ptr_block->date.c_str());
						}
						if (ImGui::TableNextColumn()) // type
						{
							ImGui::Text(type);
						}
					}
				}

				ImGui::EndTable();
			}

			ImGui::EndMenu();
		}
	}

	return change;
}

std::string RecentFilesSystem::GetSelectedFile()
{
	return puSelectedFile;
}

///////////////////////////////////////////////////////
//// CONFIGURATION ////////////////////////////////////
///////////////////////////////////////////////////////

std::string RecentFilesSystem::getXml(const std::string& vOffset, const std::string& vUserDatas)
{
	UNUSED(vUserDatas);

	std::string str;

	str += vOffset + "<RecentFilesSystem>\n";
	const std::string offset = vOffset + vOffset;

	for (const auto& block : prRecentFilesForCalc)
	{
		if (block.second.use_count())
			str += offset + 
				"<file type=\"" + ct::toStr((int)block.second->type) + 
				"\" stamp=\"" + ct::toStr(block.second->timestamp) + 
				"\" path=\"" + block.second->path + "\"/>\n";
	}

	str += vOffset + "</RecentFilesSystem>\n";

	return str;
}

bool RecentFilesSystem::setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas)
{
	UNUSED(vUserDatas);

	// The value of this child identifies the name of this element
	std::string strName;
	std::string strValue;
	std::string strParentName;

	strName = vElem->Value();
	if (vElem->GetText())
		strValue = vElem->GetText();
	if (vParent != nullptr)
		strParentName = vParent->Value();

	if (strName == "RecentFilesSystem")
	{
		for (tinyxml2::XMLElement* child = vElem->FirstChildElement(); child != nullptr; child = child->NextSiblingElement())
		{
			RecursParsingConfig(child->ToElement(), vElem);
		}
	}

	if (strParentName == "RecentFilesSystem")
	{
		if (strName == "file")
		{
			std::string path;
			BaseMeshEnum type = BaseMeshEnum::PRIMITIVE_TYPE_NONE;
			size_t stamp = std::string::npos;

			for (const tinyxml2::XMLAttribute* attr = vElem->FirstAttribute(); attr != nullptr; attr = attr->Next())
			{
				std::string attName = attr->Name();
				std::string attValue = attr->Value();

				if (attName == "path")
					path = attValue;
				else if (attName == "type")
					type = static_cast<BaseMeshEnum>(ct::ivariant(attValue).GetI());
				else if (attName == "stamp")
					stamp = static_cast<size_t>(ct::uvariant(attValue).GetU());
			}

			AddFile(path, type, stamp);
		}
	}

	return false;
}