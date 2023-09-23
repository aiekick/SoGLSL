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

#include "ImporterFromShadertoy.h"
#include <stdio.h> 
#include <ctools/Logger.h>
#include <Renderer/RenderPack.h>
#include <CodeTree/CodeTree.h>
#include <CodeTree/Parsing/ShaderStageParsing.h>
#include <stb/stb_image.h>
#include <stb/stb_image_write.h>

#include "Format_ShaderToy.h"
#include "Format_GlslSandbox.h"
#include "Format_VertexShaderArt.h"

#define SHADERTOY_COMMON_ID "COMMON_SHADERTOY_ID"
#define SHADERTOY_IMAGE_ID "IMAGE_SHADERTOY_ID"

std::list<ShaderInfos> ImporterFromShadertoy::ParseBuffer(
	const std::string& vBuffer,
	const std::string& vId,
	const bool& vImportInOneFile)
{
	std::list<ShaderInfos> shaders;

	picojson::value json;
	picojson::parse(json, vBuffer);

	ShaderToyFormat::ShadertoyStruct shStruct =
		ShaderToyFormat::ShadertoyStruct::ParseShader(json);

	if (!shStruct.error.empty())
	{
		shaders.emplace_back(ShaderInfos());
		auto error_ptr = &shaders.back();
		error_ptr->error = shStruct.error + "\n";
		error_ptr->error += "If the message say : 'Invalid Key',\n its because,\n ShaderToy Need an Api Key for do import working.\n Go in https://www.shadertoy.com/myapps for obtaining a key\n and set the ShaderToy Api key in Settings";
	}
	else
	{
		std::list<std::string> puIncludeFileNames;

		float countPass = (float)shStruct.renderpass.size();
		float idxPass = 0.0f;

		/////////////////////////////////////////////////////////////////////////
		////// 1ere passe : on va definir les ids de tout les buffers  //////////
		/////////////////////////////////////////////////////////////////////////
		for (std::list<ShaderToyFormat::ShaderToyRenderPass>::iterator it_rp = shStruct.renderpass.begin(); it_rp != shStruct.renderpass.end(); ++it_rp)
		{
			//std::string key = it_rp->first;
			ShaderToyFormat::ShaderToyRenderPass pass = *it_rp;

			// emplace_front, pour que le 1er infos soit le image
			shaders.emplace_back(ShaderInfos());
			auto bufferPass_ptr = &shaders.back();

			bufferPass_ptr->platform = ShaderPlaform::SPf_SHADERTOY;

			bufferPass_ptr->shader_id = "Shadertoy_" + shStruct.info.id;

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

			bufferPass_ptr->id = nodeName;

			// les outputs
			for (std::list<ShaderToyFormat::ShaderToyOutput>::iterator it_in = pass.outputs.begin(); it_in != pass.outputs.end(); ++it_in)
			{
				ShaderToyFormat::ShaderToyOutput output = *it_in;

				if (pass.type == "buffer")
				{
					puBuffersName[output.id] = nodeName;
					puBuffersId[output.id] = pass.name;
					puBuffersParams[output.id] = std::unordered_map<std::string, std::string>();
				}
				else if (pass.type == "cubemap")
				{
					puBuffersName[output.id] = nodeName;
					puBufferCubeMapsId[output.id] = pass.name;
					puBuffersParams[output.id] = std::unordered_map<std::string, std::string>();
				}
			}

			// un shaderInfos par passe
			if (pass.type == "image")
			{
				bufferPass_ptr->name = shStruct.info.name;
				bufferPass_ptr->description = shStruct.info.description;
				bufferPass_ptr->url = "https://www.shadertoy.com/view/" + shStruct.info.id;
				bufferPass_ptr->user = shStruct.info.username;
				bufferPass_ptr->published = (shStruct.info.published == "3");

				bufferPass_ptr->note += "\n@NOTE\n\n";
				bufferPass_ptr->note += "[[NAME]]:" + bufferPass_ptr->name + "\n";
				bufferPass_ptr->note += "[[DATE]]:" + shStruct.info.date + "\n";
				bufferPass_ptr->note += "[[URL]]:" + bufferPass_ptr->url + "\n";
				bufferPass_ptr->note += "[[USER]]:" + bufferPass_ptr->user + ":https://www.shadertoy.com/user/" + bufferPass_ptr->user + "\n";
				bufferPass_ptr->note += "Infos :\n\n" + bufferPass_ptr->description + "\n";

				puBuffersInfos[SHADERTOY_IMAGE_ID] = bufferPass_ptr;
			}
			else if (pass.type == "buffer")
			{
				if (!pass.outputs.empty())
				{
					puBuffersInfos[pass.outputs.begin()->id] = bufferPass_ptr;
				}
			}
			else if (pass.type == "cubemap")
			{
				if (!pass.outputs.empty())
				{
					puBuffersInfos[pass.outputs.begin()->id] = bufferPass_ptr;
				}
			}

			if (pass.type == "common")
			{
				puBuffersName[SHADERTOY_COMMON_ID] = nodeName;
				puBuffersId[SHADERTOY_COMMON_ID] = pass.name;
				puBuffersInfos[SHADERTOY_COMMON_ID] = bufferPass_ptr;
				puIncludeFileNames.emplace_back(nodeName + ".glsl");
				//bufferPass_ptr->uniforms += "\n@UNIFORMS\n\n";
				bufferPass_ptr->common += "\n@COMMON\n\n";
			}
			else
			{
				////////////////////////

				if (vImportInOneFile && countPass > 1.0f)
				{
					bufferPass_ptr->framebuffer += "\n@FRAMEBUFFER BUFFER(" + nodeName + ")";
					bufferPass_ptr->fragment += "\n@FRAGMENT BUFFER(" + nodeName + ")\n\n";
					bufferPass_ptr->common_uniforms += "\n@UNIFORMS\n\n";
					bufferPass_ptr->specific_uniforms += "\n@UNIFORMS BUFFER(" + nodeName + ")\n\n";
				}
				else
				{
					bufferPass_ptr->framebuffer += "\n@FRAMEBUFFER";
					bufferPass_ptr->fragment += "\n@FRAGMENT\n\n";
					bufferPass_ptr->common_uniforms += "\n@UNIFORMS\n\n";
				}

				bufferPass_ptr->vertex += "\n@VERTEX\n\n";

				////////////////////////

				//bufferPass_ptr->vertex += "#extension GL_ARB_explicit_attrib_location : enable\n";
				bufferPass_ptr->vertex += "layout(location = 0) in vec2 a_position; // Current vertex position\n";
				bufferPass_ptr->vertex += "\n";

				bufferPass_ptr->vertex += "\n";
				bufferPass_ptr->vertex += "void main()\n";
				bufferPass_ptr->vertex += "{\n";
				bufferPass_ptr->vertex += "\tgl_Position = vec4(a_position, 0.0, 1.0);\n";
				bufferPass_ptr->vertex += "}\n";
				bufferPass_ptr->vertex += "\n";

				if (pass.type == "image")
				{
					bufferPass_ptr->IsMain = true;
				}
				else
				{
					bufferPass_ptr->IsBuffer = true;
				}
			}
		}

		idxPass = 0.0f;

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
			ShaderToyFormat::ShaderToyRenderPass pass = *it_rp;

				ShaderInfos* commonBuffer = nullptr;

				if (pass.type == "common")
				{
					commonBuffer = puBuffersInfos[SHADERTOY_COMMON_ID];

					if (commonBuffer != nullptr)
					{
						commonBuffer->common += pass.code;
					}
				}
		}

		/////////////////////////////////////////////////////////////////////////
		////// 3eme passe : on fini l'association  //////////////////////////////
		/////////////////////////////////////////////////////////////////////////
		idxPass = 0.0f;
		for (std::list<ShaderToyFormat::ShaderToyRenderPass>::iterator it_rp = shStruct.renderpass.begin(); it_rp != shStruct.renderpass.end(); ++it_rp)
		{
			//std::string key = it_rp->first;
			ShaderToyFormat::ShaderToyRenderPass pass = *it_rp;

			puChannelLinks.clear();

			//float countInputs = (float)pass.inputs.size();
			//float idxInput = 0.0f;
			//float stepProgress = 30.0f / countPass;

			// les inputs
			for (std::list<ShaderToyFormat::ShaderToyInput>::iterator it_in = pass.inputs.begin(); it_in != pass.inputs.end(); ++it_in)
			{
				ShaderToyFormat::ShaderToyInput input = *it_in;

				std::string chan = input.chan;
				std::string id = input.id;

				if (input.type == "texture")
				{
					// on va donwload les eventuelles texture utilisées par le shader
					std::string picture = input.filepath; // FileHelper::Instance()->(input.src GetFileFromUrl(input.src, 0, FILE_LOCATION_Enum::FILE_LOCATION_ASSET_TEXTURE_2D, nullptr);
					auto ps = FileHelper::Instance()->ParsePathFileName(picture);
					if (ps.isOk)
					{
						picture = ps.name + "." + ps.ext;
					}

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

				if (input.type == "cubemap")
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
					//	"ctype" : "cubemap",
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

					// on va donwload les eventuelles texture utilisées par le shader
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

					auto picture = input.filepath;
					auto ps = FileHelper::Instance()->ParsePathFileName(picture);
					if (ps.isOk)
					{
						picture = ps.name + "." + ps.ext;
					}

					params["FILE1"] = picture; // GetFileFromUrl(input.src, 0, FILE_LOCATION_Enum::FILE_LOCATION_ASSET_CUBEMAP, nullptr);
					params["FILE2"] = picture + "_1" + ext;// GetFileFromUrl(url_without_ext + "_1" + ext, 0, FILE_LOCATION_Enum::FILE_LOCATION_ASSET_CUBEMAP, nullptr);
					params["FILE3"] = picture + "_2" + ext;// GetFileFromUrl(url_without_ext + "_2" + ext, 0, FILE_LOCATION_Enum::FILE_LOCATION_ASSET_CUBEMAP, nullptr);
					params["FILE4"] = picture + "_3" + ext;// GetFileFromUrl(url_without_ext + "_3" + ext, 0, FILE_LOCATION_Enum::FILE_LOCATION_ASSET_CUBEMAP, nullptr);
					params["FILE5"] = picture + "_4" + ext;// GetFileFromUrl(url_without_ext + "_4" + ext, 0, FILE_LOCATION_Enum::FILE_LOCATION_ASSET_CUBEMAP, nullptr);
					params["FILE6"] = picture + "_5" + ext;// GetFileFromUrl(url_without_ext + "_5" + ext, 0, FILE_LOCATION_Enum::FILE_LOCATION_ASSET_CUBEMAP, nullptr);

					puCubeMapsId[id] = params;
					puChannelLinks[chan] = id;
				}

				if (input.type == "buffer")
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

				if (input.type == "keyboard")
				{

				}

				if (input.type == "sound") // fature sound de shadertoy
				{

				}

				if (input.type == "musicstream")
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

				if (input.type == "music")
				{
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
					params["FILE"] = "";
					params["HISTO"] = "true";

					puTexturesSoundId[id] = params;
					puChannelLinks[chan] = id;
				}

				if (input.type == "volume")
				{
					// on va donwload les eventuelles texture utilisées par le shader
					std::string volume = input.filepath; // GetFileFromUrl(input.src, 0, FILE_LOCATION_Enum::FILE_LOCATION_ASSET_TEXTURE_3D, nullptr);
					auto ps = FileHelper::Instance()->ParsePathFileName(volume);
					if (ps.isOk)
					{
						volume = ps.name + "." + ps.ext;
					}

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
			ShaderInfos* bufferPass_ptr = nullptr;
			//ShaderInfos *commonBuffer = nullptr;

			if (pass.type == "image")
			{
				bufferPass_ptr = puBuffersInfos[SHADERTOY_IMAGE_ID];
			}
			else if (!pass.outputs.empty())
			{
				bufferPass_ptr = puBuffersInfos[pass.outputs.begin()->id];
			}

			if (bufferPass_ptr != nullptr)
			{
				std::string code = pass.code;

#ifdef _DEBUG
				FileHelper::Instance()->SaveStringToFile(code, "debug/code_buffer.glsl");
#endif

				puCodeReplacements.clear();

				if (puBuffersInfos.find(SHADERTOY_COMMON_ID) != puBuffersInfos.end()) // trouvé
				{
					std::string commonCode = puBuffersInfos[SHADERTOY_COMMON_ID]->common;
					AddUniformsFromCode(*bufferPass_ptr, code, ShaderPlaform::SPf_SHADERTOY, vImportInOneFile, &commonCode);
				}
				else
				{
					AddUniformsFromCode(*bufferPass_ptr, code, ShaderPlaform::SPf_SHADERTOY, vImportInOneFile);
				}

				if (code.find("mainVR") != std::string::npos)
				{
					bufferPass_ptr->specific_uniforms += "uniform float(vr:use) useVR;\n";
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
				// et sur pc c'est interprté par une sortie uniquement donc
				// meme si gl_FragColor est initialisé avant l'appel mainImage
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

				//bufferPass_ptr->fragment += "#extension GL_ARB_explicit_attrib_location : enable\n";
				bufferPass_ptr->fragment += "layout(location = 0) out vec4 fragColor;\n";

				if (!(vImportInOneFile && countPass > 1.0f))
				{
					// on ajoute les fichiers inclus (common)
					if (!puIncludeFileNames.empty())
					{
						bufferPass_ptr->fragment += "\n";

						for (auto it_Inc = puIncludeFileNames.begin(); it_Inc != puIncludeFileNames.end(); ++it_Inc)
						{
							bufferPass_ptr->fragment += "#include \"" + (*it_Inc) + "\"\n";
						}

						bufferPass_ptr->fragment += "\n";
					}
				}

				bufferPass_ptr->fragment += code;
			}
		}

		/////////////////////////////////////////////////////////////////////////
		////// 4eme passe : on fini les outputs  ////////////////////////////////
		/////////////////////////////////////////////////////////////////////////
		idxPass = 0.0f;
		for (std::list<ShaderToyFormat::ShaderToyRenderPass>::iterator it_rp = shStruct.renderpass.begin(); it_rp != shStruct.renderpass.end(); ++it_rp)
		{
			//std::string key = it_rp->first;
			ShaderToyFormat::ShaderToyRenderPass pass = *it_rp;

			// on defini les buffer output
			for (std::list<ShaderToyFormat::ShaderToyOutput>::iterator it_in = pass.outputs.begin(); it_in != pass.outputs.end(); ++it_in)
			{
				ShaderToyFormat::ShaderToyOutput output = *it_in;

				if (pass.type == "buffer")
				{
					SetShaderBufferFormat(output.id);
				}
				else if (pass.type == "cubemap")
				{

				}
			}
		}
	}

	// on finit par definir le buffer image
	SetShaderBufferFormat(SHADERTOY_IMAGE_ID);

	return shaders;
}

// on va rechercher les uniforms depuis le code et les ajouter automatiquement
void ImporterFromShadertoy::AddUniformsFromCode(
	ShaderInfos& vOutShaderInfos,
	std::string& vCode, 
	const ShaderPlaform& vShaderPlaform,
	const bool& vImportInOneFile, 
	const std::string* vCommonCodePtr)
{
	if (vShaderPlaform == ShaderPlaform::SPF_VERTEXSHADERART)
	{
		if (vCode.find("resolution") != string::npos)
		{
			std::string baseName = "resolution";
			vOutShaderInfos.specific_uniforms += "uniform vec2(buffer:target=0) " + baseName + ";\n";
		}
		if (vCode.find("mouse") != string::npos)
		{
			std::string baseName = "mouse";
			vOutShaderInfos.specific_uniforms += "uniform vec2(mouse:normalized) " + baseName + ";\n";
		}
		if (vCode.find("background") != string::npos)
		{
			std::string baseName = "background";
			vOutShaderInfos.specific_uniforms += "uniform vec4(color) " + baseName + ";\n";
		}
		if (vCode.find("time") != string::npos)
		{
			std::string baseName = "time";
			vOutShaderInfos.specific_uniforms += "uniform float(time) " + baseName + ";\n";
		}
		if (vCode.find("touch") != string::npos)
		{
			//std::string baseName = "touch";
		}
		if (vCode.find("volume") != string::npos)
		{
			//std::string baseName = "volume";
		}
		if (vCode.find("sound") != string::npos)
		{
			std::string baseName = "sound";
			vOutShaderInfos.specific_uniforms += "uniform sampler2D(sound_histo) " + baseName + ";\n";
		}
		if (vCode.find("floatSound") != string::npos)
		{
			std::string baseName = "floatSound";
			vOutShaderInfos.specific_uniforms += "uniform sampler2D(sound_histo) " + baseName + ";\n";
		}
		if (vCode.find("vertexCount") != string::npos)
		{
			std::string baseName = "vertexCount";
			vOutShaderInfos.specific_uniforms += "uniform float(maxpoints) " + baseName + ";\n";
		}
	}
	else
	{
		size_t varying_pos = 0;
		while ((varying_pos = vCode.find("varying ", varying_pos)) != std::string::npos)
		{
			size_t coma_pos = vCode.find(';', varying_pos);
			size_t comeback_pos = vCode.find('\n', varying_pos);
			if (coma_pos != std::string::npos)
			{
				std::string var_line = vCode.substr(varying_pos, coma_pos - varying_pos);
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
						vCode.erase(varying_pos, comeback_pos + 1 - varying_pos);
					}
					else
					{
						vCode.erase(varying_pos, coma_pos + 1 - varying_pos);
					}

					if (varying_pos > 0)
						varying_pos--;

					if (vShaderPlaform == ShaderPlaform::SPF_UNKNOW)
					{
						std::string line;

						line = var_line;
						ct::replaceString(line, "varying", "out");
						vOutShaderInfos.vertex += line + ";\n";

						line = var_line;
						ct::replaceString(line, "varying", "in");
						vOutShaderInfos.fragment += line + ";\n";

						vOutShaderInfos.varying[name] = true;
					}
					if (vShaderPlaform == ShaderPlaform::SPF_GLSLSANDBOX)
					{
						std::string line;

						line = var_line;
						ct::replaceString(line, "varying", "out");
						vOutShaderInfos.vertex += line + ";\n";

						line = var_line;
						ct::replaceString(line, "varying", "in");
						vOutShaderInfos.fragment += line + ";\n";

						vOutShaderInfos.varying[name] = true;
					}
					if (vShaderPlaform == ShaderPlaform::SPf_SHADERTOY)
					{

					}
				}
			}

			varying_pos += 1;
		}

		//size_t uniforms_start = vCode.find("@UNIFORMS_START");
		//size_t uniforpuelse = vCode.find("@UNIFORMS_ELSE");
		//size_t uniforpuend = vCode.find("@UNIFORMS_END");

		size_t uniforpupos = 0;
		while ((uniforpupos = vCode.find("uniform ", uniforpupos)) != std::string::npos)
		{
			if (!ShaderStageParsing::IfInCommentZone(vCode, uniforpupos))
			{
				size_t coma_pos = vCode.find(';', uniforpupos);
				size_t comeback_pos = vCode.find('\n', uniforpupos);
				if (coma_pos != std::string::npos)
				{
					std::string uni_line = vCode.substr(uniforpupos, coma_pos - uniforpupos);
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
						vCode.erase(uniforpupos, length_to_erase);

						if (uniforpupos > 0)
							uniforpupos--;

						if (vShaderPlaform == ShaderPlaform::SPF_UNKNOW)
						{
							vOutShaderInfos.specific_uniforms += uni_line + "\n";
						}
						if (vShaderPlaform == ShaderPlaform::SPF_GLSLSANDBOX)
						{
							std::string baseName = name;

							if (baseName == "resolution")
							{
								vOutShaderInfos.specific_uniforms += "uniform vec2(buffer) " + name + ";\n";
							}
							else if (baseName == "surfaceSize")
							{
								vOutShaderInfos.specific_uniforms += "uniform vec2(buffer) " + name + ";\n";
							}
							else if (baseName == "mouse")
							{
								vOutShaderInfos.specific_uniforms += "uniform vec2(mouse:normalized) " + name + ";\n";
							}
							else if (baseName == "time")
							{
								vOutShaderInfos.specific_uniforms += "uniform float(time) " + name + ";\n";
							}
							else if (type == "sampler2D")
							{
								vOutShaderInfos.specific_uniforms += "uniform sampler2D(buffer) " + name + ";\n";
							}
						}
						if (vShaderPlaform == ShaderPlaform::SPf_SHADERTOY)
						{
							if (name == "iGlobalTime")
								name = "iTime";

							std::string baseName = name;

							if (baseName == "iResolution")
							{
								vOutShaderInfos.common_uniforms += "uniform vec3(buffer) " + name + ";\n";
							}
							else if (baseName == "iMouse")
							{
								vOutShaderInfos.common_uniforms += "uniform vec4(mouse:shadertoy) " + name + ";\n";
							}
							else if (baseName == "iGlobalTime" || baseName == "iTime")
							{
								vOutShaderInfos.common_uniforms += "uniform float(time) " + name + ";\n";
							}
							//if (baseName == "iChannelTime[4]")
							//{
							//}
							else if (baseName == "iDate")
							{
								vOutShaderInfos.common_uniforms += "uniform vec4(date) " + name + ";\n";
							}
							//if (baseName == "iSampleRate")
							//{
							//}
							else if (baseName == "iFrame")
							{
								vOutShaderInfos.common_uniforms += "uniform int(frame) " + name + ";\n";
							}
							else if (baseName == "iTimeDelta")
							{
								vOutShaderInfos.common_uniforms += "uniform float(deltatime) " + name + ";\n";
							}
							//if (baseName == "iFrameRate")
							//{
							//}
							else if (baseName == "iChannel0")
							{
								std::vector<std::string::size_type> lst = ct::strContains(vCode, baseName);
								if (lst.empty() && vCommonCodePtr)
									lst = ct::strContains(*vCommonCodePtr, baseName);
								DoChannelInput(baseName, vOutShaderInfos, "0", lst.size(), vImportInOneFile);
							}
							else if (baseName == "iChannel1")
							{
								std::vector<std::string::size_type> lst = ct::strContains(vCode, baseName);
								if (lst.empty() && vCommonCodePtr)
									lst = ct::strContains(*vCommonCodePtr, baseName);
								DoChannelInput(baseName, vOutShaderInfos, "1", lst.size(), vImportInOneFile);
							}
							else if (baseName == "iChannel2")
							{
								std::vector<std::string::size_type> lst = ct::strContains(vCode, baseName);
								if (lst.empty() && vCommonCodePtr)
									lst = ct::strContains(*vCommonCodePtr, baseName);
								DoChannelInput(baseName, vOutShaderInfos, "2", lst.size(), vImportInOneFile);
							}
							else if (baseName == "iChannel3")
							{
								std::vector<std::string::size_type> lst = ct::strContains(vCode, baseName);
								if (lst.empty() && vCommonCodePtr)
									lst = ct::strContains(*vCommonCodePtr, baseName);
								DoChannelInput(baseName, vOutShaderInfos, "3", lst.size(), vImportInOneFile);
							}
							else if (baseName == "sampler2D")
							{
								vOutShaderInfos.specific_uniforms += "uniform sampler2D " + name + ";\n";
							}
							else if (baseName == "samplerCube")
							{
								vOutShaderInfos.specific_uniforms += "uniform samplerCube " + name + ";\n";
							}
						}
					}
				}
			}
				
			uniforpupos += 1;
		}

		size_t note_start = ShaderStageParsing::GetTagPos(vCode, "@NOTE_START", 0, false, true);
		size_t note_end = ShaderStageParsing::GetTagPos(vCode, "@NOTE_END", 0, false, true);

		if (note_start != std::string::npos)
		{
			if (note_end != std::string::npos)
			{
				if (note_start < note_end)
				{
					// on va mettre dans la section note ce qu'il y a entre @NOTE_START et @NOTE_END
					size_t loc = note_start + std::string("@NOTE_START").size();

					std::string note = vCode.substr(loc, note_end - loc);
					ct::replaceString(note, "://", ":\\");
					ct::replaceString(note, "//", "");
					ct::replaceString(note, "/*", "");
					ct::replaceString(note, "*/", "");
					ct::replaceString(note, ":\\", "://");

					vOutShaderInfos.note += note;

					note_end += std::string("@NOTE_END").size();
					vCode.erase(note_start, note_end - note_start);
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
			
		size_t uniforms_start = ShaderStageParsing::GetTagPos(vCode, "@UNIFORMS_START", 0, false, true);
		size_t uniforpuelse = ShaderStageParsing::GetTagPos(vCode, "@UNIFORMS_ELSE", 0, false, true);
		size_t uniforpuend = ShaderStageParsing::GetTagPos(vCode, "@UNIFORMS_END", 0, false, true);

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
						std::string uniforms = vCode.substr(loc, uniforpuelse - loc);
						ct::replaceString(uniforms, "//", "");
						ct::replaceString(uniforms, "/*", "");
						ct::replaceString(uniforms, "*/", "");
						ct::replaceString(uniforms, ";", "; //");

						//FileHelper::Instance()->SaveToFile(uniforms, "uniforms.glsl", FILE_LOCATION_DEBUG);

						vOutShaderInfos.specific_uniforms += uniforms;

						//FileHelper::Instance()->SaveToFile(vOutShaderInfos.uniforms, "vOutShaderInfos_uniforms.glsl", FILE_LOCATION_DEBUG);

						uniforpuend += std::string("@UNIFORMS_END").size();
						vCode.erase(uniforms_start, uniforpuend - uniforms_start);

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

std::vector<std::string> ImporterFromShadertoy::ParseUniform(const std::string& /*vUniformStr*/)
{
	std::vector<std::string> arr;

	return arr;
}

void ImporterFromShadertoy::SetShaderBufferFormat(const std::string& vBufferId)
{
	if (puBuffersInfos.find(vBufferId) != puBuffersInfos.end()) // buffer
	{
		auto infos = puBuffersInfos[vBufferId];
		if (infos)
		{
			if (vBufferId == SHADERTOY_IMAGE_ID) // image
			{
				infos->framebuffer += "\nWRAP(clamp)\nMIPMAP(false)\nFILTER(linear)\n//COUNT(1)\n//SIZE(512)\n";
			}
			else
			{
				const auto& params = puBuffersParams[vBufferId];

				std::string mipmap = "false";
				std::string filter = "linear";
				if (params.find("MAG") != params.end())
				{
					if (params.at("MAG") == "mipmap")
					{
						mipmap = "true";
					}
					else
					{
						filter = params.at("MAG");
					}
				}

				std::string wrap = "clamp";
				if (params.find("WRAPS") != params.end())
				{
					wrap = params.at("WRAPS");
				}

				infos->framebuffer += "\nWRAP(" + wrap + ")\nMIPMAP(" + mipmap + ")\nFILTER(" + filter + ")\n//COUNT(1)\n//SIZE(512)\n";
			}
		}
		else
		{
			LogVarError("%i n'est pas id de buffer valide; non trouvé !", vBufferId);
		}
	}
	else
	{
		LogVarError("ShaderInfos %i not found !", vBufferId);
	}
}

void ImporterFromShadertoy::DoChannelInput(
	const std::string& vBaseName, 
	ShaderInfos& vOutShaderInfos, 
	const std::string& vChanId, 
	const size_t& nOccurs, 
	const bool& vImportInOneFile)
{
	// a ce stade les ligne d'uniformes concernés ont été éffacés
	if (nOccurs > 0)
	{
		if (puChannelLinks.find(vChanId) != puChannelLinks.end()) // trouve
		{
			const auto& id = puChannelLinks[vChanId];

			if (puBuffersId.find(id) != puBuffersId.end()) // buffer
			{
				auto bufferId = vBaseName;
				auto bufferName = puBuffersName[id];

				puCodeReplacements[vBaseName] = bufferId;
				puCodeReplacements["iChannelResolution[" + ct::toStr(vChanId) + "]"] = bufferId + "Size";

				if (bufferName == vOutShaderInfos.id)
				{
					vOutShaderInfos.specific_uniforms += "uniform sampler2D(buffer) " + bufferId + ";\n";
				}
				else
				{
					if (vImportInOneFile)
					{
						vOutShaderInfos.specific_uniforms += "uniform sampler2D(buffer:buffer=" + bufferName + ") " + bufferId + ";\n";
					}
					else
					{
						vOutShaderInfos.specific_uniforms += "uniform sampler2D(buffer:file=" + bufferName + ") " + bufferId + ";\n";
					}
				}
				vOutShaderInfos.specific_uniforms += "uniform vec2(buffer:target=" + bufferId + ") " + bufferId + "Size;\n";
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

				vOutShaderInfos.specific_uniforms += "uniform sampler2D(picture:choice=" + pict + ":flip=" + flip + ":mipmap=" + mipmap + ":wrap=" + wrap + ":filter=" + filter + ") " + vBaseName + ";\n";
				vOutShaderInfos.specific_uniforms += "uniform vec2(picture:target=" + vBaseName + ") " + vBaseName + "Size;\n";
			}
			else if (puBufferCubeMapsId.find(id) != puBufferCubeMapsId.end()) // buffer cubemap
			{
				std::string bufferId = puBufferCubeMapsId[id];
				std::string bufferName = puBuffersName[id];

				puCodeReplacements[vBaseName] = bufferId;
				puCodeReplacements["iChannelResolution[" + ct::toStr(vChanId) + "]"] = bufferId + "Size";

				vOutShaderInfos.specific_uniforms += "uniform samplerCube(cubemap:file=" + bufferName + ") " + vBaseName + ";\n";
			}
			else if (puCubeMapsId.find(id) != puCubeMapsId.end()) // cubemap
			{
				std::unordered_map<std::string, std::string> params = puCubeMapsId[id];

				vOutShaderInfos.specific_uniforms += "uniform samplerCube(cubemap:files=";
				vOutShaderInfos.specific_uniforms += params["FILE1"] + ",";
				vOutShaderInfos.specific_uniforms += params["FILE2"] + ",";
				vOutShaderInfos.specific_uniforms += params["FILE3"] + ",";
				vOutShaderInfos.specific_uniforms += params["FILE4"] + ",";
				vOutShaderInfos.specific_uniforms += params["FILE5"] + ",";
				vOutShaderInfos.specific_uniforms += params["FILE6"] + ") " + vBaseName + ";\n";
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

				vOutShaderInfos.specific_uniforms += "uniform sampler3D(volume:file=" + pict + ":format=" + format + ":mipmap=" + mipmap + ":wrap=" + wrap + ":filter=" + filter + ") " + vBaseName + ";\n";
				vOutShaderInfos.specific_uniforms += "uniform vec3(volume:target=" + vBaseName + ") " + vBaseName + "Size;\n";
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
						vOutShaderInfos.specific_uniforms += "uniform sampler2D(sound_hsito:file=" + file + ") " + vBaseName + ";\n";
					}
					else
					{
						vOutShaderInfos.specific_uniforms += "uniform sampler1D(sound:file=" + file + ") " + vBaseName + ";\n";
					}
				}
				else if (histo == "true")
				{
					vOutShaderInfos.specific_uniforms += "uniform sampler2D(sound_histo) " + vBaseName + ";\n";
				}
				else
				{
					vOutShaderInfos.specific_uniforms += "uniform sampler1D(sound) " + vBaseName + ";\n";
				}
			}
			else
			{
				LogVarError("Channel id %i not found", id);
			}
		}
		else // l'input n'est pas référencé on va ajoute un uniform de type texture vide
		{
			vOutShaderInfos.specific_uniforms += "uniform sampler2D " + vBaseName + ";\n";
		}
	}
}