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

#include <map>
#include <list>
#include <vector>
#include <string>
#include <future>
#include <functional>
#include <imgui.h>
#include <picojson/picojson.h>
#include <tinyxml2/tinyxml2.h>
#include <ctools/ConfigAbstract.h>
#include <Importer/ImporterFromShadertoy.h>

class ShadertoyBackupFileInfo
{
public:
	bool selected = false;
	std::string id;
	std::string id_for_search; // lower case for search
	std::string date;
	std::string viewed;
	std::string name;
	std::string username;
	std::string description;
	std::string description_for_search; // lower case for search
	std::string likes;
	std::string published;
	std::string flags;
	std::string tags;
	std::string tags_for_search; // lower case for search
	size_t count_renderpasses;
	std::string hasliked;
	std::string shader_code;
};

class ShadertoyBackupFileImportDlg : public conf::ConfigAbstract
{
private:
	bool puShowDialog = false;
	bool m_CreateOneFileShader = true;
	std::string m_FilePathName; 
	std::vector<ShadertoyBackupFileInfo> m_Shaders;
	std::vector<ShadertoyBackupFileInfo> m_FilteredShaders;
	ImGuiListClipper m_VirtualClipper;
	std::function<std::string(std::string, std::list<ShaderInfos>)> m_CreateManyFilesShaderFunc = nullptr;
	std::function<std::string(std::string, std::list<ShaderInfos>)> m_CreateOneFileShaderFunc = nullptr;
	char m_SearchBuffer[1024 + 1] = "";

public:
	void OpenDialog(const std::string& vFilePathName);
	void CloseDialog();
	bool DrawDialog();

	void SetFunction_For_CreateManyFilesShader(std::function<std::string(std::string, std::list<ShaderInfos>)> vCreateManyFilesShaderFunc);
	void SetFunction_For_CreateOneFileShader(std::function<std::string(std::string, std::list<ShaderInfos>)> vCreateOneFileShaderFunc);

private:
	void Init(const std::string& vFilePathName);
	void DrawContentPane();
	void DrawButtonsPane();

public:
	std::string getXml(const std::string& vOffset, const std::string& vUserDatas = "")override;
	bool setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas = "") override;

private:
	void ParseJsonArray(const picojson::array& vJsonArray);
	void prImportSelectedShaders(const std::string& vPath);
	void prImportAllShaders(const std::string& vPath);
	void prApplyFiltering(const std::string& vSearchPattern);

public:
	static ShadertoyBackupFileImportDlg* Instance()
	{
		static ShadertoyBackupFileImportDlg _instance;
		return &_instance;
	}

protected:
	ShadertoyBackupFileImportDlg(); // Prevent construction
	ShadertoyBackupFileImportDlg(const ShadertoyBackupFileImportDlg&) {}; // Prevent construction by copying
	ShadertoyBackupFileImportDlg& operator =(const ShadertoyBackupFileImportDlg&) { return *this; }; // Prevent assignment
	~ShadertoyBackupFileImportDlg(); // Prevent unwanted destruction
};
