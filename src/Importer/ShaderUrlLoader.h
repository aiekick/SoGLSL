#ifdef USE_NETWORK
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

#include <ctools/ConfigAbstract.h>

#include <Headers/RenderPackHeaders.h>

#include <ctools/cTools.h>

#include <stdio.h>

#include <curl/curl.h>

#include <string>
#include <string>
#include <list>
#include <unordered_map>
#include <vector>
#include <list>

#include <future>
#include <functional>
#include <atomic>

enum UrlLoadingStatus { URL_LOADING_STATUE_OK = 0, URL_LOADING_STATUE_IN_PROGRESS, URL_LOADING_STATUE_ERROR, URL_LOADING_STATUE_Count };

class ShaderUrlLoader {
public:
    static void InitCurl();

private:  // buffer links
    std::unordered_map<int, int> puChannelLinks;
    std::unordered_map<int, std::unordered_map<std::string, std::string>> puTexturesId;
    std::unordered_map<int, std::unordered_map<std::string, std::string>> puCubeMapsId;
    std::unordered_map<int, std::unordered_map<std::string, std::string>> puTextures3DId;
    std::unordered_map<int, std::unordered_map<std::string, std::string>> puTexturesSoundId;
    std::unordered_map<int, std::string> puBuffersName;
    std::unordered_map<int, std::string> puBuffersId;
    std::unordered_map<int, std::string> puBufferCubeMapsId;
    std::unordered_map<int, ShaderInfos*> puBuffersInfos;
    std::unordered_map<int, std::unordered_map<std::string, std::string>> puBuffersParams;
    std::unordered_map<std::string, std::string> puCodeReplacements;

private:
    char puErrorBuffer[CURL_ERROR_SIZE] = "\0";

public:
    bool puUseProxy = false;
    std::string puUrl;
    std::string puProxyPath;
    std::string puUserPwd;
    std::string puShaderToyApiKey;

public:
    ShaderUrlLoader();
    ~ShaderUrlLoader();

    std::list<ShaderInfos> GetShaderFromUrl(std::string vUrl,
                                            std::string vApiKey,
                                            std::string vProxyPath,
                                            std::string vProxyUserPwd,
                                            int vTimeOutInSecond,
                                            std::atomic<UrlLoadingStatus>& vUrlLoadingStatus,
                                            std::atomic<float>& vProgress,
                                            std::atomic<bool>& vWorking,
                                            std::atomic<float>& vGenerationTime,
                                            std::atomic<bool>& vImportInOneFile);

    std::string GetFileContentFromUrl(std::string vUrl,
                                      std::string vProxyPath,
                                      std::string vProxyUserPwd,
                                      int vTimeOutInSecond,
                                      std::atomic<UrlLoadingStatus>& vUrlLoadingStatus,
                                      std::atomic<float>& vProgress,
                                      std::atomic<bool>& vWorking,
                                      std::atomic<float>& vGenerationTime,
                                      std::string* vErrors = nullptr);

private:
    std::string Curl_Init(CURL*& conn, char* url, int vTimeOutInSecond);
    std::string GetRealCodeUrl(std::string vBaseUrl, ShaderPlaform* vSPF, std::string* vId, std::string vApiKey, std::string* vError = nullptr);
    std::list<ShaderInfos> ParseBuffer(std::string vBuffer,
                                       ShaderPlaform vSPF,
                                       std::string vId,
                                       std::atomic<float>& vProgress,
                                       std::atomic<bool>& vWorking,
                                       std::atomic<float>& vGenerationTime,
                                       std::atomic<bool>& vImportInOneFile);
    void AddUniformsFromCode(ShaderInfos* vShaderInfos,
                             std::string* vCode,
                             ShaderPlaform vShaderPlaform,
                             std::atomic<bool>& vImportInOneFile,
                             std::string* vCommonCode = nullptr);
    void DoChannelInput(std::string vBaseName, ShaderInfos* vShaderInfos, int vChanId, size_t nOccurs, std::atomic<bool>& vImportInOneFile);
    void SetShaderBufferFormat(int vBufferId);
    std::string GetFileFromUrl(std::string vUrl, int vTimeOutInSecond, FILE_LOCATION_Enum vAssetType, std::string* vErrors);
    void ReSavePctureFile(std::string vPictureFile);
    std::vector<std::string> ParseUniform(std::string vUniformStr);
};

#endif  // #ifdef USE_NETWORK
