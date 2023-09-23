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

#include <ctools/ConfigAbstract.h>
#include <ctools/cTools.h>
#include <Headers/RenderPackHeaders.h>
#include <CodeTree/Parsing/ShaderStageParsing.h>
#include <unordered_map>
#include <string>
#include <vector>
#include <memory>

class RecentFilesInfos
{
public:
	std::string path;
	BaseMeshEnum type = BaseMeshEnum::PRIMITIVE_TYPE_NONE;
	size_t timestamp = 0U;
	std::string date;
};

class RecentFilesSystem : public conf::ConfigAbstract
{
private:
	std::unordered_map<std::string, std::shared_ptr<RecentFilesInfos>> prRecentFilesForCalc;
	std::vector<std::weak_ptr<RecentFilesInfos>> prRecentFilesForDiplay;
	std::string puSelectedFile;

public:
	void Clear();
	void AddFile(const std::string& vFilePathName, const BaseMeshEnum& vType, const size_t& vTimeStamp = 0U);

	bool DrawMenu(const char *vTitle);
	std::string GetSelectedFile();

	///////////////////////////////////////////////////////
	//// CONFIGURATION ////////////////////////////////////
	///////////////////////////////////////////////////////

	std::string getXml(const std::string& vOffset, const std::string& vUserDatas);
	bool setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas);

public:
	static RecentFilesSystem* Instance()
	{
		static RecentFilesSystem _instance;
		return &_instance;
	}

protected:
	RecentFilesSystem(); // Prevent construction
	RecentFilesSystem(const RecentFilesSystem&) {}; // Prevent construction by copying
	RecentFilesSystem& operator =(const RecentFilesSystem&) { return *this; }; // Prevent assignment
	~RecentFilesSystem(); // Prevent unwanted destruction
};
