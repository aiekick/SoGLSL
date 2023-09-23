#ifdef USE_NETWORK
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
#include <tinyxml2/tinyxml2.h>
#include <curl/curl.h>

#include <Headers/RenderPackHeaders.h>

#include "ShaderUrlLoader.h"

#include <string>
#include <stdlib.h>
#include <string>
#include <list>
#include <fstream>
#include <unordered_map>
#include <exception>
#include <list>
#include <future>
#include <functional>
#include <thread>
#include <atomic>
#include <mutex>

struct VersionStruct
{
	std::string number;
	std::string url;
	std::string changelog;

	VersionStruct()
	{
		number.clear();
		url.clear();
		changelog.clear();
	}
};

class NetCodeRetriever : public conf::ConfigAbstract
{
public:
	static std::mutex sWorkerThread_Mutex;
	static std::atomic<float> sProgress;
	static std::atomic<bool> sWorking;
	static std::atomic<float> sGenerationTime;
	static std::atomic<UrlLoadingStatus> sUrlLoadingStatus;
	static std::atomic<bool> sInportInOneFile;
	static VersionStruct sVersion;
	static std::list<ShaderInfos> sShaders;

private:
	bool puUseProxy = false;
#ifdef _DEBUG
	char puShaderToyApiKey[100] = "ftnKwW";
#else
	char puShaderToyApiKey[100] = "";
#endif
	char puUrl[1000] = "";
	char puProxyPath[100] = "";
	char puUserPwd[100] = "";

public:
	bool puCheckVersionAtStart = false;

private:
	std::thread puWorkerThread;
	float puGenerationTime = 0.0f;
	std::function<void()> puFinishFunc;

public:
	void SetProxyPath(const std::string& vProxyPath);
	void SetProxyUserPwd(const std::string& vUserPwd);
	void SetUrl(const std::string& vUrl);
	void SetShaderToyApiKey(const std::string& vApiKey);
	void SetProxyUse(bool vUseProxy);

	std::string GetProxyPath();
	std::string GetProxyUserPwd();
	std::string GetUrl();
	std::string GetShaderToyApiKey();
	bool GetProxyUse();

public:
	static NetCodeRetriever* Instance()
	{
		static NetCodeRetriever _instance;
		return &_instance;
	}

protected:
	NetCodeRetriever(); // Prevent construction
	NetCodeRetriever(const NetCodeRetriever&) {}; // Prevent construction by copying
	NetCodeRetriever& operator =(const NetCodeRetriever&) { return *this; }; // Prevent assignment
	~NetCodeRetriever(); // Prevent unwanted destruction
	
public:
	bool drawUrl(bool *vShow, bool vHideProxySettings = false, bool vHideShaderToyApiKey = false);
	void ParseVersionString(std::string vVersionString);
	void RetrieveWithoutThread(std::function<void()> vFinishFunc);
	void RetrieveLastVersionWithoutThread(std::string vUrl, std::function<void()> vFinishFunc);
	void CreateThread(std::function<void()> vFinishFunc);
	bool StopWorkerThread();
	bool IsJoinable();
	void Join();
	void FinishIfRequired();

public:
	std::string getXml(const std::string& vOffset, const std::string& vUserDatas);
	bool setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas);
};
#endif // #ifdef USE_NETWORK