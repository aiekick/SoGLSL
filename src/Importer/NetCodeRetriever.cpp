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

#ifdef USE_NETWORK

#include "NetCodeRetriever.h"
#include <stdio.h> 
#include <ctools/Logger.h>
#include <Renderer/RenderPack.h>
#include <Gui/CustomGuiWidgets.h>
#include <CodeTree/CodeTree.h>

std::atomic<float> NetCodeRetriever::sProgress(0.0f);
std::atomic<bool> NetCodeRetriever::sWorking(false);
std::atomic<float> NetCodeRetriever::sGenerationTime(0.0f);
std::atomic<UrlLoadingStatus> NetCodeRetriever::sUrlLoadingStatus(UrlLoadingStatus::URL_LOADING_STATUE_OK);
std::mutex NetCodeRetriever::sWorkerThread_Mutex;
std::list<ShaderInfos> NetCodeRetriever::sShaders = std::list<ShaderInfos>();
std::atomic<bool> NetCodeRetriever::sInportInOneFile(false);
VersionStruct NetCodeRetriever::sVersion = VersionStruct();

/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////

void GetShaderCode(
	std::string vUrl,
	std::string vApiKey,
	std::string vProxyPath,
	std::string vProxyUserPwd,
	std::atomic<UrlLoadingStatus>& vUrlLoadingStatus,
	std::atomic<float>& vProgress,
	std::atomic<bool>& vWorking,
	std::atomic<float>& vGenerationTime,
	std::atomic<bool>& vImportInOneFile)
{
	vProgress = 0.0f;

	vWorking = true;

	vGenerationTime = 0.0f;

	NetCodeRetriever::sWorkerThread_Mutex.lock();
	NetCodeRetriever::sShaders.clear();
	NetCodeRetriever::sWorkerThread_Mutex.unlock();

	auto loader = std::make_unique<ShaderUrlLoader>();
	const std::list<ShaderInfos> shaders = loader->GetShaderFromUrl(
		vUrl,
		vApiKey,
		vProxyPath,
		vProxyUserPwd,
		0,
		std::ref(vUrlLoadingStatus),
		std::ref(vProgress),
		std::ref(vWorking),
		std::ref(vGenerationTime),
		std::ref(vImportInOneFile));

	vWorking = false;

	NetCodeRetriever::sWorkerThread_Mutex.lock();
	NetCodeRetriever::sShaders = shaders;
	NetCodeRetriever::sWorkerThread_Mutex.unlock();
}

/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////

NetCodeRetriever::NetCodeRetriever()
{
	puUseProxy = false;
	puShaderToyApiKey[0] = 0;
	puUrl[0] = 0;
	puProxyPath[0] = 0;
	puUserPwd[0] = 0;
	puCheckVersionAtStart = true;

	NetCodeRetriever::sProgress = 0.0f;
	NetCodeRetriever::sInportInOneFile = false;
	NetCodeRetriever::sWorking = false;
	NetCodeRetriever::sGenerationTime = 0.0f;
	NetCodeRetriever::sUrlLoadingStatus = URL_LOADING_STATUE_OK;
	puGenerationTime = 0.0f;
}

NetCodeRetriever::~NetCodeRetriever()
{

}

//////////////////////////////////////////////////////////////////////

void NetCodeRetriever::SetShaderToyApiKey(const std::string& vApiKey)
{
	auto key = vApiKey;
#ifdef _DEBUG
	if (key == "ApiKey")
		key = "ftnKwW";
#endif
	strncpy(puShaderToyApiKey, key.c_str(), ct::mini<size_t>(key.size(), 100));
	puShaderToyApiKey[key.size()] = '\0';
}

void NetCodeRetriever::SetUrl(const std::string& vUrl)
{
	strncpy(puUrl, vUrl.c_str(), ct::mini<size_t>(vUrl.size(), 1000));
	puUrl[vUrl.size()] = '\0';
}

void NetCodeRetriever::SetProxyPath(const std::string& vProxyPath)
{
	strncpy(puProxyPath, vProxyPath.c_str(), ct::mini<size_t>(vProxyPath.size(), 100));
	puProxyPath[vProxyPath.size()] = '\0';
}

void NetCodeRetriever::SetProxyUserPwd(const std::string& vUserPwd)
{
	strncpy(puUserPwd, vUserPwd.c_str(), ct::mini<size_t>(vUserPwd.size(), 100));
	puUserPwd[vUserPwd.size()] = '\0';
}

void NetCodeRetriever::SetProxyUse(bool vUseProxy)
{
	puUseProxy = vUseProxy;
}

//////////////////////////////////////////////////////////////////////

std::string NetCodeRetriever::GetProxyPath()
{
	return std::string(puProxyPath);
}

std::string NetCodeRetriever::GetProxyUserPwd()
{
	return std::string(puUserPwd);
}

std::string NetCodeRetriever::GetUrl()
{
	return std::string(puUrl);
}

std::string NetCodeRetriever::GetShaderToyApiKey()
{
	return std::string(puShaderToyApiKey);
}

bool NetCodeRetriever::GetProxyUse()
{
	return puUseProxy;
}

//////////////////////////////////////////////////////////////////////

bool NetCodeRetriever::drawUrl(bool* vShow, bool vHideProxySettings, bool vHideShaderToyApiKey)
{
	bool load = false;

	if (!vHideProxySettings)
	{
		ImGui::Checkbox("Proxy ?", &puUseProxy); ImGui::SameLine();
		if (puUseProxy)
		{
			ImGui::PushItemWidth(150.0f);
			ImGui::InputText("##proxy", puProxyPath, 100);
			if (ImGui::IsItemHovered())
				ImGui::SetTooltip("Type here the proxy ip and port in the format => ip:port");
			ImGui::PopItemWidth();
			ImGui::SameLine();
			ImGui::PushItemWidth(150.0f);
			ImGui::InputText("##userpwd", puUserPwd, 100);
			if (ImGui::IsItemHovered())
				ImGui::SetTooltip("Type here the proxy user and password in the format => user:password");
			ImGui::PopItemWidth();
			ImGui::SameLine();
		}
	}

	if (!vHideShaderToyApiKey)
	{
		ImGui::PushItemWidth(50.0f);
		ImGui::InputText("##apikey", puShaderToyApiKey, 100);
		if (ImGui::IsItemHovered())
			ImGui::SetTooltip("Type here the ShaderToy AkiKey here");
		ImGui::PopItemWidth();
		ImGui::SameLine();
	}

	if (puWorkerThread.joinable() && NetCodeRetriever::sProgress > 0.0)
	{
		//char timeBuffer[256];
		//double t = NetCodeRetriever::GenerationTime;
		//snprintf(timeBuffer, 256, "%.2lf s", t);

		const float pr = (float)NetCodeRetriever::sProgress / 100.0f;

		const float w = ImGui::GetIO().DisplaySize.x - ImGui::GetCursorPosX() - 60.0f;

		ImGui::ProgressBar(pr, ImVec2(w, 0), puUrl);

		ImGui::SameLine();

		if (ImGui::ContrastedButton("Stop"))
		{
			StopWorkerThread();
		}
	}
	else
	{
		static float _Width = 0.0f;

		ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x - _Width);

		load |= ImGui::InputText("##url", puUrl, 1000, ImGuiInputTextFlags_EnterReturnsTrue);

		ImGui::PopItemWidth();

		if (ImGui::IsItemHovered())
			ImGui::SetTooltip("Type the url to import here. \n (ShaderToy, GlslSandBox and VertexShaderArt are partially Supported's)");

		float starP = ImGui::GetCursorPosX();

		ImGui::SameLine(); 
		
		bool _InportInOneFile = NetCodeRetriever::sInportInOneFile;
		ImGui::RadioButtonLabeled(ImVec2(120.0f,0.0f), "Import in One File", "Import all Buffers In One File", &_InportInOneFile);
		NetCodeRetriever::sInportInOneFile = _InportInOneFile;

		ImGui::SameLine();

		load |= ImGui::ContrastedButton("Load");

		ImGui::SameLine();

		if (ImGui::ContrastedButton("Hide"))
		{
			*vShow = false;
		}

		_Width = ImGui::GetCursorPosX() - starP;
	}

	FinishIfRequired();

	return load;
}

/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////

void NetCodeRetriever::RetrieveWithoutThread(std::function<void()> vFinishFunc)
{
	puFinishFunc = vFinishFunc;

	NetCodeRetriever::sWorking = true;

	auto loader = std::make_unique<ShaderUrlLoader>();
	loader->puUseProxy = puUseProxy;
	NetCodeRetriever::sShaders = loader->GetShaderFromUrl(
		std::string(puUrl),
		std::string(puShaderToyApiKey),
		std::string(puProxyPath),
		std::string(puUserPwd),
		0,
		std::ref(NetCodeRetriever::sUrlLoadingStatus),
		std::ref(NetCodeRetriever::sProgress),
		std::ref(NetCodeRetriever::sWorking),
		std::ref(NetCodeRetriever::sGenerationTime),
		std::ref(NetCodeRetriever::sInportInOneFile));

	NetCodeRetriever::sWorking = false;

	puFinishFunc();
}

void NetCodeRetriever::ParseVersionString(std::string vVersionString)
{
	if (!vVersionString.empty())
	{
		auto arr = ct::splitStringToVector(vVersionString, "\n");
		if (!arr.empty())
		{
			NetCodeRetriever::sVersion.number = arr[0];
		}
		if (arr.size() > 1)
		{
			NetCodeRetriever::sVersion.url = arr[1];
		}
		if (arr.size() > 2)
		{
			NetCodeRetriever::sVersion.changelog.clear();
			size_t idx = 0;
			for (auto it = arr.begin(); it != arr.end(); ++it)
			{
				if (idx++ > 1)
				{
					NetCodeRetriever::sVersion.changelog += *it + '\n';
				}
			}
		}
	}
}

void NetCodeRetriever::RetrieveLastVersionWithoutThread(std::string vUrl, std::function<void()> vFinishFunc)
{
	puFinishFunc = vFinishFunc;

	NetCodeRetriever::sShaders.clear();

	NetCodeRetriever::sWorking = true;

	auto loader = std::make_unique<ShaderUrlLoader>();
	loader->puUseProxy = puUseProxy;
	const std::string version = loader->GetFileContentFromUrl(
		std::string(vUrl),
		std::string(puProxyPath),
		std::string(puUserPwd),
		5,
		std::ref(NetCodeRetriever::sUrlLoadingStatus),
		std::ref(NetCodeRetriever::sProgress),
		std::ref(NetCodeRetriever::sWorking),
		std::ref(NetCodeRetriever::sGenerationTime));

	ParseVersionString(version);

	NetCodeRetriever::sWorking = false;

	puFinishFunc();
}

void NetCodeRetriever::CreateThread(std::function<void()> vFinishFunc)
{
	if (!StopWorkerThread())
	{
		puFinishFunc = vFinishFunc;
		NetCodeRetriever::sWorking = true;
		puWorkerThread =
			std::thread(
				GetShaderCode,
				std::string(puUrl),
				std::string(puShaderToyApiKey),
				std::string(puProxyPath),
				std::string(puUserPwd),
				std::ref(NetCodeRetriever::sUrlLoadingStatus),
				std::ref(NetCodeRetriever::sProgress),
				std::ref(NetCodeRetriever::sWorking),
				std::ref(NetCodeRetriever::sGenerationTime),
				std::ref(NetCodeRetriever::sInportInOneFile)
			);
	}
}

bool NetCodeRetriever::StopWorkerThread()
{
	bool res = false;

	res = puWorkerThread.joinable();
	if (res)
	{
		NetCodeRetriever::sWorking = false;
		puWorkerThread.join();
		puFinishFunc();
	}

	return res;
}

bool NetCodeRetriever::IsJoinable()
{
	return puWorkerThread.joinable();
}

void NetCodeRetriever::FinishIfRequired()
{
	if (puWorkerThread.joinable() && !NetCodeRetriever::sWorking)
	{
		puWorkerThread.join();
		puFinishFunc();
	}
}

void NetCodeRetriever::Join()
{
	puWorkerThread.join();
}

///////////////////////////////////////////////////////
//// CONFIGURATION ////////////////////////////////////
///////////////////////////////////////////////////////

std::string NetCodeRetriever::getXml(const std::string& vOffset, const std::string& vUserDatas)
{
	UNUSED(vUserDatas);

	std::string str;

	str += vOffset + "<NetCodeRetriever>\n";

	str += vOffset + "\t<proxy_use>" + (puUseProxy ? "true" : "false") + "</proxy_use>\n";
	str += vOffset + "\t<proxy_path>" + std::string(puProxyPath) + "</proxy_path>\n";
	str += vOffset + "\t<proxy_userpwd>" + std::string(puUserPwd) + "</proxy_userpwd>\n";
	str += vOffset + "\t<proxy_url>" + std::string(puUrl) + "</proxy_url>\n";
	str += vOffset + "\t<shadertoy_api_key>" + std::string(puShaderToyApiKey) + "</shadertoy_api_key>\n";
	str += vOffset + "\t<check_version_at_start>" + (puCheckVersionAtStart ? "true" : "false") + "</check_version_at_start>\n";
	str += vOffset + "\t<import_in_one_file>" + (NetCodeRetriever::sInportInOneFile ? "true" : "false") + "</import_in_one_file>\n";
	
	str += vOffset + "</NetCodeRetriever>\n";

	return str;
}

bool NetCodeRetriever::setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas)
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

	if (strName == "NetCodeRetriever")
	{
		for (tinyxml2::XMLElement* child = vElem->FirstChildElement(); child != nullptr; child = child->NextSiblingElement())
		{
			RecursParsingConfig(child->ToElement(), vElem);
		}
	}

	if (strParentName == "NetCodeRetriever")
	{
		if (strName == "proxy_use")
			puUseProxy = ct::ivariant(strValue).GetB();
		if (strName == "proxy_path")
			SetProxyPath(strValue);
		if (strName == "proxy_userpwd")
			SetProxyUserPwd(strValue);
		if (strName == "proxy_url")
			SetUrl(strValue);
		if (strName == "shadertoy_api_key")
			SetShaderToyApiKey(strValue);
		if (strName == "check_version_at_start")
			puCheckVersionAtStart = ct::ivariant(strValue).GetB();
		if (strName == "import_in_one_file")
			NetCodeRetriever::sInportInOneFile = ct::ivariant(strValue).GetB();
	}

	return false;
}

#endif // #ifdef USE_NETWORK