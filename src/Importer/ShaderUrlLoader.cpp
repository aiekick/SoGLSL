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
#include "ShaderUrlLoader.h"
#include <stdio.h> 
#include <ctools/Logger.h>
#include <Renderer/RenderPack.h>
#include <CodeTree/CodeTree.h>
#include <CodeTree/Parsing/ShaderStageParsing.h>
#include <stb/stb_image.h>
#include <stb_image_write.h>

#include "NetCodeRetriever.h"
#include "Format_ShaderToy.h"
#include "Format_GlslSandbox.h"
#include "Format_VertexShaderArt.h"

// static
void ShaderUrlLoader::InitCurl()
{
	// Ensure one argument is given
	curl_global_init(CURL_GLOBAL_DEFAULT);
}

////////////////////////////////////////

ShaderUrlLoader::ShaderUrlLoader()
{

}

ShaderUrlLoader::~ShaderUrlLoader()
{

}

// static
int writer(char *data, size_t size, size_t nmemb, std::string *writerData)
{
	if (writerData == nullptr)
		return 0;

	writerData->append(data, size * nmemb);

	return (int)(size * nmemb);
}

std::list<ShaderInfos> ShaderUrlLoader::GetShaderFromUrl(
	std::string vUrl,
	std::string vApiKey,
	std::string vProxyPath,
	std::string vProxyUserPwd, 
	int vTimeOutInSecond,
	std::atomic<UrlLoadingStatus>& vUrlLoadingStatus,
	std::atomic<float>& vProgress,
	std::atomic<bool>& vWorking,
	std::atomic<float>& vGenerationTime,
	std::atomic<bool>& vImportInOneFile)
{
	vProgress = 0.0f;

	char errorBuffer[256];

	puUrl = vUrl;
	puShaderToyApiKey = vApiKey;
	puProxyPath = vProxyPath;
	puUserPwd = vProxyUserPwd;

	std::list<ShaderInfos> shaders;

	if (!puUrl.empty())
	{
		CURL *conn = nullptr;
		CURLcode code;
	//	std::string title;

		std::string buffer;

		std::string id;
		std::string error;
		ShaderPlaform spf;
		vUrl = GetRealCodeUrl(vUrl, &spf, &id, vApiKey, &error);

		if (!error.empty())
		{
			shaders.emplace_front(ShaderInfos());
			ShaderInfos *inf = &(*shaders.begin());
			inf->error = error;
			vUrlLoadingStatus = UrlLoadingStatus::URL_LOADING_STATUE_ERROR;
		}
		else
		{
			const int BufSize = 5000;
			char url[BufSize];
			size_t size = ct::mini<size_t>(BufSize, vUrl.length());
			strncpy(url, vUrl.c_str(), size);
			url[size] = '\0';

			std::string errors = Curl_Init(conn, url, vTimeOutInSecond);
			if (errors.empty())
			{
				if (vWorking)
				{
					code = curl_easy_setopt(conn, CURLOPT_WRITEFUNCTION, writer);
					if (code != CURLE_OK)
					{
						snprintf(errorBuffer, 256, "Failed to set writer [%s]\n", puErrorBuffer);
						LogVarError(errorBuffer);

						shaders.emplace_front(ShaderInfos());
						ShaderInfos *inf = &(*shaders.begin());
						inf->error = std::string(errorBuffer);
						vUrlLoadingStatus = UrlLoadingStatus::URL_LOADING_STATUE_ERROR;
						vWorking = false;
					}
					else
					{
						code = curl_easy_setopt(conn, CURLOPT_WRITEDATA, &buffer);
						if (code != CURLE_OK)
						{
							snprintf(errorBuffer, 256, "CURLOPT_WRITEDATA fail [%s]\n", puErrorBuffer);
							LogVarError(errorBuffer);

							shaders.emplace_front(ShaderInfos());
							ShaderInfos *inf = &(*shaders.begin());
							inf->error = std::string(errorBuffer);
							vUrlLoadingStatus = UrlLoadingStatus::URL_LOADING_STATUE_ERROR;
							vWorking = false;
						}
						if (vWorking)
						{
							if (code == CURLE_OK)
							{
								code = curl_easy_perform(conn);
								if (code != CURLE_OK)
								{
									snprintf(errorBuffer, 256, "curl_easy_perform fail [%s]\n", puErrorBuffer);
									LogVarError(errorBuffer);

									shaders.emplace_front(ShaderInfos());
									ShaderInfos *inf = &(*shaders.begin());
									inf->error = std::string(errorBuffer);
									vUrlLoadingStatus = UrlLoadingStatus::URL_LOADING_STATUE_ERROR;
									vWorking = false;
								}
								if (vWorking)
								{
									if (code == CURLE_OK)
									{
										if (!buffer.empty())
										{
											shaders = ParseBuffer(
												buffer,
												spf,
												id,
												std::ref(vProgress),
												std::ref(vWorking),
												std::ref(vGenerationTime),
												std::ref(vImportInOneFile));
											vUrlLoadingStatus = UrlLoadingStatus::URL_LOADING_STATUE_OK;
										}
									}
									else
									{
										if (code == CURLcode::CURLE_UNSUPPORTED_PROTOCOL)
										{
											code = curl_easy_setopt(conn, CURLOPT_ERRORBUFFER, puErrorBuffer);

											snprintf(errorBuffer, 256, "curl_easy_setopt fail [%s]\n", puErrorBuffer);
											LogVarError(errorBuffer);

											shaders.emplace_front(ShaderInfos());
											ShaderInfos *inf = &(*shaders.begin());
											inf->error = std::string(errorBuffer);
											vUrlLoadingStatus = UrlLoadingStatus::URL_LOADING_STATUE_ERROR;
											vWorking = false;
										}
									}
								}
							}
						}

						curl_easy_cleanup(conn);
					}
				}
			}
		}
	}

	return shaders;
}

std::string ShaderUrlLoader::GetFileFromUrl(std::string vUrl, int vTimeOutInSecond, FILE_LOCATION_Enum vAssetType, std::string *vErrors)
{
	char errorBuffer[256];

	//std::string sample_url = "https://www.shadertoy.com/media/a/793a105653fbdadabdc1325ca08675e1ce48ae5f12e37973829c87bea4be3232.png";
	//vUrl = sample_url;

	// parse url for get the filename
	std::vector<string> vec = ct::splitStringToVector(vUrl, "/");
	std::string fileNameExt = *(vec.end() - 1);
	const std::string filePathNameExt = FileHelper::Instance()->GetAbsolutePathForFileLocation(fileNameExt, (int)vAssetType);

	if (vErrors)
	{
		(*vErrors).clear();
	}

	//if (wxFile::Exists(filePathNameExt) == false)
	{
		CURL *conn = nullptr;
		CURLcode code;
	//	std::string title;

		FILE *fp = nullptr;

		// Ensure one argument is given
		curl_global_init(CURL_GLOBAL_DEFAULT);

		const size_t BufSize = 5000;
		char url[BufSize];
		const size_t size = ct::mini(BufSize, vUrl.length());
		strncpy(url, vUrl.c_str(), size);
		url[size] = '\0';

		const std::string errors = Curl_Init(conn, url, vTimeOutInSecond);
		if (errors.empty())
		{
			// on ouvre le canal vers un fichier
			fp = fopen(filePathNameExt.c_str(), "wb");
			code = curl_easy_setopt(conn, CURLOPT_WRITEDATA, fp);
			if (code != CURLE_OK)
			{
				snprintf(errorBuffer, 256, "CURLOPT_WRITEDATA fail [%s]\n", puErrorBuffer);
				LogVarError(errorBuffer);
				if (vErrors)
				{
					*vErrors = std::string(errorBuffer);
				}
			}
			if (code == CURLE_OK)
			{
				code = curl_easy_perform(conn);
				if (code != CURLE_OK)
				{
					snprintf(errorBuffer, 256, "curl_easy_perform fail [%s]\n", puErrorBuffer);
					LogVarError(errorBuffer);
					if (vErrors)
					{
						*vErrors = std::string(errorBuffer);
					}
				}
				if (code == CURLE_OK)
				{
					fclose(fp);

					//ReSavePctureFile(filePathNameExt);
				}
				else
				{
					if (code == CURLcode::CURLE_UNSUPPORTED_PROTOCOL)
					{
						code = curl_easy_setopt(conn, CURLOPT_ERRORBUFFER, puErrorBuffer);
						snprintf(errorBuffer, 256, "curl_easy_setopt fail [%s]\n", puErrorBuffer);
						LogVarError(errorBuffer);
						if (vErrors)
						{
							*vErrors = std::string(errorBuffer);
						}
					}
				}
			}

			curl_easy_cleanup(conn);
		}
	}

	return fileNameExt;
}

void ShaderUrlLoader::ReSavePctureFile(std::string vPictureFile)
{
	if (vPictureFile.find(".png") != std::string::npos)
	{
		int w = 0;
		int h = 0;
		int comp = 0;

		unsigned char* image = stbi_load(vPictureFile.c_str(), &w, &h, &comp, STBI_rgb);

		if (image != nullptr && (comp == 4 || comp == 2))
		{
			stbi_image_free(image);
			image = stbi_load(vPictureFile.c_str(), &w, &h, &comp, STBI_rgb_alpha);
		}

		if (image != nullptr)
		{
			/*int resWrite = */stbi_write_png(vPictureFile.c_str(),
				w,
				h,
				comp,
				image,
				w * comp);

			stbi_image_free(image);
		}
	}
}

std::string ShaderUrlLoader::Curl_Init(CURL *&conn, char *url, int vTimeOutInSecond)
{
	char errorBuffer[256];

	CURLcode code;

	// Initialize CURL connection
	conn = curl_easy_init();

	if (conn == nullptr)
	{
		LogVarError("Failed to create CURL connection\n");
		exit(EXIT_FAILURE);
	}

	code = curl_easy_setopt(conn, CURLOPT_ERRORBUFFER, puErrorBuffer);
	if (code != CURLE_OK) {
		snprintf(errorBuffer, 256, "Failed to set error buffer [%d]\n", (int)code);
		LogVarError(errorBuffer);
		return std::string(errorBuffer);
	}

	code = curl_easy_setopt(conn, CURLOPT_URL, url);
	if (code != CURLE_OK) {
		snprintf(errorBuffer, 256, "Failed to set URL [%s]\n", puErrorBuffer);
		LogVarError(errorBuffer);
		return std::string(errorBuffer);
	}

	if (vTimeOutInSecond > 0)
	{
		code = curl_easy_setopt(conn, CURLOPT_TIMEOUT, vTimeOutInSecond);
		if (code != CURLE_OK) {
			snprintf(errorBuffer, 256, "Failed to set time out [%s]\n", puErrorBuffer);
			LogVarError(errorBuffer);
			return std::string(errorBuffer);
		}
	}
	
	code = curl_easy_setopt(conn, CURLOPT_FOLLOWLOCATION, 1L);
	if (code != CURLE_OK) {
		snprintf(errorBuffer, 256, "Failed to set redirect option [%s]\n", puErrorBuffer);
		LogVarError(errorBuffer);
		return std::string(errorBuffer);
	}

	// on veut juste lire donc pas grave si pas securis�
	code = curl_easy_setopt(conn, CURLOPT_SSL_VERIFYPEER, false);
	if (code != CURLE_OK) {
		snprintf(errorBuffer, 256, "CURLOPT_SSL_VERIFYPEER fail [%s]\n", puErrorBuffer);
		LogVarError(errorBuffer);
		return std::string(errorBuffer);
	}
	code = curl_easy_setopt(conn, CURLOPT_SSL_VERIFYHOST, 0);
	if (code != CURLE_OK) {
		snprintf(errorBuffer, 256, "CURLOPT_SSL_VERIFYHOST fail [%s]\n", puErrorBuffer);
		LogVarError(errorBuffer);
		return std::string(errorBuffer);
	}

	if (puUseProxy)
	{
		const size_t BufSize = 5000;
		size_t size = 0;

		char proxypath[BufSize];
		size = ct::mini<size_t>(BufSize, puProxyPath.length());
		strncpy(proxypath, puProxyPath.c_str(), size);
		proxypath[size] = '\0';

		code = curl_easy_setopt(conn, CURLOPT_PROXY, proxypath);
		if (code != CURLE_OK) {
			snprintf(errorBuffer, 256, "CURLOPT_PROXY fail [%s]\n", puErrorBuffer);
			LogVarError(errorBuffer);
			return std::string(errorBuffer);
		}

		char proxypwd[BufSize];
		size = ct::mini<size_t>(BufSize, puUserPwd.length());
		strncpy(proxypwd, puUserPwd.c_str(), size);
		proxypwd[size] = '\0';

		code = curl_easy_setopt(conn, CURLOPT_PROXYUSERPWD, proxypwd);
		if (code != CURLE_OK) {
			snprintf(errorBuffer, 256, "CURLOPT_PROXYUSERPWD fail [%s]\n", puErrorBuffer);
			LogVarError(errorBuffer);
			return std::string(errorBuffer);
		}
	}

	return "";
}

// https://github.com/curl/curl/blob/master/docs/examples/getinmemory.c
std::string ShaderUrlLoader::GetFileContentFromUrl(
	std::string vUrl,
	std::string vProxyPath,
	std::string vProxyUserPwd,
	int vTimeOutInSecond,
	std::atomic<UrlLoadingStatus>& vUrlLoadingStatus,
	std::atomic< float >& /*vProgress*/,
	std::atomic< bool >& /*vWorking*/,
	std::atomic< float >& /*vGenerationTime*/,
	std::string *vErrors)
{
	char errorBuffer[256];

	//std::string sample_url = "https://www.shadertoy.com/media/a/793a105653fbdadabdc1325ca08675e1ce48ae5f12e37973829c87bea4be3232.png";
	//vUrl = sample_url;

	// parse url for get the filename
	//std::vector<string> vec = ct::splitStringToVector(vUrl, "/");
	//std::string fileNameExt = *(vec.end() - 1);

	if (vErrors)
	{
		(*vErrors).clear();
	}

	//if (wxFile::Exists(filePathNameExt) == false)
	{
		CURL *conn = nullptr;
		CURLcode code;
	//	std::string title;

		FILE *fp = nullptr;

		// Ensure one argument is given
		curl_global_init(CURL_GLOBAL_DEFAULT);

		const size_t BufSize = 5000;
		char url[BufSize];
		const size_t size = ct::mini(BufSize, vUrl.length());
		strncpy(url, vUrl.c_str(), size);
		url[size] = '\0';

		puProxyPath = vProxyPath;
		puUserPwd = vProxyUserPwd;

		const std::string errors = Curl_Init(conn, url, vTimeOutInSecond);
		if (errors.empty())
		{
			// on ouvre le canal vers un fichier
			std::remove("VERSION");
			fp = fopen("VERSION", "wb");
			code = curl_easy_setopt(conn, CURLOPT_WRITEDATA, fp);
			if (code != CURLE_OK)
			{
				snprintf(errorBuffer, 256, "CURLOPT_WRITEDATA fail [%s]\n", puErrorBuffer);
				LogVarError(errorBuffer);
				if (vErrors)
				{
					*vErrors = std::string(errorBuffer);
				}
				vUrlLoadingStatus = UrlLoadingStatus::URL_LOADING_STATUE_ERROR;
			}
			if (code == CURLE_OK)
			{
				code = curl_easy_perform(conn);
				if (code != CURLE_OK)
				{
					snprintf(errorBuffer, 256, "curl_easy_perform fail [%s]\n", puErrorBuffer);
					LogVarError(errorBuffer);
					if (vErrors)
					{
						*vErrors = std::string(errorBuffer);
					}
					vUrlLoadingStatus = UrlLoadingStatus::URL_LOADING_STATUE_ERROR;
				}
				if (code == CURLE_OK)
				{
					fclose(fp);
					vUrlLoadingStatus = UrlLoadingStatus::URL_LOADING_STATUE_OK;

					//ReSavePctureFile(filePathNameExt);
				}
				else
				{
					if (code == CURLcode::CURLE_UNSUPPORTED_PROTOCOL)
					{
						code = curl_easy_setopt(conn, CURLOPT_ERRORBUFFER, puErrorBuffer);
						snprintf(errorBuffer, 256, "curl_easy_setopt fail [%s]\n", puErrorBuffer);
						LogVarError(errorBuffer);
						if (vErrors)
						{
							*vErrors = std::string(errorBuffer);
						}
						vUrlLoadingStatus = UrlLoadingStatus::URL_LOADING_STATUE_ERROR;
					}
				}
			}

			curl_easy_cleanup(conn);
		}
	}

	std::string ver = FileHelper::Instance()->LoadFileToString("VERSION", true);

	//std::remove("last_version.ver");

	return ver;
}

std::string ShaderUrlLoader::GetRealCodeUrl(std::string vBaseUrl, ShaderPlaform *vSPF, std::string *vId, std::string vApiKey, std::string *vError)
{
	std::string realUrl;

	// glslSandbox
	// base url = http://glslsandbox.com/e#38000.1
	// real code url = http://glslsandbox.com/item/38000.1
	// thumb url = http://glslsandbox.com/thumbs/38000.png

	// ShaderToy
	// il faut passer par l'api
	// on n'a aucun moyen de recuperer les codes autrement
	// base url : https://www.shadertoy.com/view/MtyXRK
	// get a shader from the shader id
	// https://www.shadertoy.com/api/v1/shaders/shaderID?key=appkey

	realUrl = ShaderToyFormat::ShadertoyStruct::GetGoodUrl(vBaseUrl, vApiKey, vId);
	if (!realUrl.empty()) // si pas vide alors c'est bien du Shadertoy
	{
		*vSPF = ShaderPlaform::SPf_SHADERTOY;
		if (vApiKey == "")
		{
			LogVarError("You must define an api key for all import from Shadertoy (in settings) !");

			if (vError)
			{
				*vError = "\nYou must define an api key\n for all import from Shadertoy\n (in settings) !";
			}

			return "";
		}
		return realUrl;
	}

	realUrl = GlslSandboxFormat::GlslSandboxStruct::GetGoodUrl(vBaseUrl, vId);
	if (!realUrl.empty()) // si pas vide alors c'est bien du GlslSandbox
	{
		*vSPF = ShaderPlaform::SPF_GLSLSANDBOX;
		return realUrl;
	}

	realUrl = VertexShaderArtFormat::VertexShaderArtStruct::GetGoodUrl(vBaseUrl, vId);
	if (!realUrl.empty()) // si pas vide alors c'est bien du VertexShaderArt
	{
		*vSPF = ShaderPlaform::SPF_VERTEXSHADERART;
		return realUrl;
	}

	return realUrl;
}

#define SHADERTOY_COMMON_ID -1258
#define SHADERTOY_IMAGE_ID -1879

std::list<ShaderInfos> ShaderUrlLoader::ParseBuffer(
	std::string vBuffer, 
	ShaderPlaform vSPF, 
	std::string vId,
	std::atomic<float>& vProgress,
	std::atomic<bool>& vWorking,
	std::atomic<float>& /*vGenerationTime*/,
	std::atomic<bool>& vImportInOneFile)
{
	vProgress = 0.0f;

	std::list<ShaderInfos> shaders;

#ifdef _DEBUG
	// on sauve le fichier pour la mise au point
	ofstream docFile("debug/lastImportBuffer.txt", ios::out);
	if (docFile.bad() == false)
	{
		docFile << vBuffer;
		docFile.close();
	}
#endif

	if (vWorking)
	{
		if (vSPF == ShaderPlaform::SPF_GLSLSANDBOX)
		{
			ShaderInfos shaderInfos;
			shaderInfos.platform = vSPF;

			GlslSandboxFormat::GlslSandboxStruct glStruct =
				GlslSandboxFormat::GlslSandboxStruct::ParseBuffer(vBuffer);

			shaderInfos.id = vId;

			shaderInfos.shader_id = "GlslSandbox_" + shaderInfos.id;

			////////////////////////
			shaderInfos.framebuffer += "\n@FRAMEBUFFER\n\n";
			shaderInfos.vertex += "\n@VERTEX\n\n";
			shaderInfos.specific_uniforms += "\n@UNIFORMS\n\n";
			shaderInfos.fragment += "\n@FRAGMENT\n\n";
			shaderInfos.note += "\n@NOTE\n\n";

			////////////////////////
			shaderInfos.name = glStruct.id;
			shaderInfos.user = glStruct.user;
			shaderInfos.url = "http://glslsandbox.com/" + glStruct.id;

			shaderInfos.note += "[[NAME]]:" + shaderInfos.name + "\n";
			shaderInfos.note += "[[URL]]:" + shaderInfos.url + "\n";
			if (glStruct.parent != "null")
			{
				shaderInfos.note += "[[PARENT_URL]]:http://glslsandbox.com/" + glStruct.parent + "\n";
			}
			shaderInfos.note += "[[USER]]:" + shaderInfos.user + "\n";

			vProgress = 60.0f;

			////////////////////////
			std::string code = glStruct.code;

			AddUniformsFromCode(&shaderInfos, &code, vSPF, vImportInOneFile);

			vProgress = 80.0f;

			////////////////////////

			//shaderInfos.vertex += "#extension GL_ARB_explicit_attrib_location : enable\n";
			shaderInfos.vertex += "layout(location = 0) in vec2 a_position; // Current vertex position\n";
			shaderInfos.vertex += "\n";

			shaderInfos.vertex += "\n";
			shaderInfos.vertex += "void main()\n";
			shaderInfos.vertex += "{\n";

			if (shaderInfos.varying.find("surfacePosition") != shaderInfos.varying.end()) // trouv�
			{
				shaderInfos.vertex += "\tsurfacePosition = a_position;\n";
			}

			shaderInfos.vertex += "\tgl_Position = vec4(a_position, 0.0, 1.0);\n";
			shaderInfos.vertex += "}\n";
			shaderInfos.vertex += "\n";

			////////////////////////

			ct::replaceString(code, "gl_FragColor", "fragColor");
			ct::replaceString(code, "texture2D(", "texture(");
			ct::replaceString(code, "textureCube(", "texture(");

			//shaderInfos.fragment += "#extension GL_ARB_explicit_attrib_location : enable\n";
			shaderInfos.fragment += "layout(location = 0) out vec4 fragColor;\n";

			shaderInfos.fragment += code;

			shaders.emplace_back(shaderInfos);

			vProgress = 100.0f;
		}
		else if (vSPF == ShaderPlaform::SPF_VERTEXSHADERART)
		{
			ShaderInfos shaderInfos;
			shaderInfos.platform = vSPF;

			VertexShaderArtFormat::VertexShaderArtStruct vsaStruct =
				VertexShaderArtFormat::VertexShaderArtStruct::ParseBuffer(vBuffer);

			shaderInfos.color = vsaStruct.backColor;

			shaderInfos.id = vId;

			shaderInfos.shader_id = "VertexShaderArt_" + shaderInfos.id;

			////////////////////////
			shaderInfos.framebuffer += "\n@FRAMEBUFFER\n\n";
			shaderInfos.vertex += "\n@VERTEX POINTS(" + vsaStruct.num + ") DISPLAY(" + vsaStruct.mode + ")\n\n";
			shaderInfos.specific_uniforms += "\n@UNIFORMS\n\n";
			shaderInfos.fragment += "\n@FRAGMENT\n\n";
			shaderInfos.note += "\n@NOTE\n\n";

			////////////////////////

			shaderInfos.name = vsaStruct.name;
			shaderInfos.user = vsaStruct.user;
			shaderInfos.url = "https://www.vertexshaderart.com/art/" + vsaStruct.id;

			shaderInfos.note += "[[NAME]]:" + shaderInfos.name + "\n";
			shaderInfos.note += "[[DATE]]:" + vsaStruct.creationDate + "\n";
			shaderInfos.note += "[[URL]]:" + shaderInfos.url + "\n";
			if (!vsaStruct.revisionId.empty())
			{
				shaderInfos.note += "[[PARENT_URL]]:https://www.vertexshaderart.com/art/" + vsaStruct.revisionId + "\n";
			}
			shaderInfos.note += "[[USER]]:" + shaderInfos.user + ":https://www.vertexshaderart.com/user/" + shaderInfos.user + "\n";
			shaderInfos.note += "[[SOUND_URL]]:" + vsaStruct.sound + "\n";
			shaderInfos.note += "Infos :\n\n not available in the exported json for the moment :(\n";

			vProgress = 60.0f;

			////////////////////////
			std::string code = vsaStruct.shader;

			AddUniformsFromCode(&shaderInfos, &code, vSPF, vImportInOneFile);

			vProgress = 80.0f;

			////////////////////////

			ct::replaceString(code, "texture2D(", "texture(");
			ct::replaceString(code, "textureCube(", "texture(");

			//shaderInfos.vertex += "#extension GL_ARB_explicit_attrib_location : enable\n";
			shaderInfos.vertex += "layout(location = 0) in float vertexId;\n";
			shaderInfos.vertex += "out vec4 v_color;\n";

			shaderInfos.vertex += code;

			////////////////////////

			//shaderInfos.fragment += "#extension GL_ARB_explicit_attrib_location : enable\n";
			shaderInfos.fragment += "layout(location = 0) out vec4 fragColor;\n";
			shaderInfos.fragment += "in vec4 v_color;\n";
			shaderInfos.fragment += "void main()\n";
			shaderInfos.fragment += "{\n";
			shaderInfos.fragment += "\tfragColor = v_color;\n";
			shaderInfos.fragment += "}\n";

			shaders.emplace_back(shaderInfos);

			vProgress = 100.0f;
		}
		else if (vSPF == ShaderPlaform::SPf_SHADERTOY)
		{
			ShaderToyFormat::ShadertoyStruct shStruct =
				ShaderToyFormat::ShadertoyStruct::ParseBuffer(vBuffer);

			if (!shStruct.error.empty())
			{
				shaders.emplace_front(ShaderInfos());
				ShaderInfos *error = &(*shaders.begin());
				error->error = shStruct.error + "\n";
				error->error += "If the message say : 'Invalid Key',\n its because,\n ShaderToy Need an Api Key for do import working.\n Go in https://www.shadertoy.com/myapps for obtaining a key\n and set the ShaderToy Api key in Settings";
			}
			else
			{
				vProgress = 10.0f;

				std::list<std::string> puIncludeFileNames;

				if (vWorking)
				{
					float countPass = (float)shStruct.renderpass.size();
					float idxPass = 0.0f;

					/////////////////////////////////////////////////////////////////////////
					////// 1ere passe : on va definir les ids de tout les buffers  //////////
					/////////////////////////////////////////////////////////////////////////
					for (std::list<ShaderToyFormat::ShaderToyRenderPass>::iterator it_rp = shStruct.renderpass.begin(); it_rp != shStruct.renderpass.end(); ++it_rp)
					{
						if (!vWorking) break;
						
						vProgress = 50.0f + 20.0f * (idxPass++) / countPass;

						//std::string key = it_rp->first;
						ShaderToyFormat::ShaderToyRenderPass pass = *it_rp;

						// emplace_front, pour que le 1er infos soit le image
						shaders.emplace_front(ShaderInfos());
						ShaderInfos *bufferPass = &(*shaders.begin());

						bufferPass->platform = vSPF;

						bufferPass->shader_id = "Shadertoy_" + shStruct.info.id;

						ct::replaceString(shStruct.info.name, " ", "_");
						ct::replaceString(shStruct.info.name, ":", "_");
						ct::replaceString(shStruct.info.name, "__", "_");
						ct::replaceString(shStruct.info.name, "/", "_");
						ct::replaceString(shStruct.info.name, "(", "_");
						ct::replaceString(shStruct.info.name, ")", "_");

						ct::replaceString(pass.name, " ", "_");
						ct::replaceString(pass.name, ":", "_");
						ct::replaceString(pass.name, "__", "_");
						ct::replaceString(pass.name, "/", "_");
						ct::replaceString(pass.name, "(", "_");
						ct::replaceString(pass.name, ")", "_");

						// buffer name
						std::string nodeName = shStruct.info.name + "_" + pass.name;
						if (vImportInOneFile && countPass > 1.0f)
						{
							if (pass.type == "image")
								nodeName = "MAIN";
							else
								nodeName = ct::toUpper(pass.name);
						}

						bufferPass->id = nodeName;

						if (vWorking)
						{
							// les outputs
							for (std::list<ShaderToyFormat::ShaderToyOutput>::iterator it_in = pass.outputs.begin(); it_in != pass.outputs.end(); ++it_in)
							{
								if (!vWorking) break;
								
								ShaderToyFormat::ShaderToyOutput output = *it_in;

								int id = ct::fvariant(output.id).GetI();

								if (pass.type == "buffer")
								{
									puBuffersName[id] = nodeName;
									puBuffersId[id] = pass.name;
									puBuffersParams[id].clear();
								}
								else if (pass.type == "cubemap")
								{
									puBuffersName[id] = nodeName;
									puBufferCubeMapsId[id] = pass.name;
									puBuffersParams[id].clear();
								}
							}

							// un shaderInfos par passe
							if (pass.type == "image")
							{
								bufferPass->name = shStruct.info.name;
								bufferPass->description = shStruct.info.description;
								bufferPass->url = "https://www.shadertoy.com/view/" + shStruct.info.id;
								bufferPass->user = shStruct.info.username;
								bufferPass->published = (shStruct.info.published == "3");

								bufferPass->note += "\n@NOTE\n\n";
								bufferPass->note += "[[NAME]]:" + bufferPass->name + "\n";
								bufferPass->note += "[[DATE]]:" + shStruct.info.date + "\n";
								bufferPass->note += "[[URL]]:" + bufferPass->url + "\n";
								bufferPass->note += "[[USER]]:" + bufferPass->user + ":https://www.shadertoy.com/user/" + bufferPass->user + "\n";
								bufferPass->note += "Infos :\n\n" + bufferPass->description + "\n";

								puBuffersInfos[SHADERTOY_IMAGE_ID] = bufferPass;
							}
							else if (pass.type == "buffer")
							{
								if (!pass.outputs.empty())
								{
									int bufferId = ct::fvariant(pass.outputs.begin()->id).GetI();
									puBuffersInfos[bufferId] = bufferPass;
								}
							}
							else if (pass.type == "cubemap")
							{
								if (!pass.outputs.empty())
								{
									int bufferId = ct::fvariant(pass.outputs.begin()->id).GetI();
									puBuffersInfos[bufferId] = bufferPass;
								}
							}

							if (pass.type == "common")
							{
								puBuffersName[SHADERTOY_COMMON_ID] = nodeName;
								puBuffersId[SHADERTOY_COMMON_ID] = pass.name;
								puBuffersInfos[SHADERTOY_COMMON_ID] = bufferPass;
								puIncludeFileNames.emplace_back(nodeName + ".glsl");
								//bufferPass->uniforms += "\n@UNIFORMS\n\n";
								bufferPass->common += "\n@COMMON\n\n";
							}
							else
							{
								////////////////////////

								if (vImportInOneFile && countPass > 1.0f)
								{
									bufferPass->framebuffer += "\n@FRAMEBUFFER BUFFER(" + nodeName + ")";
									bufferPass->fragment += "\n@FRAGMENT BUFFER(" + nodeName + ")\n\n";
									bufferPass->common_uniforms += "\n@UNIFORMS\n\n";
									bufferPass->specific_uniforms += "\n@UNIFORMS BUFFER(" + nodeName + ")\n\n";
								}
								else
								{
									bufferPass->framebuffer += "\n@FRAMEBUFFER";
									bufferPass->fragment += "\n@FRAGMENT\n\n";
									bufferPass->common_uniforms += "\n@UNIFORMS\n\n";
								}

								bufferPass->vertex += "\n@VERTEX\n\n";
								
								////////////////////////

								//bufferPass->vertex += "#extension GL_ARB_explicit_attrib_location : enable\n";
								bufferPass->vertex += "layout(location = 0) in vec2 a_position; // Current vertex position\n";
								bufferPass->vertex += "\n";

								bufferPass->vertex += "\n";
								bufferPass->vertex += "void main()\n";
								bufferPass->vertex += "{\n";
								bufferPass->vertex += "\tgl_Position = vec4(a_position, 0.0, 1.0);\n";
								bufferPass->vertex += "}\n";
								bufferPass->vertex += "\n";

								if (pass.type == "image")
								{
									bufferPass->IsMain = true;
								}
								else
								{
									bufferPass->IsBuffer = true;
								}
							}
						}
					}

					idxPass = 0.0f;

					if (vWorking)
					{
						countPass = 0.0f;
						for (std::list<ShaderToyFormat::ShaderToyRenderPass>::iterator it_rp = shStruct.renderpass.begin(); it_rp != shStruct.renderpass.end(); ++it_rp)
						{
							ShaderToyFormat::ShaderToyRenderPass pass = *it_rp;
							for (std::list<ShaderToyFormat::ShaderToyInput>::iterator it_in = pass.inputs.begin(); it_in != pass.inputs.end(); ++it_in)
							{
								countPass++;
							}
						}

						/////////////////////////////////////////////////////////////////////////
						////// 2eme passe : on recup le common  /////////////////////////////////
						/////////////////////////////////////////////////////////////////////////
						for (std::list<ShaderToyFormat::ShaderToyRenderPass>::iterator it_rp = shStruct.renderpass.begin(); it_rp != shStruct.renderpass.end(); ++it_rp)
						{
							if (!vWorking) break;

							ShaderToyFormat::ShaderToyRenderPass pass = *it_rp;

							if (vWorking)
							{
								ShaderInfos *commonBuffer = nullptr;

								if (pass.type == "common")
								{
									commonBuffer = puBuffersInfos[SHADERTOY_COMMON_ID];

									if (commonBuffer != nullptr)
									{
										commonBuffer->common += pass.code;
									}
								}
							}
						}

						/////////////////////////////////////////////////////////////////////////
						////// 3eme passe : on fini l'association  //////////////////////////////
						/////////////////////////////////////////////////////////////////////////
						idxPass = 0.0f;
						for (std::list<ShaderToyFormat::ShaderToyRenderPass>::iterator it_rp = shStruct.renderpass.begin(); it_rp != shStruct.renderpass.end(); ++it_rp)
						{
							if (!vWorking) break;
							
							//std::string key = it_rp->first;
							ShaderToyFormat::ShaderToyRenderPass pass = *it_rp;

							puChannelLinks.clear();

							if (vWorking)
							{
								//float countInputs = (float)pass.inputs.size();
								//float idxInput = 0.0f;
								//float stepProgress = 30.0f / countPass;

								// les inputs
								for (std::list<ShaderToyFormat::ShaderToyInput>::iterator it_in = pass.inputs.begin(); it_in != pass.inputs.end(); ++it_in)
								{
									if (!vWorking) break;

									vProgress = 70.0f + 30.0f * (idxPass++) / countPass;

									ShaderToyFormat::ShaderToyInput input = *it_in;

									int chan = ct::fvariant(input.chan).GetI();
									int id = ct::fvariant(input.id).GetI();

									if (input.type == "texture")
									{
										// on va donwload les eventuelles texture utilis�es par le shader
										std::string picture = GetFileFromUrl(input.filepath, 0, FILE_LOCATION_Enum::FILE_LOCATION_ASSET_TEXTURE_2D, nullptr);

										//<UNIFORM TYPE="[UTEX2D]" EXPANDED="1" PICTURE="..\assets\tex11.png" 
										// MAG="LINEAR" MIN="LINEAR_MIPMAP_LINEAR" 
										// WRAPS="REPEAT" WRAPT="REPEAT" 
										// MMBASELVL="0" MMMAXLVL="1000">uTex2D</UNIFORM>

										//"sampler": {
										//"filter": "linear",
										//	"wrap" : "clamp",
										//	"vflip" : "true",
										//	"srgb" : "false",
										//	"internal" : "byte"

										std::unordered_map<std::string, std::string> params;
										params["PICTURE"] = picture;
										params["MAG"] = input.sampler.filter;
										params["MIN"] = input.sampler.filter;
										params["WRAPS"] = input.sampler.wrap;
										params["WRAPT"] = input.sampler.wrap;
										params["MMBASELVL"] = "0";
										params["MMMAXLVL"] = "1000";
										params["FLIPY"] = input.sampler.vflip;

										puTexturesId[id] = params;
										puChannelLinks[chan] = id;
									}
									else if (input.type == "cubemap")
									{
										//<UNIFORM TYPE="UCUBEMAP" 
										//FILE1="\\f-horus\home12$\p038564\MyDocs\XShade\shaders\cube00_0.jpg" 
										//FILE2="\\f-horus\home12$\p038564\MyDocs\XShade\shaders\cube00_1.jpg" 
										//FILE3="\\f-horus\home12$\p038564\MyDocs\XShade\shaders\cube00_2.jpg" 
										//FILE4="\\f-horus\home12$\p038564\MyDocs\XShade\shaders\cube00_3.jpg" 
										//FILE5="\\f-horus\home12$\p038564\MyDocs\XShade\shaders\cube00_4.jpg" 
										//FILE6="\\f-horus\home12$\p038564\MyDocs\XShade\shaders\cube00_5.jpg">uCubeMap</UNIFORM>

										//"id": 27,
										//	"filepath" : "\/media\/a\/0681c014f6c88c356cf9c0394ffe015acc94ec1474924855f45d22c3e70b5785.png",
										//	"type" : "cubemap",
										//	"channel" : 0,
										//	"sampler" : {
										//	"filter": "linear",
										//		"wrap" : "clamp",
										//		"vflip" : "false",
										//		"srgb" : "false",
										//		"internal" : "byte"
										//},
										//	"published": 1

										std::unordered_map<std::string, std::string> params;

										// on va donwload les eventuelles texture utilis�es par le shader
										std::string url_without_ext;
										std::string ext;
										size_t ext_pos = input.filepath.find(".png", 0);
										if (ext_pos != std::string::npos)
										{
											url_without_ext = input.filepath.substr(0, ext_pos);
											ext = ".png";
										}
										else // jpeg peu etre
										{
											ext_pos = input.filepath.find(".jpg", 0);
											if (ext_pos != std::string::npos)
											{
												url_without_ext = input.filepath.substr(0, ext_pos);
												ext = ".jpg";
											}
										}

										params["FILE1"] = GetFileFromUrl(input.filepath, 0, FILE_LOCATION_Enum::FILE_LOCATION_ASSET_CUBEMAP, nullptr);
										params["FILE2"] = GetFileFromUrl(url_without_ext + "_1" + ext, 0, FILE_LOCATION_Enum::FILE_LOCATION_ASSET_CUBEMAP, nullptr);
										params["FILE3"] = GetFileFromUrl(url_without_ext + "_2" + ext, 0, FILE_LOCATION_Enum::FILE_LOCATION_ASSET_CUBEMAP, nullptr);
										params["FILE4"] = GetFileFromUrl(url_without_ext + "_3" + ext, 0, FILE_LOCATION_Enum::FILE_LOCATION_ASSET_CUBEMAP, nullptr);
										params["FILE5"] = GetFileFromUrl(url_without_ext + "_4" + ext, 0, FILE_LOCATION_Enum::FILE_LOCATION_ASSET_CUBEMAP, nullptr);
										params["FILE6"] = GetFileFromUrl(url_without_ext + "_5" + ext, 0, FILE_LOCATION_Enum::FILE_LOCATION_ASSET_CUBEMAP, nullptr);

										puCubeMapsId[id] = params;
										puChannelLinks[chan] = id;
									}
									else if (input.type == "buffer")
									{
										puChannelLinks[chan] = id;

										std::unordered_map<std::string, std::string> params;
										params["MAG"] = input.sampler.filter;
										params["MIN"] = input.sampler.filter;
										params["WRAPS"] = input.sampler.wrap;
										params["WRAPT"] = input.sampler.wrap;
										params["MMBASELVL"] = "0";
										params["MMMAXLVL"] = "1000";
										
										puBuffersParams[id] = params;
									}
									else if (input.type == "keyboard")
									{

									}
									else if (input.type == "sound") // fature sound de shadertoy
									{

									}
									else if (input.type == "musicstream")
									{
										//"id": "XsfXDl",
										//"filepath": "https://soundcloud.com/royalty-free-music-soundotcom/ho-hey-100-free-download-royalty-free-music-dance-pop-upbeat-fashion",
										//"type" : "musicstream",
										//"channel" : 1,
										//"sampler" : {
										//"filter": "linear",
										//"wrap" : "clamp",
										//"vflip" : "true",
										//"srgb" : "false",
										//"internal" : "byte"
										//},

										std::unordered_map<std::string, std::string> params;
										params["FILE"] = "";
										params["HISTO"] = "true";

										puTexturesSoundId[id] = params;
										puChannelLinks[chan] = id;
									}
									else if (input.type == "music")
									{
										// on va donwload les fichiers sons
										std::string sound_file = GetFileFromUrl(input.filepath, 0, FILE_LOCATION_Enum::FILE_LOCATION_ASSET_SOUND, nullptr);

										/*
										"id": "4df3Rn",
									   "filepath": "/media/a/3c33c415862bb7964d256f4749408247da6596f2167dca2c86cc38f83c244aa6.mp3",
									   "type": "music",
									   "channel": 1,
									   "sampler": {
										"filter": "linear",
										"wrap": "clamp",
										"vflip": "false",
										"srgb": "false",
										"internal": "byte"
										*/

										std::unordered_map<std::string, std::string> params;
										params["FILE"] = sound_file;
										params["HISTO"] = "true";

										puTexturesSoundId[id] = params;
										puChannelLinks[chan] = id;
									}
									else if (input.type == "volume")
									{
										// on va donwload les eventuelles texture utilis�es par le shader
										std::string volume = GetFileFromUrl(input.filepath, 0, FILE_LOCATION_Enum::FILE_LOCATION_ASSET_TEXTURE_3D, nullptr);

										//<UNIFORM TYPE="[UTEX2D]" EXPANDED="1" PICTURE="..\assets\tex11.png" 
										// MAG="LINEAR" MIN="LINEAR_MIPMAP_LINEAR" 
										// WRAPS="REPEAT" WRAPT="REPEAT" 
										// MMBASELVL="0" MMMAXLVL="1000">uTex2D</UNIFORM>

										//"sampler": {
										//"filter": "linear",
										//	"wrap" : "clamp",
										//	"vflip" : "true",
										//	"srgb" : "false",
										//	"internal" : "byte"

										std::unordered_map<std::string, std::string> params;
										params["PICTURE"] = volume;
										params["MAG"] = input.sampler.filter;
										params["MIN"] = "LINEAR_MIPMAP_LINEAR";
										params["WRAPS"] = input.sampler.wrap;
										params["WRAPT"] = input.sampler.wrap;
										params["MMBASELVL"] = "0";
										params["MMMAXLVL"] = "1000";
										params["FORMAT"] = "shadertoy";

										puTextures3DId[id] = params;
										puChannelLinks[chan] = id;
									}
								}

								// un shaderInfos par passe
								ShaderInfos* bufferPass = nullptr;
								//ShaderInfos *commonBuffer = nullptr;

								if (pass.type == "image")
								{
									bufferPass = puBuffersInfos[SHADERTOY_IMAGE_ID];
								}
								else if (!pass.outputs.empty())
								{
									int bufferId = ct::fvariant(pass.outputs.begin()->id).GetI();

									bufferPass = puBuffersInfos[bufferId];
								}

								if (bufferPass != nullptr)
								{
									std::string code = pass.code;

									//FileHelper::Instance()->SaveToFile(code, "code_buffer.glsl", FILE_LOCATION_DEBUG);

									puCodeReplacements.clear();

									if (puBuffersInfos.find(SHADERTOY_COMMON_ID) != puBuffersInfos.end()) // trouv�
									{
										std::string commonCode = puBuffersInfos[SHADERTOY_COMMON_ID]->common;
										AddUniformsFromCode(bufferPass, &code, vSPF, vImportInOneFile, &commonCode);
									}
									else
									{
										AddUniformsFromCode(bufferPass, &code, vSPF, vImportInOneFile);
									}

									if (code.find("mainVR") != std::string::npos)
									{
										bufferPass->specific_uniforms += "uniform float(vr:use) useVR;\n";
									}

									//code = DoContextGLSLConverion(code);

									// on va remplacer les noms de certain uniforms dans le code
									std::unordered_map<std::string, std::string>::iterator it;
									for (it = puCodeReplacements.begin(); it != puCodeReplacements.end(); ++it)
									{
										ct::replaceString(code, it->first, it->second);
									}

									// si on fait pas ca ca merde avec mon shader de reaction diffusion
									// https://www.shadertoy.com/view/MlByzR
									// car du coup shadertoy fait : mainImage(out vec4
									// et sur pc c'est interprt� par une sortie uniquement donc
									// meme si gl_FragColor est initialis� avant l'appel mainImage
									// gl_FragColor est indefini et cree du bruit dans mainImage
									// en webgl2, le out doit etre traduit par angle en in out automatiquement
									// mais sur pc un out est un out
									ct::replaceString(code, "mainImage(out", "mainImage(inout");
									ct::replaceString(code, "mainImage( out", "mainImage(inout");
									ct::replaceString(code, "mainImage (out", "mainImage(inout");
									ct::replaceString(code, "mainImage ( out", "mainImage(inout");
									ct::replaceString(code, "gl_FragColor", "fragColor");
									ct::replaceString(code, "mainVR(out", "mainVR(inout");
									ct::replaceString(code, "mainVR( out", "mainVR( inout");

									//bufferPass->fragment += "#extension GL_ARB_explicit_attrib_location : enable\n";
									bufferPass->fragment += "layout(location = 0) out vec4 fragColor;\n";

									if (!(vImportInOneFile && countPass > 1.0f))
									{
										// on ajoute les fichiers inclus (common)
										if (!puIncludeFileNames.empty())
										{
											bufferPass->fragment += "\n";

											for (auto it_Inc = puIncludeFileNames.begin(); it_Inc != puIncludeFileNames.end(); ++it_Inc)
											{
												bufferPass->fragment += "#include \"" + (*it_Inc) + "\"\n";
											}

											bufferPass->fragment += "\n";
										}
									}

									bufferPass->fragment += code;
								}
							}
						}

						/////////////////////////////////////////////////////////////////////////
						////// 4eme passe : on fini les outputs  ////////////////////////////////
						/////////////////////////////////////////////////////////////////////////
						idxPass = 0.0f;
						for (std::list<ShaderToyFormat::ShaderToyRenderPass>::iterator it_rp = shStruct.renderpass.begin(); it_rp != shStruct.renderpass.end(); ++it_rp)
						{
							if (!vWorking) break;

							//std::string key = it_rp->first;
							ShaderToyFormat::ShaderToyRenderPass pass = *it_rp;

							if (vWorking)
							{
								// on defini les buffer output
								for (std::list<ShaderToyFormat::ShaderToyOutput>::iterator it_in = pass.outputs.begin(); it_in != pass.outputs.end(); ++it_in)
								{
									if (!vWorking) break;

									ShaderToyFormat::ShaderToyOutput output = *it_in;

									int id = ct::fvariant(output.id).GetI();

									if (pass.type == "buffer")
									{
										SetShaderBufferFormat(id);
									}
									else if (pass.type == "cubemap")
									{

									}
								}
							}
						}

						vProgress = 100.0f;
					}

					// on finit par definir le buffer image
					SetShaderBufferFormat(SHADERTOY_IMAGE_ID);
				}
			}
		}
	}

	return shaders;
}

// on va rechercher les uniforms depuis le code et les ajouter automatiquement
void ShaderUrlLoader::AddUniformsFromCode(ShaderInfos *vShaderInfos, std::string *vCode, ShaderPlaform vShaderPlaform, std::atomic<bool>& vImportInOneFile, std::string *vCommonCode)
{
	if (vShaderInfos)
	{
		if (vShaderPlaform == ShaderPlaform::SPF_VERTEXSHADERART)
		{
			if (vCode->find("resolution") != string::npos)
			{
				std::string baseName = "resolution";
				vShaderInfos->specific_uniforms += "uniform vec2(buffer:target=0) " + baseName + ";\n";
			}
			if (vCode->find("mouse") != string::npos)
			{
				std::string baseName = "mouse";
				vShaderInfos->specific_uniforms += "uniform vec2(mouse:normalized) " + baseName + ";\n";
			}
			if (vCode->find("background") != string::npos)
			{
				std::string baseName = "background";
				vShaderInfos->specific_uniforms += "uniform vec4(color) " + baseName + ";\n";
			}
			if (vCode->find("time") != string::npos)
			{
				std::string baseName = "time";
				vShaderInfos->specific_uniforms += "uniform float(time) " + baseName + ";\n";
			}
			if (vCode->find("touch") != string::npos)
			{
				//std::string baseName = "touch";
			}
			if (vCode->find("volume") != string::npos)
			{
				//std::string baseName = "volume";
			}
			if (vCode->find("sound") != string::npos)
			{
				std::string baseName = "sound";
				vShaderInfos->specific_uniforms += "uniform sampler2D(sound_histo) " + baseName + ";\n";
			}
			if (vCode->find("floatSound") != string::npos)
			{
				std::string baseName = "floatSound";
				vShaderInfos->specific_uniforms += "uniform sampler2D(sound_histo) " + baseName + ";\n";
			}
			if (vCode->find("vertexCount") != string::npos)
			{
				std::string baseName = "vertexCount";
				vShaderInfos->specific_uniforms += "uniform float(maxpoints) " + baseName + ";\n";
			}
		}
		else
		{
			size_t varying_pos = 0;
			while ((varying_pos = vCode->find("varying ", varying_pos)) != std::string::npos)
			{
				size_t coma_pos = vCode->find(';', varying_pos);
				size_t comeback_pos = vCode->find('\n', varying_pos);
				if (coma_pos != std::string::npos)
				{
					std::string var_line = vCode->substr(varying_pos, coma_pos - varying_pos);
					ct::replaceString(var_line, "\n", "");
					ct::replaceString(var_line, "\t", "");
					ct::replaceString(var_line, "\r", "");
					std::vector<string> vec = ct::splitStringToVector(var_line, " ");
					if (vec.size() == 3)
					{
	//					std::string var = vec[0];
	//					std::string type = vec[1];
						std::string name = vec[2];

						if (comeback_pos == coma_pos + 1)
						{
							vCode->erase(varying_pos, comeback_pos + 1 - varying_pos);
						}
						else
						{
							vCode->erase(varying_pos, coma_pos + 1 - varying_pos);
						}

						if (varying_pos > 0)
							varying_pos--;

						if (vShaderPlaform == ShaderPlaform::SPF_UNKNOW)
						{
							std::string line;

							line = var_line;
							ct::replaceString(line, "varying", "out");
							vShaderInfos->vertex += line + ";\n";

							line = var_line;
							ct::replaceString(line, "varying", "in");
							vShaderInfos->fragment += line + ";\n";

							vShaderInfos->varying[name] = true;
						}
						if (vShaderPlaform == ShaderPlaform::SPF_GLSLSANDBOX)
						{
							std::string line;

							line = var_line;
							ct::replaceString(line, "varying", "out");
							vShaderInfos->vertex += line + ";\n";

							line = var_line;
							ct::replaceString(line, "varying", "in");
							vShaderInfos->fragment += line + ";\n";

							vShaderInfos->varying[name] = true;
						}
						if (vShaderPlaform == ShaderPlaform::SPf_SHADERTOY)
						{

						}
					}
				}

				varying_pos += 1;
			}

			//size_t uniforms_start = vCode->find("@UNIFORMS_START");
			//size_t uniforpuelse = vCode->find("@UNIFORMS_ELSE");
			//size_t uniforpuend = vCode->find("@UNIFORMS_END");

			size_t uniforpupos = 0;
			while ((uniforpupos = vCode->find("uniform ", uniforpupos)) != std::string::npos)
			{
				if (!ShaderStageParsing::IfInCommentZone(*vCode, uniforpupos))
				{
					size_t coma_pos = vCode->find(';', uniforpupos);
					size_t comeback_pos = vCode->find('\n', uniforpupos);
					if (coma_pos != std::string::npos)
					{
						std::string uni_line = vCode->substr(uniforpupos, coma_pos - uniforpupos);
						ct::replaceString(uni_line, "\n", "");
						ct::replaceString(uni_line, "\t", "");
						ct::replaceString(uni_line, "\r", "");
						std::vector<string> vec = ct::splitStringToVector(uni_line, " ");
						if (vec.size() == 3)
						{
							std::string type = vec[1];
							std::string name = vec[2];

							size_t length_to_erase = 0;
							if (comeback_pos == coma_pos + 1)
							{
								length_to_erase = comeback_pos + 1 - uniforpupos;
							}
							else
							{
								length_to_erase = coma_pos + 1 - uniforpupos;
							}
							vCode->erase(uniforpupos, length_to_erase);

							if (uniforpupos > 0)
								uniforpupos--;

							if (vShaderPlaform == ShaderPlaform::SPF_UNKNOW)
							{
								vShaderInfos->specific_uniforms += uni_line + "\n";
							}
							if (vShaderPlaform == ShaderPlaform::SPF_GLSLSANDBOX)
							{
								std::string baseName = name;

								if (baseName == "resolution")
								{
									vShaderInfos->specific_uniforms += "uniform vec2(buffer) " + name + ";\n";
								}
								else if (baseName == "surfaceSize")
								{
									vShaderInfos->specific_uniforms += "uniform vec2(buffer) " + name + ";\n";
								}
								else if (baseName == "mouse")
								{
									vShaderInfos->specific_uniforms += "uniform vec2(mouse:normalized) " + name + ";\n";
								}
								else if (baseName == "time")
								{
									vShaderInfos->specific_uniforms += "uniform float(time) " + name + ";\n";
								}
								else if (type == "sampler2D")
								{
									vShaderInfos->specific_uniforms += "uniform sampler2D(buffer) " + name + ";\n";
								}
							}
							if (vShaderPlaform == ShaderPlaform::SPf_SHADERTOY)
							{
								if (name == "iGlobalTime")
									name = "iTime";

								std::string baseName = name;

								if (baseName == "iResolution")
								{
									vShaderInfos->common_uniforms += "uniform vec3(buffer) " + name + ";\n";
								}
								else if (baseName == "iMouse")
								{
									vShaderInfos->common_uniforms += "uniform vec4(mouse:shadertoy) " + name + ";\n";
								}
								else if (baseName == "iGlobalTime" || baseName == "iTime")
								{
									vShaderInfos->common_uniforms += "uniform float(time) " + name + ";\n";
								}
								//if (baseName == "iChannelTime[4]")
								//{
								//}
								else if (baseName == "iDate")
								{
									vShaderInfos->common_uniforms += "uniform vec4(date) " + name + ";\n";
								}
								//if (baseName == "iSampleRate")
								//{
								//}
								else if (baseName == "iFrame")
								{
									vShaderInfos->common_uniforms += "uniform int(frame) " + name + ";\n";
								}
								else if (baseName == "iTimeDelta")
								{
									vShaderInfos->common_uniforms += "uniform float(deltatime) " + name + ";\n";
								}
								//if (baseName == "iFrameRate")
								//{
								//}
								else if (baseName == "iChannel0")
								{
									std::vector<std::string::size_type> lst = ct::strContains(*vCode, baseName);
									if (lst.empty() && vCommonCode)
										lst = ct::strContains(*vCommonCode, baseName);
									DoChannelInput(baseName, vShaderInfos, 0, lst.size(), vImportInOneFile);
								}
								else if (baseName == "iChannel1")
								{
									std::vector<std::string::size_type> lst = ct::strContains(*vCode, baseName);
									if (lst.empty() && vCommonCode)
										lst = ct::strContains(*vCommonCode, baseName);
									DoChannelInput(baseName, vShaderInfos, 1, lst.size(), vImportInOneFile);
								}
								else if (baseName == "iChannel2")
								{
									std::vector<std::string::size_type> lst = ct::strContains(*vCode, baseName);
									if (lst.empty() && vCommonCode)
										lst = ct::strContains(*vCommonCode, baseName);
									DoChannelInput(baseName, vShaderInfos, 2, lst.size(), vImportInOneFile);
								}
								else if (baseName == "iChannel3")
								{
									std::vector<std::string::size_type> lst = ct::strContains(*vCode, baseName);
									if (lst.empty() && vCommonCode)
										lst = ct::strContains(*vCommonCode, baseName);
									DoChannelInput(baseName, vShaderInfos, 3, lst.size(), vImportInOneFile);
								}
								else if (baseName == "sampler2D")
								{
									vShaderInfos->specific_uniforms += "uniform sampler2D(pictures:choose) " + name + ";\n";
								}
								else if (baseName == "samplerCube")
								{
									vShaderInfos->specific_uniforms += "uniform samplerCube(choose) " + name + ";\n";
								}
							}
						}
					}
				}
				
				uniforpupos += 1;
			}

			size_t note_start = ShaderStageParsing::GetTagPos(*vCode, "@NOTE_START", 0, false, true);
			size_t note_end = ShaderStageParsing::GetTagPos(*vCode, "@NOTE_END", 0, false, true);

			if (note_start != std::string::npos)
			{
				if (note_end != std::string::npos)
				{
					if (note_start < note_end)
					{
						// on va mettre dans la section note ce qu'il y a entre @NOTE_START et @NOTE_END
						size_t loc = note_start + std::string("@NOTE_START").size();

						std::string note = vCode->substr(loc, note_end - loc);
						ct::replaceString(note, "://", ":\\");
						ct::replaceString(note, "//", "");
						ct::replaceString(note, "/*", "");
						ct::replaceString(note, "*/", "");
						ct::replaceString(note, ":\\", "://");

						vShaderInfos->note += note;

						note_end += std::string("@NOTE_END").size();
						vCode->erase(note_start, note_end - note_start);
					}
					else
					{
						LogVarError("Error : @NOTE_START must be before @NOTE_END");
					}
				}
				else
				{
					LogVarError("Error : the two's tags must be present : @NOTE_START and @NOTE_END");
				}
			}
			
			size_t uniforms_start = ShaderStageParsing::GetTagPos(*vCode, "@UNIFORMS_START", 0, false, true);
			size_t uniforpuelse = ShaderStageParsing::GetTagPos(*vCode, "@UNIFORMS_ELSE", 0, false, true);
			size_t uniforpuend = ShaderStageParsing::GetTagPos(*vCode, "@UNIFORMS_END", 0, false, true);

			if (uniforms_start != std::string::npos)
			{
				if (uniforpuelse != std::string::npos)
				{
					if (uniforpuend != std::string::npos)
					{
						if (uniforms_start < uniforpuelse && uniforpuelse < uniforpuend)
						{
							// on va mettre dans la section uniform ce qu'il y a entre @UNIFORMS_START et @UNIFORMS_ELSE
							// et virer ce qu'il y a entre @UNIFORMS_ELSE et @UNIFORMS_END
							size_t loc = uniforms_start + std::string("@UNIFORMS_START").size();
							std::string uniforms = vCode->substr(loc, uniforpuelse - loc);
							ct::replaceString(uniforms, "//", "");
							ct::replaceString(uniforms, "/*", "");
							ct::replaceString(uniforms, "*/", "");
							ct::replaceString(uniforms, ";", "; //");

							//FileHelper::Instance()->SaveToFile(uniforms, "uniforms.glsl", FILE_LOCATION_DEBUG);

							vShaderInfos->specific_uniforms += uniforms;

							//FileHelper::Instance()->SaveToFile(vShaderInfos->uniforms, "vShaderInfos_uniforms.glsl", FILE_LOCATION_DEBUG);

							uniforpuend += std::string("@UNIFORMS_END").size();
							vCode->erase(uniforms_start, uniforpuend - uniforms_start);

							//FileHelper::Instance()->SaveToFile(*vCode, "vCode.glsl", FILE_LOCATION_DEBUG);

						}
						else
						{
							LogVarError("Error : @UNIFORMS_START must be before @UNIFORMS_ELSE and @UNIFORMS_ELSE before @UNIFORMS_END");
						}
					}
					else
					{
						LogVarError("Error : the three tags must be present : @UNIFORMS_START, @UNIFORMS_ELSE and @UNIFORMS_END");
					}
				}
				else
				{
					LogVarError("Error : the three tags must be present : @UNIFORMS_START, @UNIFORMS_ELSE and @UNIFORMS_END");
				}
			}
		}
	}
}

std::vector<std::string> ShaderUrlLoader::ParseUniform(std::string /*vUniformStr*/)
{
	std::vector<std::string> arr;

	return arr;
}

void ShaderUrlLoader::SetShaderBufferFormat(int vBufferId)
{
	if (puBuffersInfos.find(vBufferId) != puBuffersInfos.end()) // buffer
	{
		auto infos = puBuffersInfos[vBufferId];
		if (infos)
		{
			if (vBufferId == SHADERTOY_IMAGE_ID) // image
			{
				infos->framebuffer += " WRAP(clamp) MIPMAP(false) FILTER(linear) COUNT(1) /*SIZE(512)*/";
			}
			else
			{
				if (puBuffersParams.find(vBufferId) != puBuffersParams.end()) // buffer
				{
					const auto& params = puBuffersParams[vBufferId];

					if (params.find("MAG") != params.end() && params.find("WRAPS") != params.end())
					{
						const auto& mipmap = (params.find("MAG") != params.end() && params.at("MAG") == "mipmap" ? "true" : "false");
						const auto& filter = (params.find("MAG") != params.end() && params.at("MAG") == "mipmap" ? "linear" : params.at("MAG"));
						const auto& wrap = params.at("WRAPS");

						infos->framebuffer += " WRAP(" + wrap + ") MIPMAP(" + mipmap + ") FILTER(" + filter + ") COUNT(1) /*SIZE(512)*/";
					}
					else
					{
						infos->framebuffer += " /*WRAP(false)*/ /*MIPMAP(false)*/ /*FILTER(linear)*/ /*COUNT(1)*/ /*SIZE(512)*/";
					}
				}
			}
		}
		else
		{
			LogVarError("%i n'est pas id de buffer valide; non trouv� !", vBufferId);
		}
	}
	else
	{
		LogVarError("ShaderInfos %i not found !", vBufferId);
	}
}

void ShaderUrlLoader::DoChannelInput(std::string vBaseName, ShaderInfos *vShaderInfos, int vChanId, size_t nOccurs, std::atomic<bool>& vImportInOneFile)
{
	// a ce stade les ligne d'uniformes concern�s ont �t� �ffac�s
	if (nOccurs > 0)
	{
		if (puChannelLinks.find(vChanId) != puChannelLinks.end()) // trouve
		{
			int id = puChannelLinks[vChanId];

			if (puBuffersId.find(id) != puBuffersId.end()) // buffer
			{
				auto bufferId = vBaseName;
				auto bufferName = puBuffersName[id];

				puCodeReplacements[vBaseName] = bufferId;
				puCodeReplacements["iChannelResolution[" + ct::toStr(vChanId) + "]"] = bufferId + "Size";

				if (bufferName == vShaderInfos->id)
				{
					vShaderInfos->specific_uniforms += "uniform sampler2D(buffer) " + bufferId + ";\n";
				}
				else
				{
					if (vImportInOneFile)
					{
						vShaderInfos->specific_uniforms += "uniform sampler2D(buffer:buffer=" + bufferName + ") " + bufferId + ";\n";
					}
					else
					{
						vShaderInfos->specific_uniforms += "uniform sampler2D(buffer:file=" + bufferName + ") " + bufferId + ";\n";
					}
				}
				vShaderInfos->specific_uniforms += "uniform vec2(buffer:target=" + bufferId + ") " + bufferId + "Size;\n";
			}
			else if (puTexturesId.find(id) != puTexturesId.end()) // texture
			{
				std::unordered_map<std::string, std::string> params = puTexturesId[id];

				puCodeReplacements["iChannelResolution[" + ct::toStr(vChanId) + "]"] = vBaseName + "Size";

				std::string pict = params["PICTURE"];
				std::string flip = params["FLIPY"];
				std::string mipmap = (params["MAG"] == "mipmap" ? "true" : "false");
				std::string filter = (params["MAG"] == "mipmap" ? "linear" : params["MAG"]);
				std::string wrap = params["WRAPS"];

				vShaderInfos->specific_uniforms += "uniform sampler2D(picture:choice=" + pict + ":flip=" + flip + ":mipmap=" + mipmap + ":wrap=" + wrap + ":filter=" + filter + ") " + vBaseName + ";\n";
				vShaderInfos->specific_uniforms += "uniform vec2(picture:target=" + vBaseName + ") " + vBaseName + "Size;\n";
			}
			else if (puBufferCubeMapsId.find(id) != puBufferCubeMapsId.end()) // buffer cubemap
			{
				std::string bufferId = puBufferCubeMapsId[id];
				std::string bufferName = puBuffersName[id];

				puCodeReplacements[vBaseName] = bufferId;
				puCodeReplacements["iChannelResolution[" + ct::toStr(vChanId) + "]"] = bufferId + "Size";

				vShaderInfos->specific_uniforms += "uniform samplerCube(cubemap:file=" + bufferName + ") " + vBaseName + ";\n";
			}
			else if (puCubeMapsId.find(id) != puCubeMapsId.end()) // cubemap
			{
				std::unordered_map<std::string, std::string> params = puCubeMapsId[id];

				vShaderInfos->specific_uniforms += "uniform samplerCube(cubemap:files=";
				vShaderInfos->specific_uniforms += params["FILE1"] + ",";
				vShaderInfos->specific_uniforms += params["FILE2"] + ",";
				vShaderInfos->specific_uniforms += params["FILE3"] + ",";
				vShaderInfos->specific_uniforms += params["FILE4"] + ",";
				vShaderInfos->specific_uniforms += params["FILE5"] + ",";
				vShaderInfos->specific_uniforms += params["FILE6"] + ") " + vBaseName + ";\n";
			}
			else if (puTextures3DId.find(id) != puTextures3DId.end()) // texture3D
			{
				std::unordered_map<std::string, std::string> params = puTextures3DId[id];

				puCodeReplacements["iChannelResolution[" + ct::toStr(vChanId) + "]"] = vBaseName + "Size";

				std::string pict = params["PICTURE"];
				std::string format = params["FORMAT"];
				std::string mipmap = (params["MAG"] == "mipmap" ? "true" : "false");
				std::string filter = (params["MAG"] == "mipmap" ? "linear" : params["MAG"]);
				std::string wrap = params["WRAPS"];

				vShaderInfos->specific_uniforms += "uniform sampler3D(volume:file=" + pict + ":format=" + format + ":mipmap=" + mipmap + ":wrap=" + wrap + ":filter=" + filter + ") " + vBaseName + ";\n";
				vShaderInfos->specific_uniforms += "uniform vec3(volume:target=" + vBaseName + ") " + vBaseName + "Size;\n";
			}
			else if (puTexturesSoundId.find(id) != puTexturesSoundId.end()) // textureSound
			{
				std::unordered_map<std::string, std::string> params = puTexturesSoundId[id];

				std::string file = params["FILE"];
				std::string histo = params["HISTO"];

				if (!file.empty())
				{
					if (histo == "true")
					{
						vShaderInfos->specific_uniforms += "uniform sampler2D(sound_hsito:file=" + file + ") " + vBaseName + ";\n";
					}
					else
					{
						vShaderInfos->specific_uniforms += "uniform sampler1D(sound:file=" + file + ") " + vBaseName + ";\n";
					}
				}
				else if (histo == "true")
				{
					vShaderInfos->specific_uniforms += "uniform sampler2D(sound_histo) " + vBaseName + ";\n";
				}
				else
				{
					vShaderInfos->specific_uniforms += "uniform sampler1D(sound) " + vBaseName + ";\n";
				}
			}
			else
			{
				LogVarError("Channel id %i not found", id);
			}
		}
		else // l'input n'est pas r�f�renc� on va ajoute un uniform de type texture vide
		{
			vShaderInfos->specific_uniforms += "uniform sampler2D(pictures:choose) " + vBaseName + ";\n";
		}
	}
}
#endif // #ifdef USE_NETWORK